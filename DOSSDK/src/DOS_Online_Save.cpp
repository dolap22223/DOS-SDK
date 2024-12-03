// Copyright 2024, David Olashege, All rights reserved

#include "DOS_Online_Save.h"
#include <iostream>

namespace DenateOnlineSave
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

    void DOS_Online_Save::replaceSubstring(std::string& original, const std::string& toReplace, const std::string& replacement)
    {
        size_t pos = 0;
        while ((pos = original.find(toReplace, pos)) != std::string::npos)
        {
            original.replace(pos, toReplace.length(), replacement);
            pos += replacement.length();
        }
    }

    DOS_Online_Save::DOS_Online_Save(std::string userID, std::string appID, bool dedicatedServer, std::string token, DenateUserDetails userDetails)
    {
        this->userID = userID;
        this->appID = appID;
        this->dedicatedServer = dedicatedServer;
        this->token = token;
        this->userDetails = userDetails;
    }

    AddOnlineSaveResult DenateOnlineSave::DOS_Online_Save::AddOnlineSave(std::vector<int> OutSaveData, std::string Tag, std::string Filters)
    {
        CURL* curl;
        CURLcode res;
        AddOnlineSaveResult result;
        std::string readBuffer;
        bool saved = false;
        DenateHTTPResponse httpResponse;
        DenateOnlineSaveDetail savedetail;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            //replaceSubstring(Tag, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denateunrealsave/createunrealsave"));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            std::string byteString = "[";

            for (int i = 0; i < OutSaveData.size(); i++)
            {
                if (i == OutSaveData.size() - 1)
                {
                    byteString = byteString + "{" + "\"" + "byte" + "\"" + ":" + std::to_string(OutSaveData[i]) + "}";
                }
                else {
                    byteString = byteString + "{" + "\"" + "byte" + "\"" + ":" + std::to_string(OutSaveData[i]) + "}" + ",";
                }
            }

            byteString = byteString + "]";

            std::string json = R"({"appID": ")" + appID + "\"" + "," + R"("userID": ")" + userID + "\"" + "," + R"("player_name": ")" + userDetails.username + "\"" + "," + R"("filters": ")" + Filters + "\"" + "," + R"("tag": ")" + Tag + "\"" + "," + R"("objectByte": )" + byteString + R"(})";
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
            saved = true;

            savedetail.filters = !jsonResponse["response"]["filters"].is_null() ? jsonResponse["response"]["filters"] : "";
            savedetail.SaveID = !jsonResponse["response"]["saveID"].is_null() ? jsonResponse["response"]["saveID"] : "";
            savedetail.Tag = !jsonResponse["response"]["tag"].is_null() ? jsonResponse["response"]["tag"] : "";
        }

        result.httpResponse = httpResponse;
        result.saved = saved;
        result.saveDetail = savedetail;

        curl_global_cleanup();

        return result;
    }

    GetOnlineSaveResult DenateOnlineSave::DOS_Online_Save::GetSaveByTag(std::string Tag, std::string playerName)
    {
        CURL* curl;
        CURLcode res;
        GetOnlineSaveResult result;
        std::string readBuffer;
        bool saveGotten = false;
        DenateHTTPResponse httpResponse;
        DenatePlayerOnlineSaveDetail savedetail;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            if (playerName == "")
            {
                playerName = userDetails.username;
            }

            replaceSubstring(Tag, " ", "%20");
            replaceSubstring(playerName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denateunrealsave/getunrealsavebytag/") + std::string(playerName) + "/" + std::string(Tag));
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
        std::cout << readBuffer << std::endl;

        nlohmann::json jsonResponse = nlohmann::json::parse(readBuffer);

        httpResponse.status_code = jsonResponse["status"];
        httpResponse.message = jsonResponse["message"];

        if (jsonResponse.contains("response"))
        {
            saveGotten = true;

            savedetail.filters = !jsonResponse["response"]["filters"].is_null() ? jsonResponse["response"]["filters"] : "";
            savedetail.SaveID = !jsonResponse["response"]["saveID"].is_null() ? jsonResponse["response"]["saveID"] : "";
            savedetail.Tag = !jsonResponse["response"]["tag"].is_null() ? jsonResponse["response"]["tag"] : "";

            if (jsonResponse["response"]["objectByte"].is_array())
            {
                for (const auto& bytes : jsonResponse["response"]["objectByte"])
                {
                    savedetail.objectBytes.push_back(bytes["byte"]);
                }
            }

        }

        result.httpResponse = httpResponse;
        result.saveGotten = saveGotten;
        result.saveDetail = savedetail;

        curl_global_cleanup();

        return result;
    }

    GetOnlineSaveResult DenateOnlineSave::DOS_Online_Save::GetSaveBySaveID(std::string saveID, std::string playerName)
    {
        CURL* curl;
        CURLcode res;
        GetOnlineSaveResult result;
        std::string readBuffer;
        bool saveGotten = false;
        DenateHTTPResponse httpResponse;
        DenatePlayerOnlineSaveDetail savedetail;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            if (playerName == "")
            {
                playerName = userDetails.username;
            }

            replaceSubstring(saveID, " ", "%20");
            replaceSubstring(playerName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denateunrealsave/getunrealsavebysaveId/") + std::string(playerName) + "/" + std::string(saveID));
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
        std::cout << readBuffer << std::endl;

        nlohmann::json jsonResponse = nlohmann::json::parse(readBuffer);

        httpResponse.status_code = jsonResponse["status"];
        httpResponse.message = jsonResponse["message"];

        if (jsonResponse.contains("response"))
        {
            saveGotten = true;

            savedetail.filters = !jsonResponse["response"]["filters"].is_null() ? jsonResponse["response"]["filters"] : "";
            savedetail.SaveID = !jsonResponse["response"]["saveID"].is_null() ? jsonResponse["response"]["saveID"] : "";
            savedetail.Tag = !jsonResponse["response"]["tag"].is_null() ? jsonResponse["response"]["tag"] : "";

            if (jsonResponse["response"]["objectByte"].is_array())
            {
                for (const auto& bytes : jsonResponse["response"]["objectByte"])
                {
                    savedetail.objectBytes.push_back(bytes["byte"]);
                }
            }

        }

        result.httpResponse = httpResponse;
        result.saveGotten = saveGotten;
        result.saveDetail = savedetail;

        curl_global_cleanup();

        return result;
    }

    GetOnlineSaveResult DenateOnlineSave::DOS_Online_Save::GetLatestPlayerSave(std::string playerName)
    {
        CURL* curl;
        CURLcode res;
        GetOnlineSaveResult result;
        std::string readBuffer;
        bool saveGotten = false;
        DenateHTTPResponse httpResponse;
        DenatePlayerOnlineSaveDetail savedetail;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            if (playerName == "")
            {
                playerName = userDetails.username;
            }

            replaceSubstring(playerName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denateunrealsave/getlatestunrealplayersave/") + std::string(playerName));
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
            saveGotten = true;

            savedetail.filters = !jsonResponse["response"]["filters"].is_null() ? jsonResponse["response"]["filters"] : "";
            savedetail.SaveID = !jsonResponse["response"]["saveID"].is_null() ? jsonResponse["response"]["saveID"] : "";
            savedetail.Tag = !jsonResponse["response"]["tag"].is_null() ? jsonResponse["response"]["tag"] : "";

            if (jsonResponse["response"]["objectByte"].is_array())
            {
                for (const auto& bytes : jsonResponse["response"]["objectByte"])
                {
                    savedetail.objectBytes.push_back(bytes["byte"]);
                }
            }

        }

        result.httpResponse = httpResponse;
        result.saveGotten = saveGotten;
        result.saveDetail = savedetail;

        curl_global_cleanup();

        return result;
    }

    GetAllPlayerSavesResult DOS_Online_Save::GetAllPlayerSaves(std::string playerName)
    {
        CURL* curl;
        CURLcode res;
        GetAllPlayerSavesResult result;
        std::string readBuffer;
        bool saveGotten = false;
        DenateHTTPResponse httpResponse;
        std::vector<DenatePlayerOnlineSaveDetail> savedetails;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            if (playerName == "")
            {
                playerName = userDetails.username;
            }

            replaceSubstring(playerName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denateunrealsave/getunrealsave/") + std::string(playerName));
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
            saveGotten = true;

            if (jsonResponse["response"].is_array())
            {
                for (const auto& saves : jsonResponse["response"])
                {

                    DenatePlayerOnlineSaveDetail savedetail;

                    savedetail.filters = !saves["filters"].is_null() ? saves["filters"] : "";
                    savedetail.SaveID = !saves["saveID"].is_null() ? saves["saveID"] : "";
                    savedetail.Tag = !saves["tag"].is_null() ? saves["tag"] : "";

                    if (saves["objectByte"].is_array())
                    {
                        for (const auto& bytes : saves["objectByte"])
                        {
                            savedetail.objectBytes.push_back(bytes["byte"]);
                        }
                    }

                    savedetails.push_back(savedetail);

                }
            }
        }

        result.httpResponse = httpResponse;
        result.saveGotten = saveGotten;
        result.saveDetails = savedetails;

        curl_global_cleanup();

        return result;
    }

    DeleteSaveResult DOS_Online_Save::DeleteSaveByTag(std::string Tag, std::string playerName)
    {
        CURL* curl;
        CURLcode res;
        DeleteSaveResult result;
        std::string readBuffer;
        bool saveDeleted = false;
        DenateHTTPResponse httpResponse;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            if (playerName == "")
            {
                playerName = userDetails.username;
            }

            replaceSubstring(playerName, " ", "%20");
            replaceSubstring(Tag, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denateunrealsave/deleteunrealsavebytag/") + std::string(playerName) + "/" + std::string(Tag));
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

        if (jsonResponse["status"] >= 200 && jsonResponse["status"] < 300)
        {
            saveDeleted = true;
        }

        result.httpResponse = httpResponse;
        result.saveDeleted = saveDeleted;

        curl_global_cleanup();

        return result;
    }

    DeleteSaveResult DOS_Online_Save::DeleteSaveBySaveID(std::string saveID, std::string playerName)
    {
        CURL* curl;
        CURLcode res;
        DeleteSaveResult result;
        std::string readBuffer;
        bool saveDeleted = false;
        DenateHTTPResponse httpResponse;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            if (playerName == "")
            {
                playerName = userDetails.username;
            }

            replaceSubstring(playerName, " ", "%20");
            replaceSubstring(saveID, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("denateunrealsave/deleteunrealsavebysaveId/") + std::string(playerName) + "/" + std::string(saveID));
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

        if (jsonResponse["status"] >= 200 && jsonResponse["status"] < 300)
        {
            saveDeleted = true;
        }

        result.httpResponse = httpResponse;
        result.saveDeleted = saveDeleted;

        curl_global_cleanup();

        return result;
    }

    std::vector<DenateFilterResult> DOS_Online_Save::BreakDenateFilter(std::string Filter)
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

    std::string DOS_Online_Save::MakeDenateFilter(std::vector<DenateFilterResult> filters)
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