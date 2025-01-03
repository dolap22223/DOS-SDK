// Copyright 2024, David Olashege, All rights reserved

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

    void DOS_Local_Match::replaceSubstring(std::string& original, const std::string& toReplace, const std::string& replacement)
    {
        size_t pos = 0;
        while ((pos = original.find(toReplace, pos)) != std::string::npos)
        {
            original.replace(pos, toReplace.length(), replacement);
            pos += replacement.length();
        }
    }

    DOS_Local_Match::DOS_Local_Match(std::string userID, std::string appID, bool dedicatedServer, std::string token, DenateUserDetails userDetails, DenateConnection::DOS_Connection& denateConnection) : internalDenateConnection(denateConnection)
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


    void DOS_Local_Match::OnPlayerJoinedPrivateMatch(playerJoinedPrivateMatchListener const& playerJoinedPrivateMatch)
    {
        if (playerJoinedPrivateMatch)
        {
            internalPlayerJoinedPrivateMatch = playerJoinedPrivateMatch;
        }
    }

    void DOS_Local_Match::OnPlayerJoinedMatch(playerJoinedMatchListener const& playerJoinedMatch)
    {
        if (playerJoinedMatch)
        {
            internalPlayerJoinedMatch = playerJoinedMatch;
        }
    }

    void DOS_Local_Match::OnPlayerLeftMatch(playerLeftMatchListener const& playerLeftMatch)
    {
        if (playerLeftMatch)
        {
            internalPlayerLeftMatch = playerLeftMatch;
        }
    }

    void DOS_Local_Match::OnMessageBroadcastedToMatch(messageBroadcastedToMatchListener const& messageBroadcastedToMatch)
    {
        if (messageBroadcastedToMatch)
        {
            internalMessageBroadcastedToMatch = messageBroadcastedToMatch;
        }
    }

    void DOS_Local_Match::OnPlayerJoinTeam(playerJoinTeamListener const& playerJoinTeam)
    {
        if (playerJoinTeam)
        {
            internalPlayerJoinTeam = playerJoinTeam;
        }
    }

    void DOS_Local_Match::OnPlayerLeaveTeam(playerLeaveTeamListener const& playerLeaveTeam)
    {
        if (playerLeaveTeam)
        {
            internalPlayerLeaveTeam = playerLeaveTeam;
        }
    }

    void DOS_Local_Match::OnPlayerDestroyTeam(playerDestroyTeamListener const& playerDestroyTeam)
    {
        if (playerDestroyTeam)
        {
            internalPlayerDestroyTeam = playerDestroyTeam;
        }
    }

    void DOS_Local_Match::OnMessageBroadcastedToTeam(messageBroadcastedToTeamListener const& messageBroadcastedToTeam)
    {
        if (messageBroadcastedToTeam)
        {
            internalMessageBroadcastedToTeam = messageBroadcastedToTeam;
        }
    }

    void DOS_Local_Match::OnPlayerLeftPrivateMatch(playerLeftPrivateMatchListener const& playerLeftPrivateMatch)
    {
        if (playerLeftPrivateMatch)
        {
            internalPlayerLeftPrivateMatch = playerLeftPrivateMatch;
        }
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

            replaceSubstring(filters, " ", "");

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
                    matchDetails.maxPlayers = jsonResponse["response"]["max_players"];
                }

                currentMatchDetail = matchDetails;
                isPrivateMatch = false;
                isServer = true;

                ActivateDenateMatchConnection(true, false);

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

            replaceSubstring(filters, " ", "");

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
                    matchDetails.maxPlayers = jsonResponse["response"]["max_players"];
                }


                currentPrivateMatchDetail = matchDetails;
                isPrivateMatch = true;
                isServer = true;

                ActivateDenateMatchConnection(true, true);

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
                    matchDetails.maxPlayers = jsonResponse["response"]["max_players"];
                }

                currentMatchDetail = matchDetails;
                isPrivateMatch = false;
                isServer = false;

                ActivateDenateMatchConnection(false, false);

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
                    matchDetails.maxPlayers = jsonResponse["response"]["max_players"];
                }

                currentMatchDetail = matchDetails;
                isPrivateMatch = false;
                isServer = false;

                ActivateDenateMatchConnection(false, false);

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
                    matchDetails.maxPlayers = jsonResponse["response"]["max_players"];
                }

                currentMatchDetail = matchDetails;
                isPrivateMatch = false;
                isServer = false;

                ActivateDenateMatchConnection(false, false);

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
                    matchDetails.maxPlayers = jsonResponse["response"]["max_players"];
                }

                currentMatchDetail = matchDetails;
                isPrivateMatch = false;
                isServer = false;

                ActivateDenateMatchConnection(false, false);

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
                    matchDetails.maxPlayers = jsonResponse["response"]["max_players"];
                }

                currentPrivateMatchDetail = matchDetails;
                isPrivateMatch = true;
                isServer = false;

                ActivateDenateMatchConnection(false, true);

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
                    matchDetails.maxPlayers = jsonResponse["response"]["max_players"];
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
        //std::cout << readBuffer << std::endl;

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
                        match.maxPlayers = matchdetail["max_players"];
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
                    matchDetails.maxPlayers = jsonResponse["response"]["max_players"];
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
                    matchDetails.maxPlayers = jsonResponse["response"]["max_players"];
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
                    matchDetails.maxPlayers = jsonResponse["response"]["max_players"];
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
                    matchDetails.maxPlayers = jsonResponse["response"]["max_players"];
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
        //std::cout << readBuffer << std::endl;

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
                        match.maxPlayers = matchdetail["max_players"];
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

        if (currentMatchDetail.ipAddress == "" && currentMatchDetail.serverName == "")
        {
            std::cout << "You have to be part of a match to update it" << std::endl;
            return result;
        }

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            std::string tempServerName = currentMatchDetail.serverName;
            std::string tempIpaddr = currentMatchDetail.ipAddress;

            replaceSubstring(tempServerName, " ", "%20");
            replaceSubstring(filters, " ", "");

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
                if (jsonResponse["response"].contains("local_match_id") && !jsonResponse["response"]["local_match_id"].is_null())
                {
                    matchDetails.matchId = jsonResponse["response"]["local_match_id"];
                }
                if (jsonResponse["response"].contains("max_players") && !jsonResponse["response"]["max_players"].is_null())
                {
                    matchDetails.maxPlayers = jsonResponse["response"]["max_players"];
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

        if (currentPrivateMatchDetail.ipAddress == "" && currentPrivateMatchDetail.serverName == "")
        {
            std::cout << "You have to be part of a match to update it" << std::endl;
            return result;
        }

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            replaceSubstring(filters, " ", "");

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
                    matchDetails.maxPlayers = jsonResponse["response"]["max_players"];
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

    CreateTeamResult DOS_Local_Match::CreateTeam(std::string filters)
    {
        CURL* curl;
        CURLcode res;
        CreateTeamResult result;
        std::string readBuffer;
        bool teamCreated = false;
        DenateHTTPResponse httpResponse;
        std::string teamID;

        if (isPrivateMatch)
        {
            if (currentPrivateMatchDetail.ipAddress == "" && currentPrivateMatchDetail.serverName == "")
            {
                std::cout << "You have to be part of a match to create a team" << std::endl;
                return result;
            }
        }
        else {
            if (currentMatchDetail.ipAddress == "" && currentMatchDetail.serverName == "")
            {
                std::cout << "You have to be part of a match to create a team" << std::endl;
                return result;
            }
        }

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            replaceSubstring(filters, " ", "");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denateteams/createteam"));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            std::cout << url << std::endl;

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            std::string json = "";
            if (isPrivateMatch)
            {
                json = R"({"appID": ")" + appID + "\"" + "," + R"("userID": ")" + userID + "\"" + "," + R"("server_name": ")" + currentPrivateMatchDetail.serverName + "\"" + "," + R"("is_private_match": )" + "true" + "," + R"("max_players": )" + std::to_string(currentPrivateMatchDetail.maxPlayers) + "," + R"("match_id": )" + std::to_string(currentPrivateMatchDetail.matchId) + "," + R"("filter": ")" + filters + R"("})";
            }
            else {
                json = R"({"appID": ")" + appID + "\"" + "," + R"("userID": ")" + userID + "\"" + "," + R"("server_name": ")" + currentMatchDetail.serverName + "\"" + "," + R"("is_private_match": )" + "false" + "," + R"("max_players": )" + std::to_string(currentMatchDetail.maxPlayers) + "," + R"("match_id": )" + std::to_string(currentMatchDetail.matchId) + "," + R"("filter": ")" + filters + R"("})";
            }
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());

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
            teamCreated = true;

            if (!jsonResponse["response"].is_null())
            {
                if (jsonResponse["response"].contains("teamID") && !jsonResponse["response"]["teamID"].is_null())
                {
                    teamID = jsonResponse["response"]["teamID"];
                }

            }
        }

        result.httpResponse = httpResponse;
        result.teamCreated = teamCreated;
        result.teamID = teamID;

        curl_global_cleanup();

        return result;
    }

    JoinTeamResult DOS_Local_Match::JoinTeam(std::string teamID)
    {
        CURL* curl;
        CURLcode res;
        JoinTeamResult result;
        std::string readBuffer;
        bool teamJoined = false;
        DenateHTTPResponse httpResponse;
        DenateTeamDetails teamDetails;

        if (isPrivateMatch)
        {
            if (currentPrivateMatchDetail.ipAddress == "" && currentPrivateMatchDetail.serverName == "")
            {
                std::cout << "You have to be part of a match to join a team" << std::endl;
                return result;
            }
        }
        else {
            if (currentMatchDetail.ipAddress == "" && currentMatchDetail.serverName == "")
            {
                std::cout << "You have to be part of a match to join a team" << std::endl;
                return result;
            }
        }

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denateteams/jointeam"));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            std::cout << url << std::endl;

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            std::string json = "";
            if (isPrivateMatch)
            {
                json = R"({"appID": ")" + appID + "\"" + "," + R"("userID": ")" + userID + "\"" + "," + R"("player_name": ")" + userDetails.username + "\"" + "," + R"("match_id": )" + std::to_string(currentPrivateMatchDetail.matchId) + "," + R"("teamID": ")" + teamID + R"("})";
            }
            else {
                json = R"({"appID": ")" + appID + "\"" + "," + R"("userID": ")" + userID + "\"" + "," + R"("player_name": ")" + userDetails.username + "\"" + "," + R"("match_id": )" + std::to_string(currentMatchDetail.matchId) + "," + R"("teamID": ")" + teamID + R"("})";
            }
            //std::cout << json << std::endl;
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());

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
            teamJoined = true;

            if (!jsonResponse["response"].is_null())
            {
                teamDetails.filters = !jsonResponse["response"]["filter"].is_null() ? jsonResponse["response"]["filter"] : "";
                teamDetails.serverName = !jsonResponse["response"]["server_name"].is_null() ? jsonResponse["response"]["server_name"] : "";

                if (jsonResponse["response"].contains("teamID") && !jsonResponse["response"]["teamID"].is_null())
                {
                    teamDetails.teamId = jsonResponse["response"]["teamID"];
                }
                if (jsonResponse["response"].contains("match_id") && !jsonResponse["response"]["match_id"].is_null())
                {
                    teamDetails.matchId = jsonResponse["response"]["match_id"];
                }
                if (jsonResponse["response"].contains("is_private_match") && !jsonResponse["response"]["is_private_match"].is_null())
                {
                    teamDetails.isPrivateMatch = jsonResponse["response"]["is_private_match"];
                }
                if (jsonResponse["response"].contains("max_players") && !jsonResponse["response"]["max_players"].is_null())
                {
                    teamDetails.maxPlayers = jsonResponse["response"]["max_players"];
                }

                currentTeamDetail = teamDetails;

                ActivateDenateTeamConnection();

            }
        }

        result.httpResponse = httpResponse;
        result.teamJoined = teamJoined;
        result.teamDetails = teamDetails;

        curl_global_cleanup();

        return result;
    }

    LeaveTeamResult DOS_Local_Match::LeaveTeam(std::string teamID)
    {
        CURL* curl;
        CURLcode res;
        LeaveTeamResult result;
        std::string readBuffer;
        bool leftTeam = false;
        DenateHTTPResponse httpResponse;

        if (isPrivateMatch)
        {
            if (currentPrivateMatchDetail.ipAddress == "" && currentPrivateMatchDetail.serverName == "")
            {
                std::cout << "You have to be part of a match to leave a team" << std::endl;
                return result;
            }
        }
        else {
            if (currentMatchDetail.ipAddress == "" && currentMatchDetail.serverName == "")
            {
                std::cout << "You have to be part of a match to leave a team" << std::endl;
                return result;
            }
        }

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            if (teamID == "")
            {
                teamID = currentTeamDetail.teamId;
            }

            std::string tempPlayerName = userDetails.username;
            replaceSubstring(tempPlayerName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denateteams/leaveteam/") + std::string(tempPlayerName));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            //std::cout << url << std::endl;

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

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
            leftTeam = true;

            if (&internalDenateConnection != nullptr)
            {
                if (internalDenateConnection.isDenateOnlineServiceConnected)
                {
                    if (namespaceTeamSocket)
                    {
                        sio::message::ptr jsonMessage = sio::object_message::create();

                        jsonMessage->get_map()["player_name"] = sio::string_message::create(userDetails.username);
                        jsonMessage->get_map()["appID"] = sio::string_message::create(appID);
                        jsonMessage->get_map()["userID"] = sio::string_message::create(userID);
                        jsonMessage->get_map()["teamID"] = sio::string_message::create(currentTeamDetail.teamId);

                        std::string currentPlayerClientId;

                        for (int i = 0; i < currentTeamPlayers.size(); i++)
                        {
                            if (currentTeamPlayers[i].playerName == userDetails.username)
                            {
                                currentPlayerClientId = currentTeamPlayers[i].clientId;
                            }
                        }

                        jsonMessage->get_map()["client_id"] = sio::string_message::create(currentPlayerClientId);

                        namespaceTeamSocket->emit("leaveteam", jsonMessage, [&](sio::message::list const& ack_msg) {

                            namespaceTeamSocket->close();

                            });

                    }

                }
            }

        }

        result.httpResponse = httpResponse;
        result.leftTeam = leftTeam;

        curl_global_cleanup();

        return result;
    }

    DestroyTeamResult DOS_Local_Match::DestroyTeam(std::string teamID)
    {
        CURL* curl;
        CURLcode res;
        DestroyTeamResult result;
        std::string readBuffer;
        bool teamDestroyed = false;
        DenateHTTPResponse httpResponse;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            if (teamID == "")
            {
                teamID = currentTeamDetail.teamId;
            }

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denateteams/destroyteam/") + std::string(teamID));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            //std::cout << url << std::endl;

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

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
            teamDestroyed = true;

            if (&internalDenateConnection != nullptr)
            {
                if (internalDenateConnection.isDenateOnlineServiceConnected)
                {
                    if (namespaceTeamSocket)
                    {
                        sio::message::ptr jsonMessage = sio::object_message::create();

                        jsonMessage->get_map()["player_name"] = sio::string_message::create(userDetails.username);
                        jsonMessage->get_map()["appID"] = sio::string_message::create(appID);
                        jsonMessage->get_map()["userID"] = sio::string_message::create(userID);
                        jsonMessage->get_map()["teamID"] = sio::string_message::create(currentTeamDetail.teamId);
                        jsonMessage->get_map()["client_id"] = sio::string_message::create("");

                        namespaceTeamSocket->emit("destroyteam", jsonMessage, [&](sio::message::list const& ack_msg) {

                            if (teamID == currentTeamDetail.teamId)
                            {
                                namespaceTeamSocket->close();
                            }

                            });

                    }

                }
            }

        }

        result.httpResponse = httpResponse;
        result.teamDestroyed = teamDestroyed;

        curl_global_cleanup();

        return result;
    }

    GetPlayersInTeamResult DOS_Local_Match::GetPlayersInTeam(std::string teamID)
    {
        CURL* curl;
        CURLcode res;
        GetPlayersInTeamResult result;
        std::string readBuffer;
        bool gottenPlayers = false;
        DenateHTTPResponse httpResponse;
        std::vector<DenateTeamPlayersDetails> teamPlayers;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denateteams/getplayersinteam/") + std::string(teamID));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            std::cout << url << std::endl;

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
            gottenPlayers = true;

            if (!jsonResponse["response"].is_null())
            {
                if (jsonResponse["response"].is_array())
                {
                    for (auto& player : jsonResponse["response"])
                    {
                        DenateTeamPlayersDetails playerdetail;

                        playerdetail.clientId = !player["client_id"].is_null() ? player["client_id"] : "";
                        playerdetail.playerName = !player["player_name"].is_null() ? player["player_name"] : "";

                        teamPlayers.push_back(playerdetail);
                    }
                }
            }
        }

        result.httpResponse = httpResponse;
        result.gottenPlayers = gottenPlayers;
        result.teamPlayers = teamPlayers;

        curl_global_cleanup();

        return result;
    }

    GetTeamsInMatchResult DOS_Local_Match::GetTeamsInMatch()
    {
        CURL* curl;
        CURLcode res;
        GetTeamsInMatchResult result;
        std::string readBuffer;
        bool gottenTeams = false;
        DenateHTTPResponse httpResponse;
        std::vector<DenateTeamMatchDetail> teamDetails;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            std::string url = "";

            if (isPrivateMatch)
            {
                url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denateteams/getteamsinmatch/") + std::string(std::to_string(currentPrivateMatchDetail.matchId)) + "/" + std::string(appID));
            }
            else {
                url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denateteams/getteamsinmatch/") + std::string(std::to_string(currentMatchDetail.matchId)) + "/" + std::string(appID));
            }

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            //std::cout << url << std::endl;

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
            gottenTeams = true;

            if (!jsonResponse["response"].is_null())
            {
                if (jsonResponse["response"].is_array())
                {
                    for (auto& player : jsonResponse["response"])
                    {
                        DenateTeamMatchDetail teamdetail;

                        teamdetail.filters = !player["filter"].is_null() ? player["filter"] : "";
                        teamdetail.TeamID = !player["teamID"].is_null() ? player["teamID"] : "";

                        teamDetails.push_back(teamdetail);
                    }
                }
            }
        }

        result.httpResponse = httpResponse;
        result.gottenTeams = gottenTeams;
        result.teamsDetails = teamDetails;

        curl_global_cleanup();

        return result;
    }

    GetPlayersTeamResult DOS_Local_Match::GetPlayersTeam(std::string playerName)
    {
        CURL* curl;
        CURLcode res;
        GetPlayersTeamResult result;
        std::string readBuffer;
        bool gottenTeam = false;
        DenateHTTPResponse httpResponse;
        DenateTeamMatchDetail teamDetails;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            if (playerName == "")
            {
                playerName = userDetails.username;
            }

            replaceSubstring(playerName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denateteams/getplayersteam/") + std::string(playerName) + "/" + std::string(appID));

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            //std::cout << url << std::endl;

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
            gottenTeam = true;

            if (!jsonResponse["response"].is_null())
            {

                teamDetails.filters = !jsonResponse["response"]["filter"].is_null() ? jsonResponse["response"]["filter"] : "";
                teamDetails.TeamID = !jsonResponse["response"]["teamID"].is_null() ? jsonResponse["response"]["teamID"] : "";

            }
        }

        result.httpResponse = httpResponse;
        result.gottenTeam = gottenTeam;
        result.teamDetail = teamDetails;

        curl_global_cleanup();

        return result;
    }

    ReportPlayerResult DOS_Local_Match::ReportPlayer(std::string nameOfPlayerToReport, std::string reason)
    {
        CURL* curl;
        CURLcode res;
        ReportPlayerResult result;
        std::string readBuffer;
        bool playerReported = false;
        DenateHTTPResponse httpResponse;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("playerreport/createplayerreport"));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            std::cout << url << std::endl;

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            std::string json = R"({"appID": ")" + appID + "\"" + "," + R"("userID": ")" + userID + "\"" + "," + R"("from_player_name": ")" + userDetails.username + "\"" + "," + R"("to_player_name": ")" + nameOfPlayerToReport + "\"" + "," + R"("reason": ")" + reason + R"("})";;
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());

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
            playerReported = true;
        }

        result.httpResponse = httpResponse;
        result.playerReported = playerReported;

        curl_global_cleanup();

        return result;
    }

    InviteFriendResult DOS_Local_Match::InviteFriend(std::string friendName)
    {
        CURL* curl;
        CURLcode res;
        InviteFriendResult result;
        std::string readBuffer;
        bool inviteSent = false;
        DenateHTTPResponse httpResponse;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("localmatch/invitefriend/"));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            std::cout << url << std::endl;

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            std::string json = R"({"appID": ")" + appID + "\"" + "," + R"("userID": ")" + userID + "\"" + "," + R"("player_name": ")" + userDetails.username + "\"" + "," + R"("friend_name": ")" + friendName + "\"" + "," + R"("friend_appuserID": ")" + "" + "\"" + "," + R"("player_appuserID": ")" + "" + R"("})";;
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());

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
            inviteSent = true;

            if (&internalDenateConnection != nullptr)
            {
                if (internalDenateConnection.isDenateOnlineServiceConnected)
                {
                    sio::message::ptr jsonMessage = sio::object_message::create();

                    jsonMessage->get_map()["friend_name"] = sio::string_message::create(jsonResponse["response"]["friend_name"]);
                    jsonMessage->get_map()["player_name"] = sio::string_message::create(std::string(userDetails.username));
                    jsonMessage->get_map()["appID"] = sio::string_message::create(std::string(appID));

                    internalDenateConnection.namespaceSocket->emit("inviteplayer", jsonMessage, [&](sio::message::list const& ack_msg) {

                        std::cout << "Invite Sent" << std::endl;

                        });

                }
            }

        }

        result.httpResponse = httpResponse;
        result.inviteSent = inviteSent;

        curl_global_cleanup();

        return result;
    }

    bool DOS_Local_Match::EndDenateMatch()
    {
        return DeactivateDenateMatchConnection(isServer, isPrivateMatch);
    }

    void DOS_Local_Match::ActivateDenateMatchConnection(bool isServer, bool isPrivateMatch)
    {
        namespaceSocket = internalDenateConnection.sioClient.socket("/matchgateway");
        if (namespaceSocket)
        {

            if (isPrivateMatch)
            {
                namespaceSocket->on("joinedroom", [&](sio::event& ev) {

                    std::cout << "A player just joined your match" << std::endl;

                    std::string username;
                    std::string servername;
                    bool localisserver = false;

                    auto data = ev.get_message();

                    if (data->get_flag() == sio::message::flag_object)
                    {
                        auto jsonObj = data->get_map();

                        if (jsonObj.find("username") != jsonObj.end())
                        {
                            username = jsonObj["username"]->get_string();
                        }
                        if (jsonObj.find("server_name") != jsonObj.end())
                        {
                            servername = jsonObj["server_name"]->get_string();
                        }
                        if (jsonObj.find("is_server") != jsonObj.end())
                        {
                            bool isServerNum = jsonObj["is_server"]->get_int();
                            isServerNum ? localisserver = true : localisserver = false;
                        }
                    }

                    if (internalPlayerJoinedPrivateMatch)
                    {
                        internalPlayerJoinedPrivateMatch(username, servername, localisserver);
                    }

                    });

                namespaceSocket->on("leftroom", [&](sio::event& ev) {

                    std::cout << "A player just left your match" << std::endl;

                    std::string username;
                    std::string servername;
                    bool localisserver = false;

                    auto data = ev.get_message();

                    if (data->get_flag() == sio::message::flag_object)
                    {
                        auto jsonObj = data->get_map();

                        if (jsonObj.find("username") != jsonObj.end())
                        {
                            username = jsonObj["username"]->get_string();
                        }
                        if (jsonObj.find("server_name") != jsonObj.end())
                        {
                            servername = jsonObj["server_name"]->get_string();
                        }
                        if (jsonObj.find("is_server") != jsonObj.end())
                        {
                            bool isServerNum = jsonObj["is_server"]->get_int();
                            isServerNum ? localisserver = true : localisserver = false;
                        }
                    }

                    if (internalPlayerLeftPrivateMatch)
                    {
                        internalPlayerLeftPrivateMatch(username, servername, localisserver);
                    }

                    });
            }
            else {

                namespaceSocket->on("joinedroom", [&](sio::event& ev) {

                    std::cout << "A player just joined your match" << std::endl;

                    std::string username;
                    std::string servername;
                    bool localisserver = false;

                    auto data = ev.get_message();

                    if (data->get_flag() == sio::message::flag_object)
                    {
                        auto jsonObj = data->get_map();

                        if (jsonObj.find("username") != jsonObj.end())
                        {
                            username = jsonObj["username"]->get_string();
                        }
                        if (jsonObj.find("server_name") != jsonObj.end())
                        {
                            servername = jsonObj["server_name"]->get_string();
                        }
                        if (jsonObj.find("is_server") != jsonObj.end())
                        {
                            bool isServerNum = jsonObj["is_server"]->get_int();
                            isServerNum ? localisserver = true : localisserver = false;
                        }
                    }

                    if (internalPlayerJoinedMatch)
                    {
                        internalPlayerJoinedMatch(username, servername, localisserver);
                    }

                    });

                namespaceSocket->on("leftroom", [&](sio::event& ev) {

                    std::cout << "A player just left your match" << std::endl;

                    std::string username;
                    std::string servername;
                    bool localisserver = false;

                    auto data = ev.get_message();

                    if (data->get_flag() == sio::message::flag_object)
                    {
                        auto jsonObj = data->get_map();

                        if (jsonObj.find("username") != jsonObj.end())
                        {
                            username = jsonObj["username"]->get_string();
                        }
                        if (jsonObj.find("server_name") != jsonObj.end())
                        {
                            servername = jsonObj["server_name"]->get_string();
                        }
                        if (jsonObj.find("is_server") != jsonObj.end())
                        {
                            bool isServerNum = jsonObj["is_server"]->get_int();
                            isServerNum ? localisserver = true : localisserver = false;
                        }
                    }

                    if (internalPlayerLeftMatch)
                    {
                        internalPlayerLeftMatch(username, servername, localisserver);
                    }

                    });

            }

            namespaceSocket->on("getroomdetails", [&](sio::event& ev) {

                DenateRoomDetails roomdetails;
                std::string clientId;
                std::string room;

                auto data = ev.get_message();

                if (data->get_flag() == sio::message::flag_object)
                {
                    auto jsonObj = data->get_map();
                    if (jsonObj.find("client_id") != jsonObj.end())
                    {
                        clientId = jsonObj["client_id"]->get_string();
                    }
                    if (jsonObj.find("room") != jsonObj.end())
                    {
                        room = jsonObj["room"]->get_string();
                    }
                }

                roomdetails.clientId = clientId;
                roomdetails.roomId = room;

                roomDetails = roomdetails;

                });


            namespaceSocket->on("messagebroadcasted", [&](sio::event& ev) {

                DenateRoomDetails roomdetails;
                std::string username;
                std::string message;

                auto data = ev.get_message();

                if (data->get_flag() == sio::message::flag_object)
                {
                    auto jsonObj = data->get_map();
                    if (jsonObj.find("message") != jsonObj.end())
                    {
                        message = jsonObj["message"]->get_string();
                    }
                    if (jsonObj.find("username") != jsonObj.end())
                    {
                        username = jsonObj["username"]->get_string();
                    }
                }

                if (internalMessageBroadcastedToMatch)
                {
                    internalMessageBroadcastedToMatch(username, message);
                }

                });

            if (&internalDenateConnection != nullptr)
            {
                if (internalDenateConnection.isDenateOnlineServiceConnected)
                {
                    sio::message::ptr jsonMessage = sio::object_message::create();

                    jsonMessage->get_map()["username"] = sio::string_message::create(userDetails.username);
                    jsonMessage->get_map()["email"] = sio::string_message::create(std::string(userDetails.emailOrId));
                    jsonMessage->get_map()["room"] = sio::string_message::create(std::string(""));
                    jsonMessage->get_map()["client_id"] = sio::string_message::create(std::string(""));
                    jsonMessage->get_map()["is_server"] = sio::int_message::create(isServer ? 1 : 0);
                    jsonMessage->get_map()["is_private_match"] = sio::int_message::create(isPrivateMatch ? 1 : 0);
                    jsonMessage->get_map()["appID"] = sio::string_message::create(std::string(appID));

                    if (isPrivateMatch)
                    {
                        jsonMessage->get_map()["match_id"] = sio::int_message::create(currentPrivateMatchDetail.matchId);
                        jsonMessage->get_map()["server_name"] = sio::string_message::create(std::string(currentPrivateMatchDetail.serverName));
                    }
                    else {
                        jsonMessage->get_map()["match_id"] = sio::int_message::create(currentMatchDetail.matchId);
                        jsonMessage->get_map()["server_name"] = sio::string_message::create(std::string(currentMatchDetail.serverName));
                    }

                    namespaceSocket->emit("joinroom", jsonMessage, [&](sio::message::list const& ack_msg) {

                        //this->isServer = isServer;

                        isInMatch = true;

                        });

                }
            }

        }
    }

    bool DOS_Local_Match::DeactivateDenateMatchConnection(bool isServer, bool isPrivateMatch)
    {

        if (isInMatch)
        {

            if (&internalDenateConnection != nullptr)
            {
                if (internalDenateConnection.isDenateOnlineServiceConnected)
                {

                    if (namespaceSocket)
                    {
                        sio::message::ptr jsonMessage = sio::object_message::create();

                        jsonMessage->get_map()["username"] = sio::string_message::create(userDetails.username);
                        jsonMessage->get_map()["email"] = sio::string_message::create(std::string(userDetails.emailOrId));
                        jsonMessage->get_map()["room"] = sio::string_message::create(std::string(""));
                        jsonMessage->get_map()["client_id"] = sio::string_message::create(std::string(""));
                        jsonMessage->get_map()["is_server"] = sio::int_message::create(isServer ? 1 : 0);
                        jsonMessage->get_map()["is_private_match"] = sio::int_message::create(isPrivateMatch ? 1 : 0);
                        jsonMessage->get_map()["appID"] = sio::string_message::create(std::string(appID));

                        if (isPrivateMatch)
                        {
                            jsonMessage->get_map()["match_id"] = sio::int_message::create(currentPrivateMatchDetail.matchId);
                            jsonMessage->get_map()["server_name"] = sio::string_message::create(std::string(currentPrivateMatchDetail.serverName));
                        }
                        else {
                            jsonMessage->get_map()["match_id"] = sio::int_message::create(currentMatchDetail.matchId);
                            jsonMessage->get_map()["server_name"] = sio::string_message::create(std::string(currentMatchDetail.serverName));
                        }

                        namespaceSocket->emit("leaveroom", jsonMessage, [&](sio::message::list const& ack_msg) {

                            namespaceSocket->close();

                            });

                    }
                }

                isInMatch = false;

                if (isPrivateMatch)
                {
                    currentPrivateMatchDetail.filters = "";
                    currentPrivateMatchDetail.ipAddress = "";
                    currentPrivateMatchDetail.mapName = "";
                    currentPrivateMatchDetail.maxPlayers = 0;
                    currentPrivateMatchDetail.playerName = "";
                    currentPrivateMatchDetail.serverName = "";
                    currentPrivateMatchDetail.matchId = 0;
                }
                else {
                    currentMatchDetail.filters = "";
                    currentMatchDetail.ipAddress = "";
                    currentMatchDetail.mapName = "";
                    currentMatchDetail.maxPlayers = 0;
                    currentMatchDetail.playerName = "";
                    currentMatchDetail.serverName = "";
                    currentMatchDetail.matchId = 0;
                }

                return true;

            }
            return false;
        }
        else {
            std::cout << "You must be a part of a match to end it" << std::endl;
            return false;
        }
        return true;

    }

    bool DOS_Local_Match::BroadcastMessageToTeam(std::string message)
    {

        if (currentTeamDetail.teamId == "" && currentTeamDetail.serverName == "") {
            std::cout << "A team has to be joined first before you can send a message" << std::endl;
            return false;
        }

        if (&internalDenateConnection != nullptr)
        {
            if (internalDenateConnection.isDenateOnlineServiceConnected)
            {
                if (namespaceTeamSocket)
                {
                    sio::message::ptr jsonMessage = sio::object_message::create();

                    jsonMessage->get_map()["message"] = sio::string_message::create(message);

                    namespaceTeamSocket->emit("broadcastmessagetoteam", jsonMessage, [&](sio::message::list const& ack_msg) {

                        std::cout << "Message sent to team" << std::endl;

                        });
                    return true;
                }

            }
        }
        return false;
    }

    bool DOS_Local_Match::BroadcastMessageToMatch(std::string message)
    {

        if (&internalDenateConnection != nullptr)
        {
            if (internalDenateConnection.isDenateOnlineServiceConnected)
            {
                if (namespaceSocket)
                {
                    sio::message::ptr jsonMessage = sio::object_message::create();

                    jsonMessage->get_map()["message"] = sio::string_message::create(message);

                    namespaceSocket->emit("broadcastmessage", jsonMessage, [&](sio::message::list const& ack_msg) {

                        std::cout << "Message sent to match" << std::endl;

                        });
                    return true;
                }

            }
        }
        return false;
    }

    void DOS_Local_Match::ActivateDenateTeamConnection()
    {
        namespaceTeamSocket = internalDenateConnection.sioClient.socket("/teamgateway");
        if (namespaceTeamSocket)
        {
            namespaceTeamSocket->on("joinedteam", [&](sio::event& ev) {

                std::cout << "A player just joined a team you are a part of" << std::endl;

                DenateTeamPlayersDetails teamplayerdetails;
                std::string username;
                std::string clientId;

                auto data = ev.get_message();

                if (data->get_flag() == sio::message::flag_object)
                {
                    auto jsonObj = data->get_map();

                    if (jsonObj.find("player_name") != jsonObj.end())
                    {
                        username = jsonObj["player_name"]->get_string();
                    }
                    if (jsonObj.find("client_id") != jsonObj.end())
                    {
                        clientId = jsonObj["client_id"]->get_string();
                    }
                }

                teamplayerdetails.clientId = clientId;
                teamplayerdetails.playerName = username;

                currentTeamPlayers.push_back(teamplayerdetails);

                if (internalPlayerJoinTeam)
                {
                    internalPlayerJoinTeam(teamplayerdetails);
                }

                });

            namespaceTeamSocket->on("leftteam", [&](sio::event& ev) {

                std::cout << "A player just left the team you are a part of" << std::endl;

                DenateTeamPlayersDetails teamplayerdetails;
                std::string username;
                std::string clientId;

                auto data = ev.get_message();

                if (data->get_flag() == sio::message::flag_object)
                {
                    auto jsonObj = data->get_map();

                    if (jsonObj.find("player_name") != jsonObj.end())
                    {
                        username = jsonObj["player_name"]->get_string();
                    }
                    if (jsonObj.find("client_id") != jsonObj.end())
                    {
                        clientId = jsonObj["client_id"]->get_string();
                    }
                }

                teamplayerdetails.clientId = clientId;
                teamplayerdetails.playerName = username;

                std::vector<int> indexestodelete;
                for (int i = 0; i < currentTeamPlayers.size(); i++)
                {
                    if (currentTeamPlayers[i].playerName == teamplayerdetails.playerName) {
                        indexestodelete.push_back(i);
                    }
                }

                std::sort(indexestodelete.rbegin(), indexestodelete.rend());
                for (int index : indexestodelete)
                {
                    if (index >= 0 && index < currentTeamPlayers.size())
                    {
                        currentTeamPlayers.erase(currentTeamPlayers.begin() + index);
                    }
                }


                if (internalPlayerLeaveTeam)
                {
                    internalPlayerLeaveTeam(teamplayerdetails);
                }

                });

            namespaceTeamSocket->on("destroyedteam", [&](sio::event& ev) {

                std::string teamID;

                auto data = ev.get_message();

                if (data->get_flag() == sio::message::flag_object)
                {
                    auto jsonObj = data->get_map();
                    if (jsonObj.find("teamID") != jsonObj.end())
                    {
                        teamID = jsonObj["teamID"]->get_string();
                    }

                }

                if (currentTeamDetail.teamId == teamID)
                {
                    currentTeamDetail.filters = "";
                    currentTeamDetail.isPrivateMatch = false;
                    currentTeamDetail.matchId = 0;
                    currentTeamDetail.maxPlayers = 0;
                    currentTeamDetail.serverName = "";
                    currentTeamDetail.teamId = "";

                    currentTeamPlayers.clear();

                    if (internalPlayerDestroyTeam)
                    {
                        internalPlayerDestroyTeam(teamID);
                    }

                }

                });


            namespaceTeamSocket->on("messagebroadcastedtoteam", [&](sio::event& ev) {

                std::string username;
                std::string message;

                auto data = ev.get_message();

                if (data->get_flag() == sio::message::flag_object)
                {
                    auto jsonObj = data->get_map();
                    if (jsonObj.find("message") != jsonObj.end())
                    {
                        message = jsonObj["message"]->get_string();
                    }
                    if (jsonObj.find("username") != jsonObj.end())
                    {
                        username = jsonObj["username"]->get_string();
                    }
                }

                if (internalMessageBroadcastedToTeam)
                {
                    internalMessageBroadcastedToTeam(username, message);
                }

                });

            if (&internalDenateConnection != nullptr)
            {
                if (internalDenateConnection.isDenateOnlineServiceConnected)
                {
                    sio::message::ptr jsonMessage = sio::object_message::create();

                    jsonMessage->get_map()["player_name"] = sio::string_message::create(userDetails.username);
                    jsonMessage->get_map()["teamID"] = sio::string_message::create(std::string(currentTeamDetail.teamId));
                    jsonMessage->get_map()["userID"] = sio::string_message::create(std::string(userID));
                    jsonMessage->get_map()["client_id"] = sio::string_message::create(std::string(""));
                    jsonMessage->get_map()["appID"] = sio::string_message::create(std::string(appID));

                    if (isPrivateMatch)
                    {
                        jsonMessage->get_map()["match_id"] = sio::int_message::create(currentPrivateMatchDetail.matchId);
                    }
                    else {
                        jsonMessage->get_map()["match_id"] = sio::int_message::create(currentMatchDetail.matchId);
                    }

                    namespaceTeamSocket->emit("jointeam", jsonMessage, [&](sio::message::list const& ack_msg) {

                        });

                }
            }

        }
    }

}