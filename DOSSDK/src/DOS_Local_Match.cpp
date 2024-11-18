#include "DOS_Local_Match.h"
#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif
namespace DenateLocalMatch
{

    std::string getHostName() {
        char hostname[256];

#if defined(_WIN32) || defined(_WIN64)
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            return "Error initializing Winsock";
        }
        if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) {
            WSACleanup();
            return "Error getting hostname";
        }
        WSACleanup();
#else
        if (gethostname(hostname, sizeof(hostname)) != 0) {
            return "Error getting hostname";
        }
#endif

        return std::string(hostname);
    }

    std::vector<std::string> getIPAddresses() {
        std::vector<std::string> ipAddresses;
        char hostname[256];

#if defined(_WIN32) || defined(_WIN64)
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            return { "Error initializing Winsock" };
        }
        if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) {
            WSACleanup();
            return { "Error getting hostname" };
        }
#else
        if (gethostname(hostname, sizeof(hostname)) != 0) {
            return { "Error getting hostname" };
        }
#endif

        struct addrinfo hints, * res, * p;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        if (getaddrinfo(hostname, NULL, &hints, &res) != 0) {
            return { "Error getting address info" };
        }

        for (p = res; p != NULL; p = p->ai_next) {
            void* addr;
            char ipstr[INET6_ADDRSTRLEN];

            if (p->ai_family == AF_INET) { // IPv4
                struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;
                addr = &(ipv4->sin_addr);
            }
            else { // IPv6
                struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)p->ai_addr;
                addr = &(ipv6->sin6_addr);
            }

            inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
            ipAddresses.push_back(std::string(ipstr));
        }

        freeaddrinfo(res);

#if defined(_WIN32) || defined(_WIN64)
        WSACleanup();
