#include "DOS_Leaderboard.h"
#include <iostream>
#include <vector>

namespace DenateLeaderboard
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

    void DOS_Leaderboard::replaceSubstring(std::string& original, const std::string& toReplace, const std::string& replacement)
    {
        size_t pos = 0;
        while ((pos = original.find(toReplace, pos)) != std::string::npos)
        {
            original.replace(pos, toReplace.length(), replacement);
            pos += replacement.length();
        }
    }

    DOS_Leaderboard::DOS_Leaderboard(std::string userID, std::string appID, bool dedicatedServer, std::string token, DenateUserDetails userDetails)
    {
        this->userID = userID;
        this->appID = appID;
        this->dedicatedServer = dedicatedServer;
        this->token = token;
        this->userDetails = userDetails;
    }

    AddPlayerLeaderboardResult DOS_Leaderboard::AddPlayerLeaderboard(std::string leaderboardName, std::string filters)
    {

        CURL* curl;
        CURLcode res;
        AddPlayerLeaderboardResult result;
        std::string readBuffer;
        bool leaderboardAdded = false;
        DenateHTTPResponse httpResponse;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            //replaceSubstring(Tag, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("leaderboard/addleaderboard"));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            std::string playerName = userDetails.username;

            /*replaceSubstring(playerName, " ", "%20");
            replaceSubstring(leaderboardName, " ", "%20");*/

            std::string json = R"({"appID": ")" + appID + "\"" + "," + R"("userID": ")" + userID + "\"" + "," + R"("player_name": ")" + userDetails.username + "\"" + "," + R"("leaderboard_name": ")" + leaderboardName + "\"" + "," + R"("appuserID": ")" + userDetails.appUserID + "\"" + "," + R"("filters": ")" + filters + R"("})";
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
            leaderboardAdded = true;
        }

        result.httpResponse = httpResponse;
        result.leaderboardAdded = leaderboardAdded;

        curl_global_cleanup();

        return result;

    }

    GetPlayerLeaderboardResult DOS_Leaderboard::GetPlayerLeaderboard(std::string leaderboardName, std::string playerName)
    {
        CURL* curl;
        CURLcode res;
        GetPlayerLeaderboardResult result;
        std::string readBuffer;
        bool gottenLeaderboard = false;
        DenateHTTPResponse httpResponse;
        std::string filters;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            //replaceSubstring(Tag, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("leaderboard/getleaderboardbyplayername"));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            if (playerName == "")
            {
                playerName = userDetails.username;
            }

            /*replaceSubstring(playerName, " ", "%20");
            replaceSubstring(leaderboardName, " ", "%20");*/

            std::string json = R"({"leaderboard_name": ")" + leaderboardName + "\"" + "," + R"("player_name": ")" + playerName + R"("})";
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
            gottenLeaderboard = true;
            if (jsonResponse["response"].contains("filters"))
            {
                filters = !jsonResponse["response"]["filters"].is_null() ? jsonResponse["response"]["filters"] : "";
            }
        }

        result.httpResponse = httpResponse;
        result.gottenLeaderboard = gottenLeaderboard;
        result.leaderboardFilters = filters;

        curl_global_cleanup();

        return result;

    }

    GetAllLeaderboardResult DOS_Leaderboard::GetAllLeaderboard(std::string leaderboardName)
    {
        CURL* curl;
        CURLcode res;
        GetAllLeaderboardResult result;
        std::string readBuffer;
        bool gottenLeaderboard = false;
        DenateHTTPResponse httpResponse;
        std::vector<DenateLeaderboardResult> filters;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            replaceSubstring(leaderboardName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("leaderboard/getleaderboardbyname/") + std::string(leaderboardName));
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
            gottenLeaderboard = true;

            if (jsonResponse["response"].is_array())
            {
                for (const auto& leaderboard : jsonResponse["response"])
                {
                    DenateLeaderboardResult localleaderboard;
                    if (leaderboard.contains("filters"))
                    {
                        localleaderboard.filters = !leaderboard["filters"].is_null() ? leaderboard["filters"] : "";
                    }
                    if (leaderboard.contains("leaderboard_name"))
                    {
                        localleaderboard.LeaderboardName = !leaderboard["leaderboard_name"].is_null() ? leaderboard["leaderboard_name"] : "";
                    }
                    if (leaderboard.contains("player_name"))
                    {
                        localleaderboard.playerName = !leaderboard["player_name"].is_null() ? leaderboard["player_name"] : "";
                    }
                    filters.push_back(localleaderboard);
                }
            }
        }

        result.httpResponse = httpResponse;
        result.gottenLeaderboard = gottenLeaderboard;
        result.leaderboardFilters = filters;

        curl_global_cleanup();

        return result;
    }

    GetAllPlayerLeaderboardResult DOS_Leaderboard::GetAllPlayerLeaderboard(std::string playerName)
    {
        CURL* curl;
        CURLcode res;
        GetAllPlayerLeaderboardResult result;
        std::string readBuffer;
        bool gottenLeaderboard = false;
        DenateHTTPResponse httpResponse;
        std::vector<DenateLeaderboardResult> filters;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            if (playerName == "")
            {
                playerName = userDetails.username;
            }

            replaceSubstring(playerName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("leaderboard/getallplayerleaderboard/") + std::string(playerName));
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
            gottenLeaderboard = true;

            if (jsonResponse["response"].is_array())
            {
                for (const auto& leaderboard : jsonResponse["response"])
                {
                    DenateLeaderboardResult localleaderboard;
                    if (leaderboard.contains("filters"))
                    {
                        localleaderboard.filters = !leaderboard["filters"].is_null() ? leaderboard["filters"] : "";
                    }
                    if (leaderboard.contains("leaderboard_name"))
                    {
                        localleaderboard.LeaderboardName = !leaderboard["leaderboard_name"].is_null() ? leaderboard["leaderboard_name"] : "";
                    }
                    if (leaderboard.contains("player_name"))
                    {
                        localleaderboard.playerName = !leaderboard["player_name"].is_null() ? leaderboard["player_name"] : "";
                    }
                    filters.push_back(localleaderboard);
                }
            }
        }

        result.httpResponse = httpResponse;
        result.gottenLeaderboard = gottenLeaderboard;
        result.leaderboardFilters = filters;

        curl_global_cleanup();

        return result;
    }

    std::vector<DenateFilterResult> DOS_Leaderboard::BreakDenateFilter(std::string Filter)
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

    std::string DOS_Leaderboard::MakeDenateFilter(std::vector<DenateFilterResult> filters)
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