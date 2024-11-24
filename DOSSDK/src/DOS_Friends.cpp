#include "DOS_Friends.h"
#include <vector>
#include <iostream>

namespace DenateFriends
{

    size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
    {

        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;

    }

    std::vector<std::string> parseIntoVector(const std::string& str, const std::string& delimiter) {
        std::vector<std::string> filters;
        size_t start = 0;
        size_t end = str.find(delimiter);

        while (str.find(delimiter))
        {
            filters.push_back(str.substr(start, end));
            start = end + delimiter.length();
            end = str.find(delimiter, start);
        }
        filters.push_back(str.substr(start));
        return filters;
    }

    void DOS_Friends::replaceSubstring(std::string& original, const std::string& toReplace, const std::string& replacement)
    {
        size_t pos = 0;
        while ((pos = original.find(toReplace, pos)) != std::string::npos)
        {
            original.replace(pos, toReplace.length(), replacement);
            pos += replacement.length();
        }
    }

    DOS_Friends::DOS_Friends(std::string userID, std::string appID, bool dedicatedServer, std::string token, DenateUserDetails userDetails, DenateConnection::DOS_Connection& denateConnection): internalDenateConnection(denateConnection)
    {
        this->userID = userID;
        this->appID = appID;
        this->dedicatedServer = dedicatedServer;
        this->token = token;
        this->userDetails = userDetails;
    }

