#include "DOS_Voice_Chat.h"
#include <iostream>
#include <thread>

namespace DenateVoiceChat
{

    std::vector<std::string> parseIntoVector(const std::string& str, const std::string& delimiter) {
        std::vector<std::string> filters;
        size_t start = 0;
        size_t end = str.find(delimiter);

        while (end != std::string::npos) {
            filters.push_back(str.substr(start, end - start));
            start = end + delimiter.length();
            end = str.find(delimiter, start);
        }
        filters.push_back(str.substr(start));
        return filters;
    }

    size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
    {

        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;

    }

    void DOS_Voice_Chat::replaceSubstring(std::string& original, const std::string& toReplace, const std::string& replacement)
    {
        size_t pos = 0;
        while ((pos = original.find(toReplace, pos)) != std::string::npos)
        {
            original.replace(pos, toReplace.length(), replacement);
            pos += replacement.length();
        }
    }

    void DOS_Voice_Chat::OnPlayerJoinVoiceChannel(playerJoinVoiceChannelListener const& playerJoinVoiceChannel)
    {
        if (playerJoinVoiceChannel)
        {
            internalPlayerJoinVoiceChannel = playerJoinVoiceChannel;
        }
    }

    void DOS_Voice_Chat::OnPlayerLeftVoiceChannel(playerLeftVoiceChannelListener const& playerLeftVoiceChannel)
    {
        if (playerLeftVoiceChannel)
        {
            internalPlayerLeftVoiceChannel = playerLeftVoiceChannel;
        }
    }

    void DOS_Voice_Chat::OnPlayerDestroyVoiceChannel(playerDestroyVoiceChannelListener const& playerDestroyVoiceChannel)
    {
        if (playerDestroyVoiceChannel)
        {
            internalPlayerDestroyVoiceChannel = playerDestroyVoiceChannel;
        }
    }

    void DOS_Voice_Chat::OnRecieveVoiceData(recieveVoiceDataListener const& recieveVoiceData)
    {
        if (recieveVoiceData)
        {
            internalRecieveVoiceData = recieveVoiceData;
        }
    }

    DOS_Voice_Chat::DOS_Voice_Chat(std::string userID, std::string appID, bool dedicatedServer, std::string token, DenateUserDetails userDetails, DenateConnection::DOS_Connection& denateConnection) : internalDenateConnection(denateConnection)
    {
        this->userID = userID;
        this->appID = appID;
        this->dedicatedServer = dedicatedServer;
        this->token = token;
        this->userDetails = userDetails;
    }

    std::vector<DenateFilterResult> DOS_Voice_Chat::BreakDenateFilter(std::string Filter)
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

    std::string DOS_Voice_Chat::MakeDenateFilter(std::vector<DenateFilterResult> filters)
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

    CreateVoiceChannelResult DOS_Voice_Chat::CreateVoiceChannel()
    {
        CURL* curl;
        CURLcode res;
        CreateVoiceChannelResult result;
        std::string readBuffer;
        bool createdChannel = false;
        DenateHTTPResponse httpResponse;
        std::string channelID;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            //replaceSubstring(Tag, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denatevoicechat/createvoicechannel"));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

            //std::string json = R"({"appID": ")" + appID + "\"" + "," + R"("userID": ")" + userID + "\"" + "," + R"("player_name": ")" + userDetails.username + "\"" + "," + R"("leaderboard_name": ")" + leaderboardName + "\"" + "," + R"("appuserID": ")" + userDetails.appUserID + "\"" + "," + R"("filters": ")" + filters + R"("})";
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
            //std::cout << json << std::endl;

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
            createdChannel = true;

            result.channelID = !jsonResponse["response"]["room"].is_null() ? jsonResponse["response"]["room"] : "";
        }

        result.httpResponse = httpResponse;
        result.createdChannel = createdChannel;
        

        curl_global_cleanup();

