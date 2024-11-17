#include "DOS_Voice_Chat.h"
#include <iostream>

namespace DenateVoiceChat
{

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

    DOS_Voice_Chat::DOS_Voice_Chat(std::string userID, std::string appID, bool dedicatedServer, std::string token, DenateUserDetails userDetails)
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

}