    AddFriendResult DOS_Friends::AddFriend(std::string friendName)
    {
        
        CURL* curl;
        CURLcode res;
        AddFriendResult result;
        std::string readBuffer;
        bool friendRequestSent = false;
        DenateHTTPResponse httpResponse;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            //replaceSubstring(Tag, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denatefriends/addfriendbyusername"));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);


            std::string json = R"({"appID": ")" + appID + "\"" + "," + R"("userID": ")" + userID + "\"" + "," + R"("player_name": ")" + userDetails.username + "\"" + "," + R"("friend_name": ")" + friendName + "\"" + "," + R"("player_name_appuserID": ")" + "" + "\"" + "," + R"("friend_name_appuserID": ")" + "" + "\"" + "," + R"("accepted": )" + std::to_string(0) + R"(})";
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
            std::cout << json << std::endl;

            struct curl_slist* headers = NULL;
            //headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("appID: " + std::string(appID)).c_str());
            headers = curl_slist_append(headers, ("userID: " + std::string(userID)).c_str());
            headers = curl_slist_append(headers, ("token: " + std::string(token)).c_str());

            std::string dedicatedServerString;
            dedicatedServer ? dedicatedServerString = "True" : "False";
            headers = curl_slist_append(headers, ("dedicatedServer: " + std::string(dedicatedServerString)).c_str());

            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                fprintf(stderr, "Request Failed . Curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                httpResponse.requestSuccessful = false;
                std::cout << "Denate Request Failed" << std::endl;
                return result;
            }
            else {
                fprintf(stdout, "Request Successful . Curl_easy_perform() Success: %s\n", curl_easy_strerror(res));
                httpResponse.requestSuccessful = true;
                std::cout << "Denate Request Successful" << std::endl;
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }
        //std::cout << readBuffer << std::endl;

        nlohmann::json jsonResponse = nlohmann::json::parse(readBuffer);

        httpResponse.status_code = jsonResponse["status"];
        httpResponse.message = jsonResponse["message"];

        if (jsonResponse.contains("response"))
        {
            friendRequestSent = true;

            if (&internalDenateConnection != nullptr)
            {
                if (internalDenateConnection.isDenateOnlineServiceConnected)
                {
                    sio::message::ptr jsonMessage = sio::object_message::create();

                    jsonMessage->get_map()["appID"] = sio::string_message::create(appID);
                    jsonMessage->get_map()["player_name"] = sio::string_message::create(std::string(jsonResponse["response"]["friend_name"]));
                    jsonMessage->get_map()["friend_name"] = sio::string_message::create(std::string(jsonResponse["response"]["player_name"]));

                    internalDenateConnection.namespaceSocket->emit("addplayer", jsonMessage, [&](sio::message::list const& ack_msg) {

                        });
                }
            }

        }

        result.httpResponse = httpResponse;
        result.friendRequestSent = friendRequestSent;

        curl_global_cleanup();

        return result;

    }

    GetFriendRequestsResult DOS_Friends::GetFriendRequests(std::string playerName)
    {

        CURL* curl;
        CURLcode res;
        GetFriendRequestsResult result;
        std::string readBuffer;
        bool gottenFriendRequest = false;
        DenateHTTPResponse httpResponse;
        std::vector<DenateFriendDetails> frienddetails;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            if (playerName == "")
            {
                playerName = userDetails.username;
            }

            replaceSubstring(playerName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denatefriends/getallfriendrequest/") + std::string(playerName));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);


            struct curl_slist* headers = NULL;
            //headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("appID: " + std::string(appID)).c_str());
            headers = curl_slist_append(headers, ("userID: " + std::string(userID)).c_str());
            headers = curl_slist_append(headers, ("token: " + std::string(token)).c_str());

            std::string dedicatedServerString;
            dedicatedServer ? dedicatedServerString = "True" : "False";
            headers = curl_slist_append(headers, ("dedicatedServer: " + std::string(dedicatedServerString)).c_str());

            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                fprintf(stderr, "Request Failed . Curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                httpResponse.requestSuccessful = false;
                std::cout << "Denate Request Failed" << std::endl;
                return result;
            }
            else {
                fprintf(stdout, "Request Successful . Curl_easy_perform() Success: %s\n", curl_easy_strerror(res));
                httpResponse.requestSuccessful = true;
                std::cout << "Denate Request Successful" << std::endl;
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }
        //std::cout << readBuffer << std::endl;

        nlohmann::json jsonResponse = nlohmann::json::parse(readBuffer);

        httpResponse.status_code = jsonResponse["status"];
        httpResponse.message = jsonResponse["message"];

        if (jsonResponse.contains("response"))
        {
            gottenFriendRequest = true;

            if (jsonResponse["response"].is_array())
            {
                for (const auto& detail : jsonResponse["response"])
                {

                    DenateFriendDetails frienddetail;

                    frienddetail.playerName = !detail["player_name"].is_null() ? detail["player_name"] : "";
                    frienddetail.appUserID = !detail["player_name_appuserID"].is_null() ? detail["player_name_appuserID"] : "";
                    frienddetail.picture = !detail["picture"].is_null() ? detail["picture"] : "";
                    
                    if (!detail["ingame"].is_null())
                    {
                        frienddetail.online = detail["ingame"];
                    }

                    if (detail.contains("online"))
                    {
                        if (!detail["online"].is_null())
                        {
                            frienddetail.online = detail["online"];
                        }
                    }
                    else {
                        frienddetail.online = false;
                    }

                    frienddetails.push_back(frienddetail);

                }
            }

        }

        result.httpResponse = httpResponse;
        result.gottenFriendRequest = gottenFriendRequest;
        result.friendDetails = frienddetails;

        curl_global_cleanup();

        return result;

    }

    GetFriendsResult DOS_Friends::GetFriends(std::string playerName)
    {

        CURL* curl;
        CURLcode res;
        GetFriendsResult result;
        std::string readBuffer;
        bool foundFriends = false;
        DenateHTTPResponse httpResponse;
        std::vector<DenateFriendDetails> frienddetails;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            if (playerName == "")
            {
                playerName = userDetails.username;
            }

            replaceSubstring(playerName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denatefriends/getallfriends/") + std::string(playerName));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);


            struct curl_slist* headers = NULL;
            //headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("appID: " + std::string(appID)).c_str());
            headers = curl_slist_append(headers, ("userID: " + std::string(userID)).c_str());
            headers = curl_slist_append(headers, ("token: " + std::string(token)).c_str());

            std::string dedicatedServerString;
            dedicatedServer ? dedicatedServerString = "True" : "False";
            headers = curl_slist_append(headers, ("dedicatedServer: " + std::string(dedicatedServerString)).c_str());

            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                fprintf(stderr, "Request Failed . Curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                httpResponse.requestSuccessful = false;
                std::cout << "Denate Request Failed" << std::endl;
                return result;
            }
            else {
                fprintf(stdout, "Request Successful . Curl_easy_perform() Success: %s\n", curl_easy_strerror(res));
                httpResponse.requestSuccessful = true;
                std::cout << "Denate Request Successful" << std::endl;
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }
        //std::cout << readBuffer << std::endl;

        nlohmann::json jsonResponse = nlohmann::json::parse(readBuffer);

        httpResponse.status_code = jsonResponse["status"];
        httpResponse.message = jsonResponse["message"];

        if (jsonResponse.contains("response"))
        {
            foundFriends = true;

            if (jsonResponse["response"].is_array())
            {
                for (const auto& detail : jsonResponse["response"])
                {

                    DenateFriendDetails frienddetail;

                    frienddetail.picture = !detail["picture"].is_null() ? detail["picture"] : "";

                    if (!detail["ingame"].is_null())
                    {
                        frienddetail.online = detail["ingame"];
                    }

                    if (detail.contains("online"))
                    {
                        if (!detail["online"].is_null())
                        {
                            frienddetail.online = detail["online"];
                        }
                    }
                    else {
                        frienddetail.online = false;
                    }

                    if (detail.contains("friendrequestsent") && !detail["friendrequestsent"].is_null())
                    {
                        bool friendrequestsent = detail["friendrequestsent"];
                        if (friendrequestsent)
                        {
                            frienddetail.playerName = !detail["friend_name"].is_null() ? detail["friend_name"] : "";
                            frienddetail.appUserID = !detail["player_name_appuserID"].is_null() ? detail["friend_name_appuserID"] : "";
                        }
                        else {
                            frienddetail.playerName = !detail["player_name"].is_null() ? detail["player_name"] : "";
                            frienddetail.appUserID = !detail["player_name_appuserID"].is_null() ? detail["player_name_appuserID"] : "";
                        }
                    }

                    frienddetails.push_back(frienddetail);

                }
            }

        }

        result.httpResponse = httpResponse;
        result.foundFriends = foundFriends;
        result.friendDetails = frienddetails;

        curl_global_cleanup();

        return result;

    }

    DeclineFriendRequestResult DOS_Friends::DeclineFriendRequest(std::string friendName)
    {
        CURL* curl;
        CURLcode res;
        DeclineFriendRequestResult result;
        std::string readBuffer;
        bool declined = false;
        DenateHTTPResponse httpResponse;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            std::string playerName = userDetails.username;

            replaceSubstring(playerName, " ", "%20");
            replaceSubstring(friendName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denatefriends/declinefriendrequest/") + std::string(playerName) + "/" + std::string(friendName));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            struct curl_slist* headers = NULL;
            //headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("appID: " + std::string(appID)).c_str());
            headers = curl_slist_append(headers, ("userID: " + std::string(userID)).c_str());
            headers = curl_slist_append(headers, ("token: " + std::string(token)).c_str());

            std::string dedicatedServerString;
            dedicatedServer ? dedicatedServerString = "True" : "False";
            headers = curl_slist_append(headers, ("dedicatedServer: " + std::string(dedicatedServerString)).c_str());

            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                fprintf(stderr, "Request Failed . Curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                httpResponse.requestSuccessful = false;
                std::cout << "Denate Request Failed" << std::endl;
                return result;
            }
            else {
                fprintf(stdout, "Request Successful . Curl_easy_perform() Success: %s\n", curl_easy_strerror(res));
                httpResponse.requestSuccessful = true;
                std::cout << "Denate Request Successful" << std::endl;
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }
        //std::cout << readBuffer << std::endl;

        nlohmann::json jsonResponse = nlohmann::json::parse(readBuffer);

        httpResponse.status_code = jsonResponse["status"];
        httpResponse.message = jsonResponse["message"];

        if (jsonResponse.contains("response"))
        {
            declined = true;

            if (&internalDenateConnection != nullptr)
            {
                if (internalDenateConnection.isDenateOnlineServiceConnected)
                {
                    sio::message::ptr jsonMessage = sio::object_message::create();

                    jsonMessage->get_map()["appID"] = sio::string_message::create(appID);
                    jsonMessage->get_map()["player_name"] = sio::string_message::create(std::string(jsonResponse["response"]["friend_name"]));
                    jsonMessage->get_map()["friend_name"] = sio::string_message::create(std::string(jsonResponse["response"]["player_name"]));

                    internalDenateConnection.namespaceSocket->emit("denyfriendrequest", jsonMessage, [&](sio::message::list const& ack_msg) {

                        });
                }
            }

        }

        result.httpResponse = httpResponse;
        result.friendRequestDeclined = declined;

        curl_global_cleanup();

        return result;
    }

    UnfriendPlayerResult DOS_Friends::UnfriendPlayer(std::string friendName)
    {
        CURL* curl;
        CURLcode res;
        UnfriendPlayerResult result;
        std::string readBuffer;
        bool unfriended = false;
        DenateHTTPResponse httpResponse;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            std::string playerName = userDetails.username;

            replaceSubstring(playerName, " ", "%20");
            replaceSubstring(friendName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denatefriends/unfrienduser/") + std::string(playerName) + "/" + std::string(friendName));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            struct curl_slist* headers = NULL;
            //headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("appID: " + std::string(appID)).c_str());
            headers = curl_slist_append(headers, ("userID: " + std::string(userID)).c_str());
            headers = curl_slist_append(headers, ("token: " + std::string(token)).c_str());

            std::string dedicatedServerString;
            dedicatedServer ? dedicatedServerString = "True" : "False";
            headers = curl_slist_append(headers, ("dedicatedServer: " + std::string(dedicatedServerString)).c_str());

            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                fprintf(stderr, "Request Failed . Curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                httpResponse.requestSuccessful = false;
                std::cout << "Denate Request Failed" << std::endl;
                return result;
            }
            else {
                fprintf(stdout, "Request Successful . Curl_easy_perform() Success: %s\n", curl_easy_strerror(res));
                httpResponse.requestSuccessful = true;
                std::cout << "Denate Request Successful" << std::endl;
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }
        //std::cout << readBuffer << std::endl;

        nlohmann::json jsonResponse = nlohmann::json::parse(readBuffer);

        httpResponse.status_code = jsonResponse["status"];
        httpResponse.message = jsonResponse["message"];

        if (jsonResponse.contains("response"))
        {
            unfriended = true;
        }

        result.httpResponse = httpResponse;
        result.unfriended = unfriended;

        curl_global_cleanup();

        return result;
    }

    AcceptFriendRequestResult DOS_Friends::AcceptFriendRequest(std::string friendName)
    {
        CURL* curl;
        CURLcode res;
        AcceptFriendRequestResult result;
        std::string readBuffer;
        bool accepted = false;
        DenateHTTPResponse httpResponse;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            std::string playerName = userDetails.username;

            replaceSubstring(playerName, " ", "%20");
            replaceSubstring(friendName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denatefriends/acceptfriendrequest/") + std::string(playerName) + "/" + std::string(friendName));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            struct curl_slist* headers = NULL;
            //headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("appID: " + std::string(appID)).c_str());
            headers = curl_slist_append(headers, ("userID: " + std::string(userID)).c_str());
            headers = curl_slist_append(headers, ("token: " + std::string(token)).c_str());

            std::string dedicatedServerString;
            dedicatedServer ? dedicatedServerString = "True" : "False";
            headers = curl_slist_append(headers, ("dedicatedServer: " + std::string(dedicatedServerString)).c_str());

            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                fprintf(stderr, "Request Failed . Curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                httpResponse.requestSuccessful = false;
                std::cout << "Denate Request Failed" << std::endl;
                return result;
            }
            else {
                fprintf(stdout, "Request Successful . Curl_easy_perform() Success: %s\n", curl_easy_strerror(res));
                httpResponse.requestSuccessful = true;
                std::cout << "Denate Request Successful" << std::endl;
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }
        //std::cout << readBuffer << std::endl;

        nlohmann::json jsonResponse = nlohmann::json::parse(readBuffer);

        httpResponse.status_code = jsonResponse["status"];
        httpResponse.message = jsonResponse["message"];

        if (jsonResponse.contains("response"))
        {
            accepted = true;

            if (&internalDenateConnection != nullptr)
            {
                if (internalDenateConnection.isDenateOnlineServiceConnected)
                {
                    sio::message::ptr jsonMessage = sio::object_message::create();

                    jsonMessage->get_map()["appID"] = sio::string_message::create(appID);
                    jsonMessage->get_map()["player_name"] = sio::string_message::create(std::string(jsonResponse["response"]["friend_name"]));
                    jsonMessage->get_map()["friend_name"] = sio::string_message::create(std::string(jsonResponse["response"]["player_name"]));

                    internalDenateConnection.namespaceSocket->emit("acceptfriendrequest", jsonMessage, [&](sio::message::list const& ack_msg) {

                        });
                }
            }

        }

        result.httpResponse = httpResponse;
        result.accepted = accepted;

        curl_global_cleanup();

        return result;
    }

    SearchPlayerResult DOS_Friends::SearchPlayer(std::string query)
    {
        CURL* curl;
        CURLcode res;
        SearchPlayerResult result;
        std::string readBuffer;
        bool found = false;
        DenateHTTPResponse httpResponse;
        std::vector<DenateSearchUserDetails> searchDetails;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            std::string playerName = userDetails.username;

            replaceSubstring(playerName, " ", "%20");
            replaceSubstring(query, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denatefriends/searchplayers/") + std::string(query) + "/" + std::string(playerName));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            struct curl_slist* headers = NULL;
            //headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("appID: " + std::string(appID)).c_str());
            headers = curl_slist_append(headers, ("userID: " + std::string(userID)).c_str());
            headers = curl_slist_append(headers, ("token: " + std::string(token)).c_str());

            std::string dedicatedServerString;
            dedicatedServer ? dedicatedServerString = "True" : "False";
            headers = curl_slist_append(headers, ("dedicatedServer: " + std::string(dedicatedServerString)).c_str());

            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                fprintf(stderr, "Request Failed . Curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                httpResponse.requestSuccessful = false;
                std::cout << "Denate Request Failed" << std::endl;
                return result;
            }
            else {
                fprintf(stdout, "Request Successful . Curl_easy_perform() Success: %s\n", curl_easy_strerror(res));
                httpResponse.requestSuccessful = true;
                std::cout << "Denate Request Successful" << std::endl;
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }
        //std::cout << readBuffer << std::endl;

        nlohmann::json jsonResponse = nlohmann::json::parse(readBuffer);

        httpResponse.status_code = jsonResponse["status"];
        httpResponse.message = jsonResponse["message"];

        if (jsonResponse.contains("response"))
        {
            found = true;

            if (jsonResponse["response"].is_array())
            {
                for (const auto& user : jsonResponse["response"])
                {

                    DenateSearchUserDetails searchdetail;

                    searchdetail.playerName = !user["player_name"].is_null() ? user["player_name"] : "";
                    searchdetail.appUserID = !user["appuserid"].is_null() ? user["appuserid"] : "";
                    searchdetail.picture = !user["picture"].is_null() ? user["picture"] : "";

                    if (!user["ingame"].is_null() && user.contains("ingame"))
                    {
                        searchdetail.inGame = user["ingame"];
                    }
                    if (!user["online"].is_null() && user.contains("online"))
                    {
                        searchdetail.online = user["online"];
                    }
                    if (!user["friend"].is_null() && user.contains("friend"))
                    {
                        searchdetail.is_a_Friend = user["friend"];
                    }
                    if (!user["friendrequestsent"].is_null() && user.contains("friendrequestsent"))
                    {
                        searchdetail.friendRequestSent = user["friendrequestsent"];
                    }
                    if (!user["friendrequestrecieved"].is_null() && user.contains("friendrequestrecieved"))
                    {
                        searchdetail.friendRequestRecieved = user["friendrequestrecieved"];
                    }

                    searchDetails.push_back(searchdetail);

                }
            }

        }

        result.httpResponse = httpResponse;
        result.found = found;
        result.searchUserDetails = searchDetails;

        curl_global_cleanup();

        return result;
    }

    CheckPlayerOnlineStatusResult DOS_Friends::CheckPlayerOnlineStatus(std::string playerName)
    {
        CURL* curl;
        CURLcode res;
        CheckPlayerOnlineStatusResult result;
        std::string readBuffer;
        bool requestSuccessful = false;
        DenateHTTPResponse httpResponse;
        std::vector<DenateSearchUserDetails> searchDetails;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            replaceSubstring(playerName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denatefriends/checkplayeronlinestatus/") + std::string(playerName));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            struct curl_slist* headers = NULL;
            //headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("appID: " + std::string(appID)).c_str());
            headers = curl_slist_append(headers, ("userID: " + std::string(userID)).c_str());
            headers = curl_slist_append(headers, ("token: " + std::string(token)).c_str());

            std::string dedicatedServerString;
            dedicatedServer ? dedicatedServerString = "True" : "False";
            headers = curl_slist_append(headers, ("dedicatedServer: " + std::string(dedicatedServerString)).c_str());

            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                fprintf(stderr, "Request Failed . Curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                httpResponse.requestSuccessful = false;
                std::cout << "Denate Request Failed" << std::endl;
                return result;
            }
            else {
                fprintf(stdout, "Request Successful . Curl_easy_perform() Success: %s\n", curl_easy_strerror(res));
                httpResponse.requestSuccessful = true;
                std::cout << "Denate Request Successful" << std::endl;
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }
        //std::cout << readBuffer << std::endl;

        nlohmann::json jsonResponse = nlohmann::json::parse(readBuffer);

        httpResponse.status_code = jsonResponse["status"];
        httpResponse.message = jsonResponse["message"];

        if (jsonResponse.contains("response"))
        {
            requestSuccessful = true;

            if (jsonResponse["response"].is_array())
            {

                if (!jsonResponse["response"]["ingame"].is_null() && jsonResponse["response"].contains("ingame"))
                {
                    result.inGame = jsonResponse["response"]["ingame"];
                }
                if (!jsonResponse["response"]["online"].is_null() && jsonResponse["response"].contains("online"))
                {
                    result.online = jsonResponse["response"]["online"];
                }
                
            }

        }

        result.httpResponse = httpResponse;
        result.requestSuccessful = requestSuccessful;

        curl_global_cleanup();

        return result;
    }

    AcceptInviteResult DOS_Friends::AcceptInvite(std::string friendName)
    {
        CURL* curl;
        CURLcode res;
        AcceptInviteResult result;
        std::string readBuffer;
        bool inviteAccepted = false;
        DenateHTTPResponse httpResponse;
        DenateMatchDetails matchDetails;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            std::string playerName = userDetails.username;

            replaceSubstring(playerName, " ", "%20");
            replaceSubstring(friendName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denatefriends/acceptinvite/") + std::string(playerName) + "/" + std::string(friendName));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            struct curl_slist* headers = NULL;
            //headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("appID: " + std::string(appID)).c_str());
            headers = curl_slist_append(headers, ("userID: " + std::string(userID)).c_str());
            headers = curl_slist_append(headers, ("token: " + std::string(token)).c_str());

            std::string dedicatedServerString;
            dedicatedServer ? dedicatedServerString = "True" : "False";
            headers = curl_slist_append(headers, ("dedicatedServer: " + std::string(dedicatedServerString)).c_str());

            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                fprintf(stderr, "Request Failed . Curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                httpResponse.requestSuccessful = false;
                std::cout << "Denate Request Failed" << std::endl;
                return result;
            }
            else {
                fprintf(stdout, "Request Successful . Curl_easy_perform() Success: %s\n", curl_easy_strerror(res));
                httpResponse.requestSuccessful = true;
                std::cout << "Denate Request Successful" << std::endl;
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }
        //std::cout << readBuffer << std::endl;

        nlohmann::json jsonResponse = nlohmann::json::parse(readBuffer);

        httpResponse.status_code = jsonResponse["status"];
        httpResponse.message = jsonResponse["message"];

        if (jsonResponse.contains("response"))
        {
            inviteAccepted = true;
            
            matchDetails.filters = !jsonResponse["response"]["filter"].is_null() ? jsonResponse["response"]["filter"] : "";
            matchDetails.ipAddress = !jsonResponse["response"]["IP_address"].is_null() ? jsonResponse["response"]["IP_address"] : "";
            matchDetails.serverName = !jsonResponse["response"]["server_name"].is_null() ? jsonResponse["response"]["server_name"] : "";
            matchDetails.playerName = !jsonResponse["response"]["player_name"].is_null() ? jsonResponse["response"]["player_name"] : "";
            matchDetails.mapName = !jsonResponse["response"]["map_name"].is_null() ? jsonResponse["response"]["map_name"] : "";

            if (jsonResponse["response"]["max_players"].is_null() && jsonResponse["response"].contains("max_players"))
            {
                matchDetails.maxPlayers = jsonResponse["response"]["max_players"];
            } 

        }

        result.httpResponse = httpResponse;
        result.inviteAccepted = inviteAccepted;
        result.matchDetails = matchDetails;

        curl_global_cleanup();

        return result;
    }

    DeclineInviteResult DOS_Friends::DeclineInvite(std::string friendName)
    {
        CURL* curl;
        CURLcode res;
        DeclineInviteResult result;
        std::string readBuffer;
        bool inviteDeclined = false;
        DenateHTTPResponse httpResponse;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            std::string playerName = userDetails.username;

            replaceSubstring(playerName, " ", "%20");
            replaceSubstring(friendName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denatefriends/declineinvite/") + std::string(playerName) + "/" + std::string(friendName));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            struct curl_slist* headers = NULL;
            //headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("appID: " + std::string(appID)).c_str());
            headers = curl_slist_append(headers, ("userID: " + std::string(userID)).c_str());
            headers = curl_slist_append(headers, ("token: " + std::string(token)).c_str());

            std::string dedicatedServerString;
            dedicatedServer ? dedicatedServerString = "True" : "False";
            headers = curl_slist_append(headers, ("dedicatedServer: " + std::string(dedicatedServerString)).c_str());

            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                fprintf(stderr, "Request Failed . Curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                httpResponse.requestSuccessful = false;
                std::cout << "Denate Request Failed" << std::endl;
                return result;
            }
            else {
                fprintf(stdout, "Request Successful . Curl_easy_perform() Success: %s\n", curl_easy_strerror(res));
                httpResponse.requestSuccessful = true;
                std::cout << "Denate Request Successful" << std::endl;
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }
        //std::cout << readBuffer << std::endl;

        nlohmann::json jsonResponse = nlohmann::json::parse(readBuffer);

        httpResponse.status_code = jsonResponse["status"];
        httpResponse.message = jsonResponse["message"];

        if (jsonResponse.contains("response"))
        {
            inviteDeclined = true;
        }

        result.httpResponse = httpResponse;
        result.inviteDeclined = inviteDeclined;

        curl_global_cleanup();

        return result;
    }

    GetInvitesResultResult DOS_Friends::GetInvites()
    {

        CURL* curl;
        CURLcode res;
        GetInvitesResultResult result;
        std::string readBuffer;
        bool gottenInvites = false;
        DenateHTTPResponse httpResponse;
        std::vector<DenateFriendDetails> frienddetails;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            
            std::string playerName = userDetails.username;

            replaceSubstring(playerName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denatefriends/getinvites/") + std::string(playerName));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);


            struct curl_slist* headers = NULL;
            //headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("appID: " + std::string(appID)).c_str());
            headers = curl_slist_append(headers, ("userID: " + std::string(userID)).c_str());
            headers = curl_slist_append(headers, ("token: " + std::string(token)).c_str());

            std::string dedicatedServerString;
            dedicatedServer ? dedicatedServerString = "True" : "False";
            headers = curl_slist_append(headers, ("dedicatedServer: " + std::string(dedicatedServerString)).c_str());

            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                fprintf(stderr, "Request Failed . Curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                httpResponse.requestSuccessful = false;
                std::cout << "Denate Request Failed" << std::endl;
                return result;
            }
            else {
                fprintf(stdout, "Request Successful . Curl_easy_perform() Success: %s\n", curl_easy_strerror(res));
                httpResponse.requestSuccessful = true;
                std::cout << "Denate Request Successful" << std::endl;
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }
        //std::cout << readBuffer << std::endl;

        nlohmann::json jsonResponse = nlohmann::json::parse(readBuffer);

        httpResponse.status_code = jsonResponse["status"];
        httpResponse.message = jsonResponse["message"];

        if (jsonResponse.contains("response"))
        {
            gottenInvites = true;

            if (jsonResponse["response"].is_array())
            {
                for (const auto& detail : jsonResponse["response"])
                {

                    DenateFriendDetails frienddetail;

                    frienddetail.playerName = !detail["player_name"].is_null() ? detail["player_name"] : "";
                    frienddetail.appUserID = !detail["player_name_appuserID"].is_null() ? detail["player_name_appuserID"] : "";
                    frienddetail.picture = !detail["picture"].is_null() ? detail["picture"] : "";

                    if (!detail["ingame"].is_null())
                    {
                        frienddetail.online = detail["ingame"];
                    }

                    if (detail.contains("online"))
                    {
                        if (!detail["online"].is_null())
                        {
                            frienddetail.online = detail["online"];
                        }
                    }
                    else {
                        frienddetail.online = false;
                    }

                    frienddetails.push_back(frienddetail);

                }
            }

        }

        result.httpResponse = httpResponse;
        result.gottenInvites = gottenInvites;
        result.inviteDetails = frienddetails;

        curl_global_cleanup();

        return result;

    }

    std::vector<DenateFilterResult> BreakDenateFilter(std::string Filter)
    {
        std::vector<DenateFilterResult> result;
        std::vector<std::string> brokendownfilters;
        brokendownfilters = parseIntoVector(Filter, ",");
        for (int i = 0; i < brokendownfilters.size(); i++)
        {
            std::vector<std::string> titleandvalue;
            titleandvalue = parseIntoVector(brokendownfilters[i], "=");

            DenateFilterResult localresult;
            localresult.title = titleandvalue[0];
            localresult.value = titleandvalue[1];

            result.push_back(localresult);

        }
        return result;
    }

    std::string MakeDenateFilter(std::vector<DenateFilterResult> filters)
    {
        std::string result;
        for (int i = 0; i < filters.size(); i++)
        {
            if (i == filters.size() - 1)
            {
                result = result + filters[i].title + "=" + filters[i].value;
            }
            else {
                result = result + filters[i].title + "=" + filters[i].value + ",";
            }

        }
        return result;
    }

}