        return result;
    }

    JoinVoiceChannelResult DOS_Voice_Chat::JoinVoiceChannel(std::string channelID, std::string playerName)
    {
        CURL* curl;
        CURLcode res;
        JoinVoiceChannelResult result;
        std::string readBuffer;
        bool joined = false;
        DenateHTTPResponse httpResponse;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            if (playerName == "")
            {
                playerName = userDetails.username;
            }

            replaceSubstring(playerName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denatevoicechat/joinvoicechannel"));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

            std::string json = R"({"appID": ")" + appID + "\"" + "," + R"("username": ")" + playerName + "\"" + "," + R"("room": ")" + channelID + R"("})";
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
            //std::cout << json << std::endl;

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
            joined = true;

            for (const auto& channels : jsonResponse["extra"])
            {
                DenateVoiceChannelDetails voicedetails;
                if (channels.contains("client_id"))
                {
                    voicedetails.clientId = !channels["client_id"].is_null() ? channels["client_id"] : "";
                }
                if (channels.contains("room"))
                {
                    voicedetails.channelId = !channels["room"].is_null() ? channels["room"] : "";
                }
                if (channels.contains("username"))
                {
                    voicedetails.playerName = !channels["username"].is_null() ? channels["username"] : "";
                }

                bool found;
                for (int i = 0; i < allConnectedChannels.size(); i++)
                {
                    if (allConnectedChannels[i] == voicedetails) {
                        found = true;
                    }
                }
                if (!found)
                {
                    allConnectedChannels.push_back(voicedetails);
                }
                currentVoiceChannel = voicedetails;
            }

            if (&internalDenateConnection != nullptr)
            {
                if (internalDenateConnection.isDenateOnlineServiceConnected)
                {
                    if (voiceChatActivated)
                    {
                        sio::message::ptr jsonMessage = sio::object_message::create();

                        jsonMessage->get_map()["appID"] = sio::string_message::create(appID);
                        jsonMessage->get_map()["username"] = sio::string_message::create(std::string(jsonResponse["response"]["username"]));
                        jsonMessage->get_map()["room"] = sio::string_message::create(std::string(jsonResponse["response"]["room"]));

                        namespaceSocket->emit("joinchannel", jsonMessage, [&](sio::message::list const& ack_msg) {

                            });
                    }
                    else {
                        std::cout << "Activate the voice chat" << std::endl;
                    }
                }
            }

        }

        result.httpResponse = httpResponse;
        result.joined = joined;


        curl_global_cleanup();

        return result;
    }

    LeaveVoiceChannelResult DOS_Voice_Chat::LeaveVoiceChannel(std::string channelID, std::string playerName)
    {
        CURL* curl;
        CURLcode res;
        LeaveVoiceChannelResult result;
        std::string readBuffer;
        bool leftChannel = false;
        DenateHTTPResponse httpResponse;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            if (playerName == "")
            {
                playerName = userDetails.username;
            }

            replaceSubstring(playerName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denatevoicechat/leavevoicechannel/") + std::string(playerName) + "/" + std::string(channelID));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

            //std::string json = R"({"appID": ")" + appID + "\"" + "," + R"("username": ")" + playerName + "\"" + "," + R"("room": ")" + channelID + R"("})";
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
            //std::cout << json << std::endl;

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
            leftChannel = true;

            
            std::vector<int> indexestodelete;
            for (int i = 0; i < allConnectedChannels.size(); i++)
            {
                if (allConnectedChannels[i].channelId == channelID) {
                    indexestodelete.push_back(i);
                }
            }

            std::sort(indexestodelete.rbegin(), indexestodelete.rend());
            for (int index : indexestodelete)
            { 
                if (index >= 0 && index < allConnectedChannels.size())
                {
                    allConnectedChannels.erase(allConnectedChannels.begin() + index);
                } 
            }

            if (&internalDenateConnection != nullptr)
            {
                if (internalDenateConnection.isDenateOnlineServiceConnected)
                {
                    if (voiceChatActivated)
                    {
                        sio::message::ptr jsonMessage = sio::object_message::create();

                        jsonMessage->get_map()["appID"] = sio::string_message::create(appID);
                        jsonMessage->get_map()["username"] = sio::string_message::create(std::string(jsonResponse["response"]["username"]));
                        jsonMessage->get_map()["room"] = sio::string_message::create(std::string(jsonResponse["response"]["room"]));

                        namespaceSocket->emit("leavechannel", jsonMessage, [&](sio::message::list const& ack_msg) {

                            });
                    }
                    else {
                        std::cout << "Activate the voice chat" << std::endl;
                    }
                }
            }

        }

        result.httpResponse = httpResponse;
        result.leftChannel = leftChannel;


        curl_global_cleanup();

        return result;
    }

    DestroyVoiceChannelResult DOS_Voice_Chat::DestroyVoiceChannel(std::string channelID)
    {
        CURL* curl;
        CURLcode res;
        DestroyVoiceChannelResult result;
        std::string readBuffer;
        bool destroyedChannel = false;
        DenateHTTPResponse httpResponse;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denatevoicechat/destroyvoicechannel/") + std::string(channelID));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

            //std::string json = R"({"appID": ")" + appID + "\"" + "," + R"("username": ")" + playerName + "\"" + "," + R"("room": ")" + channelID + R"("})";
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
            //std::cout << json << std::endl;

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
            destroyedChannel = true;

            std::vector<int> indexestodelete;
            for (int i = 0; i < allConnectedChannels.size(); i++)
            {
                if (allConnectedChannels[i].channelId == channelID) {
                    indexestodelete.push_back(i);
                }
            }

            std::sort(indexestodelete.rbegin(), indexestodelete.rend());
            for (int index : indexestodelete)
            {
                if (index >= 0 && index < allConnectedChannels.size())
                {
                    allConnectedChannels.erase(allConnectedChannels.begin() + index);
                }
            }

            if (&internalDenateConnection != nullptr)
            {
                if (internalDenateConnection.isDenateOnlineServiceConnected)
                {
                    if (voiceChatActivated)
                    {
                        sio::message::ptr jsonMessage = sio::object_message::create();

                        jsonMessage->get_map()["appID"] = sio::string_message::create(appID);
                        jsonMessage->get_map()["username"] = sio::string_message::create(std::string(userDetails.username));
                        jsonMessage->get_map()["room"] = sio::string_message::create(std::string(jsonResponse["response"]["room"]));

                        namespaceSocket->emit("destroychannel", jsonMessage, [&](sio::message::list const& ack_msg) {

                            });
                    }
                    else {
                        std::cout << "Activate the voice chat" << std::endl;
                    }
                }
            }

        }

        result.httpResponse = httpResponse;
        result.destroyedChannel = destroyedChannel;


        curl_global_cleanup();

        return result;
    }

    GetAllVoiceConnectedPlayersResult DOS_Voice_Chat::GetAllVoiceConnectedPlayers(std::string channelID)
    {
        CURL* curl;
        CURLcode res;
        GetAllVoiceConnectedPlayersResult result;
        std::string readBuffer;
        bool gottenplayers = false;
        DenateHTTPResponse httpResponse;
        std::vector<DenateVoiceChannelDetails> voicechannels;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denatevoicechat/getvoiceconnectedplayers/") + std::string(channelID));
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
            gottenplayers = true;

            if (jsonResponse["response"].is_array())
            {
                for (const auto& channels : jsonResponse["response"])
                {
                    DenateVoiceChannelDetails voicedetails;
                    if (channels.contains("client_id"))
                    {
                        voicedetails.clientId = !channels["client_id"].is_null() ? channels["client_id"] : "";
                    }
                    if (channels.contains("room"))
                    {
                        voicedetails.channelId = !channels["room"].is_null() ? channels["room"] : "";
                    }
                    if (channels.contains("username"))
                    {
                        voicedetails.playerName = !channels["username"].is_null() ? channels["username"] : "";
                    }
                    voicechannels.push_back(voicedetails);
                }
            }
        }

        result.httpResponse = httpResponse;
        result.gottenPlayers = gottenplayers;
        result.voiceChannels = voicechannels;

        curl_global_cleanup();

        return result;
    }

    GetAllPlayerConnectedChannelsResult DOS_Voice_Chat::GetAllPlayerConnectedChannels(std::string playerName)
    {
        CURL* curl;
        CURLcode res;
        GetAllPlayerConnectedChannelsResult result;
        std::string readBuffer;
        bool gottenChannels = false;
        DenateHTTPResponse httpResponse;
        std::vector<DenatePlayerChannelDetails> playerChannelDetails;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            if (playerName == "")
            {
                playerName = userDetails.username;
            }

            replaceSubstring(playerName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denatevoicechat/getallplayerconnectedchannels/") + std::string(playerName));
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
            gottenChannels = true;

            if (jsonResponse["response"].is_array())
            {
                for (const auto& channels : jsonResponse["response"])
                {
                    DenatePlayerChannelDetails voicedetails;
                    if (channels.contains("client_id"))
                    {
                        voicedetails.clientId = !channels["client_id"].is_null() ? channels["client_id"] : "";
                    }
                    if (channels.contains("room"))
                    {
                        voicedetails.channelId = !channels["room"].is_null() ? channels["room"] : "";
                    }
                    
                    playerChannelDetails.push_back(voicedetails);
                }
            }
        }

        result.httpResponse = httpResponse;
        result.gottenChannels = gottenChannels;
        result.playerChannelDetails = playerChannelDetails;

        curl_global_cleanup();

        return result;
    }

    void DOS_Voice_Chat::Activate()
    {
        namespaceSocket = internalDenateConnection.sioClient.socket("/voicegateway");
        if (namespaceSocket)
        {
            voiceChatActivated = true;

            namespaceSocket->on("joinedchannel", [&](sio::event& ev) {

                std::cout << "Player just joined a channel you are a part of" << std::endl;

                DenateVoiceChannelDetails channelDetails;
                std::string playerName;
                std::string clientId;
                std::string room;

                auto data = ev.get_message();

                if (data->get_flag() == sio::message::flag_object)
                {
                    auto jsonObj = data->get_map();
                    if (jsonObj.find("username") != jsonObj.end())
                    {
                        playerName = jsonObj["username"]->get_string();
                    }
                    if (jsonObj.find("client_id") != jsonObj.end())
                    {
                        clientId = jsonObj["client_id"]->get_string();
                    }
                    if (jsonObj.find("room") != jsonObj.end())
                    {
                        room = jsonObj["room"]->get_string();
                    }
                }

                channelDetails.clientId = clientId;
                channelDetails.channelId = room;
                channelDetails.playerName = playerName;

                bool found = false;
                for (int i = 0; i < allConnectedChannels.size(); i++)
                {
                    if (allConnectedChannels[i] == channelDetails) {
                        found = true;
                    }
                }
                if (!found)
                {
                    allConnectedChannels.push_back(channelDetails);
                }

                if (internalPlayerJoinVoiceChannel)
                {
                    internalPlayerJoinVoiceChannel(channelDetails);
                }

                });

            namespaceSocket->on("destroyedchannel", [&](sio::event& ev) {

                std::cout << "A channel you are currently a part of just got destroyed" << std::endl;

                std::string room;

                auto data = ev.get_message();

                if (data->get_flag() == sio::message::flag_object)
                {
                    auto jsonObj = data->get_map();

                    if (jsonObj.find("room") != jsonObj.end())
                    {
                        room = jsonObj["room"]->get_string();
                    }
                }

                std::vector<int> indexestodelete;
                for (int i = 0; i < allConnectedChannels.size(); i++)
                {
                    if (allConnectedChannels[i].channelId == room) {
                        indexestodelete.push_back(i);
                    }
                }

                std::sort(indexestodelete.rbegin(), indexestodelete.rend());
                for (int index : indexestodelete)
                {
                    if (index >= 0 && index < allConnectedChannels.size())
                    {
                        allConnectedChannels.erase(allConnectedChannels.begin() + index);
                    }
                }

                if (internalPlayerDestroyVoiceChannel)
                {
                    internalPlayerDestroyVoiceChannel(room);
                }

                });

            namespaceSocket->on("leftchannel", [&](sio::event& ev) {

                std::cout << "A Player just left a channel you are a part of" << std::endl;

                DenateVoiceChannelDetails channelDetails;
                std::string playerName;
                std::string clientId;
                std::string room;

                auto data = ev.get_message();

                if (data->get_flag() == sio::message::flag_object)
                {
                    auto jsonObj = data->get_map();
                    if (jsonObj.find("username") != jsonObj.end())
                    {
                        playerName = jsonObj["username"]->get_string();
                    }
                    if (jsonObj.find("client_id") != jsonObj.end())
                    {
                        clientId = jsonObj["client_id"]->get_string();
                    }
                    if (jsonObj.find("room") != jsonObj.end())
                    {
                        room = jsonObj["room"]->get_string();
                    }
                }

                std::vector<int> indexestodelete;
                for (int i = 0; i < allConnectedChannels.size(); i++)
                {
                    if (allConnectedChannels[i].channelId == room) {
                        indexestodelete.push_back(i);
                    }
                }

                std::sort(indexestodelete.rbegin(), indexestodelete.rend());
                for (int index : indexestodelete)
                {
                    if (index >= 0 && index < allConnectedChannels.size())
                    {
                        allConnectedChannels.erase(allConnectedChannels.begin() + index);
                    }
                }

                channelDetails.clientId = clientId;
                channelDetails.channelId = room;
                channelDetails.playerName = playerName;

                if (internalPlayerLeftVoiceChannel)
                {
                    internalPlayerLeftVoiceChannel(channelDetails);
                }

                });

            namespaceSocket->on("voicemessagesent", [&](sio::event& ev) {

                std::string playerName;
                std::vector<int> audioDataArray;
                auto data = ev.get_message();

                if (data->get_flag() == sio::message::flag_object)
                {
                    auto jsonObj = data->get_map();

                    if (jsonObj.find("from_player") != jsonObj.end())
                    {
                        playerName = jsonObj["from_player"]->get_string();
                    }
                    if (jsonObj.find("audioData") != jsonObj.end() && jsonObj["audioData"]->get_flag() == sio::message::flag_array) 
                    {
                        auto audioData = jsonObj["audioData"]->get_vector(); 
                        for (const auto& item : audioData) 
                        { 
                            if (item->get_flag() == sio::message::flag_object)
                            {
                                //audioDataArray.push_back(item->get_int());

                                auto newjsonObj = item->get_map();

                                if (newjsonObj.find("data") != newjsonObj.end())
                                {
                                    int data = newjsonObj["data"]->get_int();
                                    audioDataArray.push_back(data);
                                }

                            }
                        }
                    }
                }

                if (internalRecieveVoiceData)
                {
                    internalRecieveVoiceData(playerName, audioDataArray);
                }

                });

        }
    }

    void DOS_Voice_Chat::Deactivate()
    {
        namespaceSocket->close();
        
        voiceChatActivated = false;
    }

    void DOS_Voice_Chat::sendAudioData(std::vector<int> audioData, std::vector<std::string> clients)
    {
        
        if (&internalDenateConnection != nullptr)
        {
            if (internalDenateConnection.isDenateOnlineServiceConnected)
            {
                if (voiceChatActivated)
                {
                    sio::message::ptr audioDataArray = sio::array_message::create();
                    for (const auto& dataValue : audioData) {
                        sio::message::ptr dataObject = sio::object_message::create();
                        dataObject->get_map()["data"] = sio::int_message::create(dataValue);
                        audioDataArray->get_vector().push_back(dataObject);

                    }

                    sio::message::ptr clientArray = sio::array_message::create();
                    for (const auto& dataValue : clients) {
                        sio::message::ptr dataObject = sio::object_message::create();
                        dataObject->get_map()["client_id"] = sio::string_message::create(dataValue);
                        clientArray->get_vector().push_back(dataObject);

                    }

                    sio::message::ptr jsonMessage = sio::object_message::create();

                    jsonMessage->get_map()["bufferSize"] = sio::int_message::create(audioData.size());
                    jsonMessage->get_map()["from_player"] = sio::string_message::create(std::string(userDetails.username));
                    jsonMessage->get_map()["room"] = sio::string_message::create(std::string(currentVoiceChannel.channelId));
                    jsonMessage->get_map()["audioData"] = audioDataArray;
                    jsonMessage->get_map()["clients"] = clientArray;

                    namespaceSocket->emit("voicemessage", jsonMessage, [&](sio::message::list const& ack_msg) {

                        });

                    
                }
                else {
                    std::cout << "Activate the voice chat" << std::endl;
                }
            }
        }

    }


}