#endif

        return ipAddresses;
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

    size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
    {

        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;

    }

    void DOS_Local_Match::replaceSubstring(std::string& original, const std::string& toReplace, const std::string& replacement)
    {
        size_t pos = 0;
        while ((pos = original.find(toReplace, pos)) != std::string::npos)
        {
            original.replace(pos, toReplace.length(), replacement);
            pos += replacement.length();
        }
    }

    DOS_Local_Match::DOS_Local_Match(std::string userID, std::string appID, bool dedicatedServer, std::string token, DenateUserDetails userDetails)
    {
        this->userID = userID;
        this->appID = appID;
        this->dedicatedServer = dedicatedServer;
        this->token = token;
        this->userDetails = userDetails;
    }

    std::vector<DenateFilterResult> DOS_Local_Match::BreakDenateFilter(std::string Filter)
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

    std::string DOS_Local_Match::MakeDenateFilter(std::vector<DenateFilterResult> filters)
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

    HostDenateLocalMatchResult DOS_Local_Match::HostDenateLocalMatch(std::string mapName, std::string filters, int maxPlayers, std::string serverName, std::string playerName)
    {

        CURL* curl;
        CURLcode res;
        HostDenateLocalMatchResult result;
        std::string readBuffer;
        bool matchCreated = false;
        DenateHTTPResponse httpResponse;
        DenateMatchDetails matchDetails;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            //replaceSubstring(Tag, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("localmatch/createlocalmatch"));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            if (playerName == "")
            {
                playerName = userDetails.username;
            }

            if (serverName == "")
            {
                serverName = getHostName();
            }
            
            std::vector<std::string> ipAddresses = getIPAddresses();
            
            std::string json = R"({"appID": ")" + appID + "\"" + "," + R"("userID": ")" + userID + "\"" + "," + R"("player_name": ")" + playerName + "\"" + "," + R"("server_name": ")" + serverName + "\"" + "," + R"("IP_address": ")" + ipAddresses[1] + "\"" + "," + R"("map_name": ")" + mapName + "\"" + "," + R"("max_players": )" + std::to_string(maxPlayers) + "," + R"("filter": ")" + filters + R"("})";
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
            
            matchCreated = true;

            if (!jsonResponse["response"].is_null())
            {
                
                matchDetails.filters = !jsonResponse["response"]["filter"].is_null() ? jsonResponse["response"]["filter"] : "";
                matchDetails.ipAddress = !jsonResponse["response"]["IP_address"].is_null() ? jsonResponse["response"]["IP_address"] : "";
                matchDetails.mapName = !jsonResponse["response"]["map_name"].is_null() ? jsonResponse["response"]["map_name"] : "";
                matchDetails.playerName = !jsonResponse["response"]["player_name"].is_null() ? jsonResponse["response"]["player_name"] : "";
                matchDetails.serverName = !jsonResponse["response"]["server_name"].is_null() ? jsonResponse["response"]["server_name"] : "";
                if (jsonResponse["response"].contains("local_match_id") && !jsonResponse["response"]["local_match_id"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["local_match_id"];
                }
                if (jsonResponse["response"].contains("max_players") && !jsonResponse["response"]["max_players"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["max_players"];
                }
                
                currentMatchDetail = matchDetails;
                isPrivateMatch = false;
                isServer = true;

            }
        }

        result.httpResponse = httpResponse;
        result.matchCreated = matchCreated;
        result.matchDetails = matchDetails;

        curl_global_cleanup();

        return result;

    }

    HostDenatePrivateLocalMatchResult DOS_Local_Match::HostDenatePrivateLocalMatch(std::string mapName, std::string filters, int maxPlayers, std::string serverName, std::string playerName)
    {

        CURL* curl;
        CURLcode res;
        HostDenatePrivateLocalMatchResult result;
        std::string readBuffer;
        bool matchCreated = false;
        DenateHTTPResponse httpResponse;
        DenatePrivateMatchDetails matchDetails;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            //replaceSubstring(Tag, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("privatematch/hostprivatematch"));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            if (playerName == "")
            {
                playerName = userDetails.username;
            }

            if (serverName == "")
            {
                serverName = getHostName();
            }

            std::vector<std::string> ipAddresses = getIPAddresses();

            std::string json = R"({"appID": ")" + appID + "\"" + "," + R"("userID": ")" + userID + "\"" + "," + R"("player_name": ")" + playerName + "\"" + "," + R"("server_name": ")" + serverName + "\"" + "," + R"("IP_address": ")" + ipAddresses[1] + "\"" + "," + R"("map_name": ")" + mapName + "\"" + "," + R"("max_players": )" + std::to_string(maxPlayers) + "," + R"("filter": ")" + filters + R"("})";
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
            matchCreated = true;

            if (!jsonResponse["response"].is_null())
            {
                matchDetails.filters = !jsonResponse["response"]["filter"].is_null() ? jsonResponse["response"]["filter"] : "";
                matchDetails.ipAddress = !jsonResponse["response"]["IP_address"].is_null() ? jsonResponse["response"]["IP_address"] : "";
                matchDetails.mapName = !jsonResponse["response"]["map_name"].is_null() ? jsonResponse["response"]["map_name"] : "";
                matchDetails.playerName = !jsonResponse["response"]["player_name"].is_null() ? jsonResponse["response"]["player_name"] : "";
                matchDetails.serverName = !jsonResponse["response"]["server_name"].is_null() ? jsonResponse["response"]["server_name"] : "";
                matchDetails.gameCode = !jsonResponse["response"]["game_code"].is_null() ? jsonResponse["response"]["game_code"] : "";
                if (jsonResponse["response"].contains("private_match_id") && !jsonResponse["response"]["private_match_id"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["private_match_id"];
                }
                if (jsonResponse["response"].contains("max_players") && !jsonResponse["response"]["max_players"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["max_players"];
                }


                currentPrivateMatchDetail = matchDetails;
                isPrivateMatch = true;
                isServer = true;

            }
        }

        result.httpResponse = httpResponse;
        result.matchCreated = matchCreated;
        result.privateMatchDetails = matchDetails;

        curl_global_cleanup();

        return result;

    }

    JoinDenateLocalMatchResult DOS_Local_Match::JoinDenateLocalMatchByServerName(std::string serverName, std::string playerName)
    {

        CURL* curl;
        CURLcode res;
        JoinDenateLocalMatchResult result;
        std::string readBuffer;
        bool matchJoined = false;
        DenateHTTPResponse httpResponse;
        DenateMatchDetails matchDetails;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            if (playerName == "")
            {
                playerName = userDetails.username;
            }

            replaceSubstring(playerName, " ", "%20");
            replaceSubstring(serverName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("localclientmatch/joinlocalclientmatchbyservername/") + std::string(serverName) + "/" + std::string(appID) + "/" + std::string(playerName));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            /*std::string json = R"({"appID": ")" + appID + "\"" + "," + R"("userID": ")" + userID + "\"" + "," + R"("player_name": ")" + playerName + "\"" + "," + R"("server_name": ")" + serverName + "\"" + "," + R"("IP_address": ")" + ipAddresses[1] + "\"" + "," + R"("map_name": ")" + mapName + "\"" + "," + R"("max_players": )" + std::to_string(maxPlayers) + "," + R"("filter": ")" + filters + R"("})";
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
            std::cout << json << std::endl;*/

            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

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
            matchJoined = true;

            if (!jsonResponse["response"].is_null())
            {
                
                matchDetails.filters = !jsonResponse["response"]["filter"].is_null() ? jsonResponse["response"]["filter"] : "";
                matchDetails.ipAddress = !jsonResponse["response"]["IP_address"].is_null() ? jsonResponse["response"]["IP_address"] : "";
                matchDetails.mapName = !jsonResponse["response"]["map_name"].is_null() ? jsonResponse["response"]["map_name"] : "";
                matchDetails.playerName = !jsonResponse["response"]["player_name"].is_null() ? jsonResponse["response"]["player_name"] : "";
                matchDetails.serverName = !jsonResponse["response"]["server_name"].is_null() ? jsonResponse["response"]["server_name"] : "";
                if (jsonResponse["response"].contains("local_client_match_id") && !jsonResponse["response"]["local_client_match_id"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["local_client_match_id"];
                }
                if (jsonResponse["response"].contains("max_players") && !jsonResponse["response"]["max_players"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["max_players"];
                }

                currentMatchDetail = matchDetails;
                isPrivateMatch = false;
                isServer = false;

            }
        }

        result.httpResponse = httpResponse;
        result.matchJoined = matchJoined;
        result.matchDetails = matchDetails;

        curl_global_cleanup();

        return result;


    }

    JoinDenateLocalMatchResult DOS_Local_Match::JoinDenateLocalMatchByPlayerName(std::string playerToFind, std::string playerName)
    {

        CURL* curl;
        CURLcode res;
        JoinDenateLocalMatchResult result;
        std::string readBuffer;
        bool matchJoined = false;
        DenateHTTPResponse httpResponse;
        DenateMatchDetails matchDetails;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            if (playerName == "")
            {
                playerName = userDetails.username;
            }

            replaceSubstring(playerName, " ", "%20");
            replaceSubstring(playerToFind, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("localclientmatch/joinlocalclientmatchbyplayername/") + std::string(playerToFind) + "/" + std::string(appID) + "/" + std::string(playerName));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            /*std::string json = R"({"appID": ")" + appID + "\"" + "," + R"("userID": ")" + userID + "\"" + "," + R"("player_name": ")" + playerName + "\"" + "," + R"("server_name": ")" + serverName + "\"" + "," + R"("IP_address": ")" + ipAddresses[1] + "\"" + "," + R"("map_name": ")" + mapName + "\"" + "," + R"("max_players": )" + std::to_string(maxPlayers) + "," + R"("filter": ")" + filters + R"("})";
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
            std::cout << json << std::endl;*/

            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

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
            matchJoined = true;

            if (!jsonResponse["response"].is_null())
            {
                matchDetails.filters = !jsonResponse["response"]["filter"].is_null() ? jsonResponse["response"]["filter"] : "";
                matchDetails.ipAddress = !jsonResponse["response"]["IP_address"].is_null() ? jsonResponse["response"]["IP_address"] : "";
                matchDetails.mapName = !jsonResponse["response"]["map_name"].is_null() ? jsonResponse["response"]["map_name"] : "";
                matchDetails.playerName = !jsonResponse["response"]["player_name"].is_null() ? jsonResponse["response"]["player_name"] : "";
                matchDetails.serverName = !jsonResponse["response"]["server_name"].is_null() ? jsonResponse["response"]["server_name"] : "";
                if (jsonResponse["response"].contains("local_client_match_id") && !jsonResponse["response"]["local_client_match_id"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["local_client_match_id"];
                }
                if (jsonResponse["response"].contains("max_players") && !jsonResponse["response"]["max_players"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["max_players"];
                }

                currentMatchDetail = matchDetails;
                isPrivateMatch = false;
                isServer = false;

            }
        }

        result.httpResponse = httpResponse;
        result.matchJoined = matchJoined;
        result.matchDetails = matchDetails;

        curl_global_cleanup();

        return result;


    }

    JoinDenateLocalMatchResult DOS_Local_Match::JoinDenateLocalMatchByFilter(std::string filters, std::string playerName)
    {

        CURL* curl;
        CURLcode res;
        JoinDenateLocalMatchResult result;
        std::string readBuffer;
        bool matchJoined = false;
        DenateHTTPResponse httpResponse;
        DenateMatchDetails matchDetails;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            if (playerName == "")
            {
                playerName = userDetails.username;
            }

            replaceSubstring(playerName, " ", "%20");
            replaceSubstring(filters, " ", "");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("localclientmatch/joinlocalclientmatchbyfilter/") + std::string(filters) + "/" + std::string(appID) + "/" + std::string(playerName));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            /*std::string json = R"({"appID": ")" + appID + "\"" + "," + R"("userID": ")" + userID + "\"" + "," + R"("player_name": ")" + playerName + "\"" + "," + R"("server_name": ")" + serverName + "\"" + "," + R"("IP_address": ")" + ipAddresses[1] + "\"" + "," + R"("map_name": ")" + mapName + "\"" + "," + R"("max_players": )" + std::to_string(maxPlayers) + "," + R"("filter": ")" + filters + R"("})";
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
            std::cout << json << std::endl;*/

            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

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
            matchJoined = true;

            if (!jsonResponse["response"].is_null())
            {
                matchDetails.filters = !jsonResponse["response"]["filter"].is_null() ? jsonResponse["response"]["filter"] : "";
                matchDetails.ipAddress = !jsonResponse["response"]["IP_address"].is_null() ? jsonResponse["response"]["IP_address"] : "";
                matchDetails.mapName = !jsonResponse["response"]["map_name"].is_null() ? jsonResponse["response"]["map_name"] : "";
                matchDetails.playerName = !jsonResponse["response"]["player_name"].is_null() ? jsonResponse["response"]["player_name"] : "";
                matchDetails.serverName = !jsonResponse["response"]["server_name"].is_null() ? jsonResponse["response"]["server_name"] : "";
                if (jsonResponse["response"].contains("local_client_match_id") && !jsonResponse["response"]["local_client_match_id"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["local_client_match_id"];
                }
                if (jsonResponse["response"].contains("max_players") && !jsonResponse["response"]["max_players"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["max_players"];
                }

                currentMatchDetail = matchDetails;
                isPrivateMatch = false;
                isServer = false;

            }
        }

        result.httpResponse = httpResponse;
        result.matchJoined = matchJoined;
        result.matchDetails = matchDetails;

        curl_global_cleanup();

        return result;


    }

    JoinDenateLocalMatchResult DOS_Local_Match::JoinDenateLocalMatch(std::string playerName)
    {

        CURL* curl;
        CURLcode res;
        JoinDenateLocalMatchResult result;
        std::string readBuffer;
        bool matchJoined = false;
        DenateHTTPResponse httpResponse;
        DenateMatchDetails matchDetails;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            if (playerName == "")
            {
                playerName = userDetails.username;
            }

            replaceSubstring(playerName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("localclientmatch/joinlocalclientmatch/") + std::string(appID) + "/" + std::string(playerName));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            /*std::string json = R"({"appID": ")" + appID + "\"" + "," + R"("userID": ")" + userID + "\"" + "," + R"("player_name": ")" + playerName + "\"" + "," + R"("server_name": ")" + serverName + "\"" + "," + R"("IP_address": ")" + ipAddresses[1] + "\"" + "," + R"("map_name": ")" + mapName + "\"" + "," + R"("max_players": )" + std::to_string(maxPlayers) + "," + R"("filter": ")" + filters + R"("})";
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
            std::cout << json << std::endl;*/

            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

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
            matchJoined = true;

            if (!jsonResponse["response"].is_null())
            {
                matchDetails.filters = !jsonResponse["response"]["filter"].is_null() ? jsonResponse["response"]["filter"] : "";
                matchDetails.ipAddress = !jsonResponse["response"]["IP_address"].is_null() ? jsonResponse["response"]["IP_address"] : "";
                matchDetails.mapName = !jsonResponse["response"]["map_name"].is_null() ? jsonResponse["response"]["map_name"] : "";
                matchDetails.playerName = !jsonResponse["response"]["player_name"].is_null() ? jsonResponse["response"]["player_name"] : "";
                matchDetails.serverName = !jsonResponse["response"]["server_name"].is_null() ? jsonResponse["response"]["server_name"] : "";
                if (jsonResponse["response"].contains("local_client_match_id") && !jsonResponse["response"]["local_client_match_id"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["local_client_match_id"];
                }
                if (jsonResponse["response"].contains("max_players") && !jsonResponse["response"]["max_players"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["max_players"];
                }

                currentMatchDetail = matchDetails;
                isPrivateMatch = false;
                isServer = false;

            }
        }

        result.httpResponse = httpResponse;
        result.matchJoined = matchJoined;
        result.matchDetails = matchDetails;

        curl_global_cleanup();

        return result;


    }

    JoinDenatePrivateLocalMatchResult DOS_Local_Match::JoinDenatePrivateMatch(std::string gameCode, std::string playerName)
    {

        CURL* curl;
        CURLcode res;
        JoinDenatePrivateLocalMatchResult result;
        std::string readBuffer;
        bool matchJoined = false;
        DenateHTTPResponse httpResponse;
        DenatePrivateMatchDetails matchDetails;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            if (playerName == "")
            {
                playerName = userDetails.username;
            }

            replaceSubstring(playerName, " ", "%20");
            replaceSubstring(gameCode, " ", "");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("privatematch/joinprivatematch/") + std::string(gameCode) + "/" + std::string(appID) + "/" + std::string(playerName));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            /*std::string json = R"({"appID": ")" + appID + "\"" + "," + R"("userID": ")" + userID + "\"" + "," + R"("player_name": ")" + playerName + "\"" + "," + R"("server_name": ")" + serverName + "\"" + "," + R"("IP_address": ")" + ipAddresses[1] + "\"" + "," + R"("map_name": ")" + mapName + "\"" + "," + R"("max_players": )" + std::to_string(maxPlayers) + "," + R"("filter": ")" + filters + R"("})";
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
            std::cout << json << std::endl;*/

            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

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
            matchJoined = true;

            if (!jsonResponse["response"].is_null())
            {
                matchDetails.filters = !jsonResponse["response"]["filter"].is_null() ? jsonResponse["response"]["filter"] : "";
                matchDetails.ipAddress = !jsonResponse["response"]["IP_address"].is_null() ? jsonResponse["response"]["IP_address"] : "";
                matchDetails.mapName = !jsonResponse["response"]["map_name"].is_null() ? jsonResponse["response"]["map_name"] : "";
                matchDetails.playerName = !jsonResponse["response"]["player_name"].is_null() ? jsonResponse["response"]["player_name"] : "";
                matchDetails.serverName = !jsonResponse["response"]["server_name"].is_null() ? jsonResponse["response"]["server_name"] : "";
                matchDetails.serverName = !jsonResponse["response"]["game_code"].is_null() ? jsonResponse["response"]["game_code"] : "";
                if (jsonResponse["response"].contains("private_client_match_id") && !jsonResponse["response"]["private_client_match_id"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["private_client_match_id"];
                }
                if (jsonResponse["response"].contains("max_players") && !jsonResponse["response"]["max_players"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["max_players"];
                }

                currentPrivateMatchDetail = matchDetails;
                isPrivateMatch = true;
                isServer = false;

            }
        }

        result.httpResponse = httpResponse;
        result.matchJoined = matchJoined;
        result.matchDetails = matchDetails;

        curl_global_cleanup();

        return result;


    }

    FindDenateLocalMatchResult DOS_Local_Match::FindDenateLocalMatch()
    {
        
        CURL* curl;
        CURLcode res;
        FindDenateLocalMatchResult result;
        std::string readBuffer;
        bool matchFound = false;
        DenateHTTPResponse httpResponse;
        DenateMatchDetails matchDetails;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("localmatch/findlocalmatch/") + std::string(appID));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            /*std::string json = R"({"appID": ")" + appID + "\"" + "," + R"("userID": ")" + userID + "\"" + "," + R"("player_name": ")" + playerName + "\"" + "," + R"("server_name": ")" + serverName + "\"" + "," + R"("IP_address": ")" + ipAddresses[1] + "\"" + "," + R"("map_name": ")" + mapName + "\"" + "," + R"("max_players": )" + std::to_string(maxPlayers) + "," + R"("filter": ")" + filters + R"("})";
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
            std::cout << json << std::endl;*/

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
            matchFound = true;

            if (!jsonResponse["response"].is_null())
            {
                matchDetails.filters = !jsonResponse["response"]["filter"].is_null() ? jsonResponse["response"]["filter"] : "";
                matchDetails.ipAddress = !jsonResponse["response"]["IP_address"].is_null() ? jsonResponse["response"]["IP_address"] : "";
                matchDetails.mapName = !jsonResponse["response"]["map_name"].is_null() ? jsonResponse["response"]["map_name"] : "";
                matchDetails.playerName = !jsonResponse["response"]["player_name"].is_null() ? jsonResponse["response"]["player_name"] : "";
                matchDetails.serverName = !jsonResponse["response"]["server_name"].is_null() ? jsonResponse["response"]["server_name"] : "";
                if (jsonResponse["response"].contains("local_match_id") && !jsonResponse["response"]["local_match_id"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["local_match_id"];
                }
                if (jsonResponse["response"].contains("max_players") && !jsonResponse["response"]["max_players"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["max_players"];
                }

            }
        }

        result.httpResponse = httpResponse;
        result.matchFound = matchFound;
        result.matchDetails = matchDetails;

        curl_global_cleanup();

        return result;

    }

    FindDenateLocalMatchesResult DOS_Local_Match::FindDenateLocalMatches()
    {

        CURL* curl;
        CURLcode res;
        FindDenateLocalMatchesResult result;
        std::string readBuffer;
        bool matchFound = false;
        DenateHTTPResponse httpResponse;
        std::vector<DenateMatchDetails> matchDetails;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("localmatch/findlocalmatchesbyappID/") + std::string(appID));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            /*std::string json = R"({"appID": ")" + appID + "\"" + "," + R"("userID": ")" + userID + "\"" + "," + R"("player_name": ")" + playerName + "\"" + "," + R"("server_name": ")" + serverName + "\"" + "," + R"("IP_address": ")" + ipAddresses[1] + "\"" + "," + R"("map_name": ")" + mapName + "\"" + "," + R"("max_players": )" + std::to_string(maxPlayers) + "," + R"("filter": ")" + filters + R"("})";
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
            std::cout << json << std::endl;*/

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
        std::cout << readBuffer << std::endl;

        nlohmann::json jsonResponse = nlohmann::json::parse(readBuffer);

        httpResponse.status_code = jsonResponse["status"];
        httpResponse.message = jsonResponse["message"];

        if (jsonResponse.contains("response"))
        {
            matchFound = true;

            if (!jsonResponse["response"].is_null())
            {
                
                for (auto& matchdetail : jsonResponse["response"] )
                {
                    DenateMatchDetails match;

                    match.filters = !matchdetail["filter"].is_null() ? matchdetail["filter"] : "";
                    match.ipAddress = !matchdetail["IP_address"].is_null() ? matchdetail["IP_address"] : "";
                    match.mapName = !matchdetail["map_name"].is_null() ? matchdetail["map_name"] : "";
                    match.playerName = !matchdetail["player_name"].is_null() ? matchdetail["player_name"] : "";
                    match.serverName = !matchdetail["server_name"].is_null() ? matchdetail["server_name"] : "";
                    if (matchdetail.contains("local_match_id") && !matchdetail["local_match_id"].is_null())
                    {
                        match.matchId = matchdetail["local_match_id"];
                    }
                    if (matchdetail.contains("max_players") && !matchdetail["max_players"].is_null())
                    {
                        match.matchId = matchdetail["max_players"];
                    }
                    matchDetails.push_back(match);
                }

            }
        }

        result.httpResponse = httpResponse;
        result.matchFound = matchFound;
        result.matchDetails = matchDetails;

        curl_global_cleanup();

        return result;

    }

    FindDenateLocalMatchResult DOS_Local_Match::FindDenateLocalMatchByServerName(std::string serverName)
    {

        CURL* curl;
        CURLcode res;
        FindDenateLocalMatchResult result;
        std::string readBuffer;
        bool matchFound = false;
        DenateHTTPResponse httpResponse;
        DenateMatchDetails matchDetails;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            replaceSubstring(serverName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("localmatch/findlocalmatchbyservername/") + std::string(serverName) + "/" + std::string(appID));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            /*std::string json = R"({"appID": ")" + appID + "\"" + "," + R"("userID": ")" + userID + "\"" + "," + R"("player_name": ")" + playerName + "\"" + "," + R"("server_name": ")" + serverName + "\"" + "," + R"("IP_address": ")" + ipAddresses[1] + "\"" + "," + R"("map_name": ")" + mapName + "\"" + "," + R"("max_players": )" + std::to_string(maxPlayers) + "," + R"("filter": ")" + filters + R"("})";
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
            std::cout << json << std::endl;*/

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
            matchFound = true;

            if (!jsonResponse["response"].is_null())
            {
                matchDetails.filters = !jsonResponse["response"]["filter"].is_null() ? jsonResponse["response"]["filter"] : "";
                matchDetails.ipAddress = !jsonResponse["response"]["IP_address"].is_null() ? jsonResponse["response"]["IP_address"] : "";
                matchDetails.mapName = !jsonResponse["response"]["map_name"].is_null() ? jsonResponse["response"]["map_name"] : "";
                matchDetails.playerName = !jsonResponse["response"]["player_name"].is_null() ? jsonResponse["response"]["player_name"] : "";
                matchDetails.serverName = !jsonResponse["response"]["server_name"].is_null() ? jsonResponse["response"]["server_name"] : "";
                if (jsonResponse["response"].contains("local_match_id") && !jsonResponse["response"]["local_match_id"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["local_match_id"];
                }
                if (jsonResponse["response"].contains("max_players") && !jsonResponse["response"]["max_players"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["max_players"];
                }

            }
        }

        result.httpResponse = httpResponse;
        result.matchFound = matchFound;
        result.matchDetails = matchDetails;

        curl_global_cleanup();

        return result;

    }

    FindDenateLocalMatchResult DOS_Local_Match::FindDenateLocalMatchByPlayerName(std::string playerName)
    {

        CURL* curl;
        CURLcode res;
        FindDenateLocalMatchResult result;
        std::string readBuffer;
        bool matchFound = false;
        DenateHTTPResponse httpResponse;
        DenateMatchDetails matchDetails;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            replaceSubstring(playerName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("localmatch/findlocalmatchbyplayername/") + std::string(playerName) + "/" + std::string(appID));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            /*std::string json = R"({"appID": ")" + appID + "\"" + "," + R"("userID": ")" + userID + "\"" + "," + R"("player_name": ")" + playerName + "\"" + "," + R"("server_name": ")" + serverName + "\"" + "," + R"("IP_address": ")" + ipAddresses[1] + "\"" + "," + R"("map_name": ")" + mapName + "\"" + "," + R"("max_players": )" + std::to_string(maxPlayers) + "," + R"("filter": ")" + filters + R"("})";
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
            std::cout << json << std::endl;*/

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
            matchFound = true;

            if (!jsonResponse["response"].is_null())
            {
                matchDetails.filters = !jsonResponse["response"]["filter"].is_null() ? jsonResponse["response"]["filter"] : "";
                matchDetails.ipAddress = !jsonResponse["response"]["IP_address"].is_null() ? jsonResponse["response"]["IP_address"] : "";
                matchDetails.mapName = !jsonResponse["response"]["map_name"].is_null() ? jsonResponse["response"]["map_name"] : "";
                matchDetails.playerName = !jsonResponse["response"]["player_name"].is_null() ? jsonResponse["response"]["player_name"] : "";
                matchDetails.serverName = !jsonResponse["response"]["server_name"].is_null() ? jsonResponse["response"]["server_name"] : "";
                if (jsonResponse["response"].contains("local_match_id") && !jsonResponse["response"]["local_match_id"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["local_match_id"];
                }
                if (jsonResponse["response"].contains("max_players") && !jsonResponse["response"]["max_players"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["max_players"];
                }

            }
        }

        result.httpResponse = httpResponse;
        result.matchFound = matchFound;
        result.matchDetails = matchDetails;

        curl_global_cleanup();

        return result;

    }

    FindDenateLocalMatchResult DOS_Local_Match::FindDenateLocalMatchByMapName(std::string mapName)
    {

        CURL* curl;
        CURLcode res;
        FindDenateLocalMatchResult result;
        std::string readBuffer;
        bool matchFound = false;
        DenateHTTPResponse httpResponse;
        DenateMatchDetails matchDetails;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            replaceSubstring(mapName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("localmatch/findlocalmatchbymapname/") + std::string(mapName) + "/" + std::string(appID));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            /*std::string json = R"({"appID": ")" + appID + "\"" + "," + R"("userID": ")" + userID + "\"" + "," + R"("player_name": ")" + playerName + "\"" + "," + R"("server_name": ")" + serverName + "\"" + "," + R"("IP_address": ")" + ipAddresses[1] + "\"" + "," + R"("map_name": ")" + mapName + "\"" + "," + R"("max_players": )" + std::to_string(maxPlayers) + "," + R"("filter": ")" + filters + R"("})";
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
            std::cout << json << std::endl;*/

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
            matchFound = true;

            if (!jsonResponse["response"].is_null())
            {
                matchDetails.filters = !jsonResponse["response"]["filter"].is_null() ? jsonResponse["response"]["filter"] : "";
                matchDetails.ipAddress = !jsonResponse["response"]["IP_address"].is_null() ? jsonResponse["response"]["IP_address"] : "";
                matchDetails.mapName = !jsonResponse["response"]["map_name"].is_null() ? jsonResponse["response"]["map_name"] : "";
                matchDetails.playerName = !jsonResponse["response"]["player_name"].is_null() ? jsonResponse["response"]["player_name"] : "";
                matchDetails.serverName = !jsonResponse["response"]["server_name"].is_null() ? jsonResponse["response"]["server_name"] : "";
                if (jsonResponse["response"].contains("local_match_id") && !jsonResponse["response"]["local_match_id"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["local_match_id"];
                }
                if (jsonResponse["response"].contains("max_players") && !jsonResponse["response"]["max_players"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["max_players"];
                }

            }
        }

        result.httpResponse = httpResponse;
        result.matchFound = matchFound;
        result.matchDetails = matchDetails;

        curl_global_cleanup();

        return result;

    }

    FindDenateLocalMatchResult DOS_Local_Match::FindDenateLocalMatchByFilter(std::string filters)
    {

        CURL* curl;
        CURLcode res;
        FindDenateLocalMatchResult result;
        std::string readBuffer;
        bool matchFound = false;
        DenateHTTPResponse httpResponse;
        DenateMatchDetails matchDetails;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            replaceSubstring(filters, " ", "");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("localmatch/findlocalmatchbyfilter/") + std::string(filters) + "/" + std::string(appID));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            /*std::string json = R"({"appID": ")" + appID + "\"" + "," + R"("userID": ")" + userID + "\"" + "," + R"("player_name": ")" + playerName + "\"" + "," + R"("server_name": ")" + serverName + "\"" + "," + R"("IP_address": ")" + ipAddresses[1] + "\"" + "," + R"("map_name": ")" + mapName + "\"" + "," + R"("max_players": )" + std::to_string(maxPlayers) + "," + R"("filter": ")" + filters + R"("})";
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
            std::cout << json << std::endl;*/

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
            matchFound = true;

            if (!jsonResponse["response"].is_null())
            {
                matchDetails.filters = !jsonResponse["response"]["filter"].is_null() ? jsonResponse["response"]["filter"] : "";
                matchDetails.ipAddress = !jsonResponse["response"]["IP_address"].is_null() ? jsonResponse["response"]["IP_address"] : "";
                matchDetails.mapName = !jsonResponse["response"]["map_name"].is_null() ? jsonResponse["response"]["map_name"] : "";
                matchDetails.playerName = !jsonResponse["response"]["player_name"].is_null() ? jsonResponse["response"]["player_name"] : "";
                matchDetails.serverName = !jsonResponse["response"]["server_name"].is_null() ? jsonResponse["response"]["server_name"] : "";
                if (jsonResponse["response"].contains("local_match_id") && !jsonResponse["response"]["local_match_id"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["local_match_id"];
                }
                if (jsonResponse["response"].contains("max_players") && !jsonResponse["response"]["max_players"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["max_players"];
                }

            }
        }

        result.httpResponse = httpResponse;
        result.matchFound = matchFound;
        result.matchDetails = matchDetails;

        curl_global_cleanup();

        return result;

    }

    FindDenateLocalMatchesResult DOS_Local_Match::FindDenateLocalMatchesByFilter(std::string filters)
    {

        CURL* curl;
        CURLcode res;
        FindDenateLocalMatchesResult result;
        std::string readBuffer;
        bool matchFound = false;
        DenateHTTPResponse httpResponse;
        std::vector<DenateMatchDetails> matchDetails;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            replaceSubstring(filters, " ", "");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("localmatch/findlocalmatchesbyfilter/") + std::string(filters) + "/" + std::string(appID));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            /*std::string json = R"({"appID": ")" + appID + "\"" + "," + R"("userID": ")" + userID + "\"" + "," + R"("player_name": ")" + playerName + "\"" + "," + R"("server_name": ")" + serverName + "\"" + "," + R"("IP_address": ")" + ipAddresses[1] + "\"" + "," + R"("map_name": ")" + mapName + "\"" + "," + R"("max_players": )" + std::to_string(maxPlayers) + "," + R"("filter": ")" + filters + R"("})";
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
            std::cout << json << std::endl;*/

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
        std::cout << readBuffer << std::endl;

        nlohmann::json jsonResponse = nlohmann::json::parse(readBuffer);

        httpResponse.status_code = jsonResponse["status"];
        httpResponse.message = jsonResponse["message"];

        if (jsonResponse.contains("response"))
        {
            matchFound = true;

            if (!jsonResponse["response"].is_null())
            {

                for (auto& matchdetail : jsonResponse["response"])
                {
                    DenateMatchDetails match;

                    match.filters = !matchdetail["filter"].is_null() ? matchdetail["filter"] : "";
                    match.ipAddress = !matchdetail["IP_address"].is_null() ? matchdetail["IP_address"] : "";
                    match.mapName = !matchdetail["map_name"].is_null() ? matchdetail["map_name"] : "";
                    match.playerName = !matchdetail["player_name"].is_null() ? matchdetail["player_name"] : "";
                    match.serverName = !matchdetail["server_name"].is_null() ? matchdetail["server_name"] : "";
                    if (matchdetail.contains("local_match_id") && !matchdetail["local_match_id"].is_null())
                    {
                        match.matchId = matchdetail["local_match_id"];
                    }
                    if (matchdetail.contains("max_players") && !matchdetail["max_players"].is_null())
                    {
                        match.matchId = matchdetail["max_players"];
                    }
                    matchDetails.push_back(match);
                }

            }
        }

        result.httpResponse = httpResponse;
        result.matchFound = matchFound;
        result.matchDetails = matchDetails;

        curl_global_cleanup();

        return result;

    }

    UpdateDenateLocalMatchResult DOS_Local_Match::UpdateDenateLocalMatch(std::string mapName, std::string filters, int maxPlayers, std::string serverName, std::string playerName)
    {

        CURL* curl;
        CURLcode res;
        UpdateDenateLocalMatchResult result;
        std::string readBuffer;
        bool matchUpdated = false;
        DenateHTTPResponse httpResponse;
        DenateMatchDetails matchDetails;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            std::string tempServerName = currentMatchDetail.serverName;
            std::string tempIpaddr = currentMatchDetail.ipAddress;

            replaceSubstring(tempServerName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("localmatch/updatelocalmatch/") + std::string(tempIpaddr) + "/" + std::string(tempServerName) + "/" + std::string(appID));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            std::cout << url << std::endl;

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            std::vector<std::string> ipAddresses = getIPAddresses();

            std::string json = R"({"appID": ")" + appID + "\"" + "," + R"("userID": ")" + userID + "\"" + "," + R"("player_name": ")" + playerName + "\"" + "," + R"("server_name": ")" + serverName + "\"" + "," + R"("IP_address": ")" + ipAddresses[1] + "\"" + "," + R"("map_name": ")" + mapName + "\"" + "," + R"("max_players": )" + std::to_string(maxPlayers) + "," + R"("filter": ")" + filters + R"("})";
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
            
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");

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
        std::cout << readBuffer << std::endl;

        nlohmann::json jsonResponse = nlohmann::json::parse(readBuffer);

        httpResponse.status_code = jsonResponse["status"];
        httpResponse.message = jsonResponse["message"];

        if (jsonResponse.contains("response"))
        {
            matchUpdated = true;

            if (!jsonResponse["response"].is_null())
            {
                matchDetails.filters = !jsonResponse["response"]["filter"].is_null() ? jsonResponse["response"]["filter"] : "";
                matchDetails.ipAddress = !jsonResponse["response"]["IP_address"].is_null() ? jsonResponse["response"]["IP_address"] : "";
                matchDetails.mapName = !jsonResponse["response"]["map_name"].is_null() ? jsonResponse["response"]["map_name"] : "";
                matchDetails.playerName = !jsonResponse["response"]["player_name"].is_null() ? jsonResponse["response"]["player_name"] : "";
                matchDetails.serverName = !jsonResponse["response"]["server_name"].is_null() ? jsonResponse["response"]["server_name"] : "";
                if (jsonResponse["response"].contains("local_match_id") && !jsonResponse["response"]["local_match_id"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["local_match_id"];
                }
                if (jsonResponse["response"].contains("max_players") && !jsonResponse["response"]["max_players"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["max_players"];
                }

            }
        }

        result.httpResponse = httpResponse;
        result.matchUpdated = matchUpdated;
        result.matchDetails = matchDetails;

        currentMatchDetail = matchDetails;

        curl_global_cleanup();

        return result;

    }

    UpdateDenatePrivateLocalMatchResult DOS_Local_Match::UpdateDenateLocalPrivateMatch(std::string mapName, std::string filters, int maxPlayers, std::string serverName, std::string playerName)
    {

        CURL* curl;
        CURLcode res;
        UpdateDenatePrivateLocalMatchResult result;
        std::string readBuffer;
        bool matchUpdated = false;
        DenateHTTPResponse httpResponse;
        DenatePrivateMatchDetails matchDetails;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            std::string tempServerName = currentPrivateMatchDetail.serverName;

            replaceSubstring(tempServerName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("privatematch/updateprivatelocalmatch/") + std::string(currentPrivateMatchDetail.ipAddress) + "/" + std::string(tempServerName) + "/" + std::string(appID));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            std::vector<std::string> ipAddresses = getIPAddresses();

            std::string json = R"({"appID": ")" + appID + "\"" + "," + R"("userID": ")" + userID + "\"" + "," + R"("player_name": ")" + playerName + "\"" + "," + R"("server_name": ")" + serverName + "\"" + "," + R"("IP_address": ")" + ipAddresses[1] + "\"" + "," + R"("map_name": ")" + mapName + "\"" + "," + R"("max_players": )" + std::to_string(maxPlayers) + "," + R"("filter": ")" + filters + R"("})";
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
            std::cout << json << std::endl;

            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");

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
            matchUpdated = true;

            if (!jsonResponse["response"].is_null())
            {
                matchDetails.filters = !jsonResponse["response"]["filter"].is_null() ? jsonResponse["response"]["filter"] : "";
                matchDetails.ipAddress = !jsonResponse["response"]["IP_address"].is_null() ? jsonResponse["response"]["IP_address"] : "";
                matchDetails.mapName = !jsonResponse["response"]["map_name"].is_null() ? jsonResponse["response"]["map_name"] : "";
                matchDetails.playerName = !jsonResponse["response"]["player_name"].is_null() ? jsonResponse["response"]["player_name"] : "";
                matchDetails.serverName = !jsonResponse["response"]["server_name"].is_null() ? jsonResponse["response"]["server_name"] : "";
                matchDetails.gameCode = !jsonResponse["response"]["game_code"].is_null() ? jsonResponse["response"]["game_code"] : "";
                if (jsonResponse["response"].contains("private_match_id") && !jsonResponse["response"]["private_match_id"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["private_match_id"];
                }
                if (jsonResponse["response"].contains("max_players") && !jsonResponse["response"]["max_players"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["max_players"];
                }

            }
        }

        result.httpResponse = httpResponse;
        result.matchUpdated = matchUpdated;
        result.privateMatchDetails = matchDetails;

        currentPrivateMatchDetail = matchDetails;

        curl_global_cleanup();

        return result;

    }

}