#include "DOS_Achievement.h"
#include <stdio.h>
#include <iostream>
#include <cstring>

namespace DenateAchievement {

    size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
    {

        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;

    }

    void  DenateAchievement::DOS_Achievement::replaceSubstring(std::string& original, const std::string& toReplace, const std::string& replacement)
    {
        size_t pos = 0;
        while ((pos = original.find(toReplace, pos)) != std::string::npos)
        {
            original.replace(pos, toReplace.length(), replacement);
            pos += replacement.length();
        }
    }

    DOS_Achievement::DOS_Achievement(std::string userID, std::string appID, bool dedicatedServer, std::string token, DenateUserDetails userDetails)
    {
        this->userID = userID;
        this->appID = appID;
        this->dedicatedServer = dedicatedServer;
        this->token = token;
        this->userDetails = userDetails;
    }

    GetAllGameAchievementsResult DenateAchievement::DOS_Achievement::GetAllGameAchievements()
    {
        CURL* curl;
        CURLcode res;
        GetAllGameAchievementsResult result;
        std::string readBuffer;
        bool gottenAchievement = false;
        DenateHTTPResponse httpResponse;
        std::vector<DenateAchievementDetails> localachievements;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            //replaceSubstring(EmailOrIdOrUsername, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("achievement/getallgameachievements"));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            //headers = curl_slist_append(headers, "Content-Type: application/json");
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
            gottenAchievement = true;

            if (jsonResponse["response"].is_array())
            {
                for (const auto& achievement : jsonResponse["response"])
                {
                    DenateAchievementDetails localachievementdetail;

                    localachievementdetail.achievementDescription = !achievement["achievement_description"].is_null() ? achievement["achievement_description"] : "";
                    localachievementdetail.achievementName = !achievement["achievement_name"].is_null() ? achievement["achievement_name"] : "";
                    localachievementdetail.lockedAchievementName = !achievement["locked_achievement_name"].is_null() ? achievement["locked_achievement_name"] : "";
                    localachievementdetail.lockedAchievementDescription = !achievement["locked_achievement_description"].is_null() ? achievement["locked_achievement_description"] : "";
                    localachievementdetail.unlockedAchievementName = !achievement["unlocked_achievement_name"].is_null() ? achievement["unlocked_achievement_name"] : "";
                    localachievementdetail.unlockedAchievementDescription = !achievement["unlocked_achievement_description"].is_null() ? achievement["unlocked_achievement_description"] : "";
                    localachievementdetail.achievementID = !achievement["achievementID"].is_null() ? achievement["achievementID"] : "";
                    if (!achievement["attachment"].is_null())
                    {
                        localachievementdetail.lockedAchievementImageURL = !achievement["attachment"]["locked_icon_URL"].is_null() ? achievement["attachment"]["locked_icon_URL"] : "";
                        localachievementdetail.unlockedAchievementImageURL = !achievement["attachment"]["unlocked_icon_URL"].is_null() ? achievement["attachment"]["unlocked_icon_URL"] : "";
                    }
                    else {
                        localachievementdetail.lockedAchievementImageURL = "";
                        localachievementdetail.unlockedAchievementImageURL = "";
                    }
                    
                    localachievements.push_back(localachievementdetail);
                }
            }

        }

        result.httpResponse = httpResponse;
        result.gottenAchievement = gottenAchievement;
        result.achievementDetails = localachievements;

        curl_global_cleanup();

        return result;
    }

    GetAllGameAchievementsResult DenateAchievement::DOS_Achievement::GetAllPlayerUnlockedAchievements(std::string playerName)
    {
        CURL* curl;
        CURLcode res;
        GetAllGameAchievementsResult result;
        std::string readBuffer;
        bool gottenAchievement = false;
        DenateHTTPResponse httpResponse;
        std::vector<DenateAchievementDetails> localachievements;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            replaceSubstring(playerName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("achievement/getallplayerunlockedachievement/") + std::string(playerName));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            //headers = curl_slist_append(headers, "Content-Type: application/json");
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
            gottenAchievement = true;

            if (jsonResponse["response"].is_array())
            {
                for (const auto& achievement : jsonResponse["response"])
                {
                    DenateAchievementDetails localachievementdetail;

                    if (!achievement["playerachievement"].is_null())
                    {
                        localachievementdetail.achievementDescription = !achievement["playerachievement"]["achievement_description"].is_null() ? achievement["playerachievement"]["achievement_description"] : "";
                        localachievementdetail.achievementName = !achievement["playerachievement"]["achievement_name"].is_null() ? achievement["playerachievement"]["achievement_name"] : "";
                        localachievementdetail.lockedAchievementName = !achievement["playerachievement"]["locked_achievement_name"].is_null() ? achievement["playerachievement"]["locked_achievement_name"] : "";
                        localachievementdetail.lockedAchievementDescription = !achievement["playerachievement"]["locked_achievement_description"].is_null() ? achievement["playerachievement"]["locked_achievement_description"] : "";
                        localachievementdetail.unlockedAchievementName = !achievement["playerachievement"]["unlocked_achievement_name"].is_null() ? achievement["playerachievement"]["unlocked_achievement_name"] : "";
                        localachievementdetail.unlockedAchievementDescription = !achievement["playerachievement"]["unlocked_achievement_description"].is_null() ? achievement["playerachievement"]["unlocked_achievement_description"] : "";
                        localachievementdetail.achievementID = !achievement["playerachievement"]["achievementID"].is_null() ? achievement["playerachievement"]["achievementID"] : "";
                        if (!achievement["playerachievement"]["attachment"].is_null())
                        {
                            localachievementdetail.lockedAchievementImageURL = !achievement["playerachievement"]["attachment"]["locked_icon_URL"].is_null() ? achievement["playerachievement"]["attachment"]["locked_icon_URL"] : "";
                            localachievementdetail.unlockedAchievementImageURL = !achievement["playerachievement"]["attachment"]["unlocked_icon_URL"].is_null() ? achievement["playerachievement"]["attachment"]["unlocked_icon_URL"] : "";
                        }
                        else {
                            localachievementdetail.lockedAchievementImageURL = "";
                            localachievementdetail.unlockedAchievementImageURL = "";
                        }
                    }

                    localachievements.push_back(localachievementdetail);
                }
            }

        }

        result.httpResponse = httpResponse;
        result.gottenAchievement = gottenAchievement;
        result.achievementDetails = localachievements;

        curl_global_cleanup();

        return result;
    }

    GetAllGameAchievementsResult DenateAchievement::DOS_Achievement::GetAllPlayerLockedAchievements(std::string playerName)
    {
        CURL* curl;
        CURLcode res;
        GetAllGameAchievementsResult result;
        std::string readBuffer;
        bool gottenAchievement = false;
        DenateHTTPResponse httpResponse;
        std::vector<DenateAchievementDetails> localachievements;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            replaceSubstring(playerName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("achievement/getallplayerlockedachievement/") + std::string(playerName));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            //headers = curl_slist_append(headers, "Content-Type: application/json");
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
            gottenAchievement = true;

            if (jsonResponse["response"].is_array())
            {
                for (const auto& achievement : jsonResponse["response"])
                {
                    DenateAchievementDetails localachievementdetail;

                    localachievementdetail.achievementDescription = !achievement["achievement_description"].is_null() ? achievement["achievement_description"] : "";
                    localachievementdetail.achievementName = !achievement["achievement_name"].is_null() ? achievement["achievement_name"] : "";
                    localachievementdetail.lockedAchievementName = !achievement["locked_achievement_name"].is_null() ? achievement["locked_achievement_name"] : "";
                    localachievementdetail.lockedAchievementDescription = !achievement["locked_achievement_description"].is_null() ? achievement["locked_achievement_description"] : "";
                    localachievementdetail.unlockedAchievementName = !achievement["unlocked_achievement_name"].is_null() ? achievement["unlocked_achievement_name"] : "";
                    localachievementdetail.unlockedAchievementDescription = !achievement["unlocked_achievement_description"].is_null() ? achievement["unlocked_achievement_description"] : "";
                    localachievementdetail.achievementID = !achievement["achievementID"].is_null() ? achievement["achievementID"] : "";
                    if (!achievement["attachment"].is_null())
                    {
                        localachievementdetail.lockedAchievementImageURL = !achievement["attachment"]["locked_icon_URL"].is_null() ? achievement["attachment"]["locked_icon_URL"] : "";
                        localachievementdetail.unlockedAchievementImageURL = !achievement["attachment"]["unlocked_icon_URL"].is_null() ? achievement["attachment"]["unlocked_icon_URL"] : "";
                    }
                    else {
                        localachievementdetail.lockedAchievementImageURL = "";
                        localachievementdetail.unlockedAchievementImageURL = "";
                    }

                    localachievements.push_back(localachievementdetail);
                }
            }

        }

        result.httpResponse = httpResponse;
        result.gottenAchievement = gottenAchievement;
        result.achievementDetails = localachievements;

        curl_global_cleanup();

        return result;
    }

    LockOrUnlockAchievementByAchievementResult DenateAchievement::DOS_Achievement::LockOrUnlockAchievementByAchievementName(std::string achievementName, bool Unlock, std::string playerName)
    {
        CURL* curl;
        CURLcode res;
        LockOrUnlockAchievementByAchievementResult result;
        std::string readBuffer;
        bool achievementUpdated = false;
        DenateHTTPResponse httpResponse;
        DenateAchievementDetails localachievements;
        bool achievementUnlocked;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            replaceSubstring(playerName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("achievement/unlockorlockachievementbyname"));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            std::string unlockString = Unlock ? "false" : "true";

            std::string json = R"({"appID": ")" + appID + "\"" + "," + R"("userID": ")" + userID + "\"" + "," + R"("player_name": ")" + playerName + "\"" + "," + R"("appuserID": ")" + userDetails.appUserID + "\"" + "," + R"("achievement_name": ")" + achievementName + "\"" + "," + R"("locked": )" + unlockString + R"(})";
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());

            struct curl_slist* headers = NULL;
            //headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("appID: " + std::string(appID)).c_str());
            headers = curl_slist_append(headers, ("userID: " + std::string(userID)).c_str());

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
            achievementUpdated = true;

            localachievements.achievementDescription = !jsonResponse["response"]["achievement_description"].is_null() ? jsonResponse["response"]["achievement_description"] : "";
            localachievements.achievementName = !jsonResponse["response"]["achievement_name"].is_null() ? jsonResponse["response"]["achievement_name"] : "";
            localachievements.lockedAchievementName = !jsonResponse["response"]["locked_achievement_name"].is_null() ? jsonResponse["response"]["locked_achievement_name"] : "";
            localachievements.lockedAchievementDescription = !jsonResponse["response"]["locked_achievement_description"].is_null() ? jsonResponse["response"]["locked_achievement_description"] : "";
            localachievements.unlockedAchievementName = !jsonResponse["response"]["unlocked_achievement_name"].is_null() ? jsonResponse["response"]["unlocked_achievement_name"] : "";
            localachievements.unlockedAchievementDescription = !jsonResponse["response"]["unlocked_achievement_description"].is_null() ? jsonResponse["response"]["unlocked_achievement_description"] : "";
            localachievements.achievementID = !jsonResponse["response"]["achievementID"].is_null() ? jsonResponse["response"]["achievementID"] : "";
            if (!jsonResponse["response"]["attachment"].is_null())
            {
                localachievements.lockedAchievementImageURL = !jsonResponse["response"]["attachment"]["locked_icon_URL"].is_null() ? jsonResponse["response"]["attachment"]["locked_icon_URL"] : "";
                localachievements.unlockedAchievementImageURL = !jsonResponse["response"]["attachment"]["unlocked_icon_URL"].is_null() ? jsonResponse["response"]["attachment"]["unlocked_icon_URL"] : "";
            }
            else {
                localachievements.lockedAchievementImageURL = "";
                localachievements.unlockedAchievementImageURL = "";
            }
            achievementUnlocked = jsonResponse["response"]["locked"];
        }

        result.httpResponse = httpResponse;
        result.achievementUpdated = achievementUpdated;
        result.achievementDetails = localachievements;
        result.achievementUnlocked = !achievementUnlocked;

        curl_global_cleanup();

        return result;
    }

    LockOrUnlockAchievementByAchievementResult DenateAchievement::DOS_Achievement::LockOrUnlockAchievementByAchievementID(std::string achievementID, bool Unlock, std::string playerName)
    {
        CURL* curl;
        CURLcode res;
        LockOrUnlockAchievementByAchievementResult result;
        std::string readBuffer;
        bool achievementUpdated = false;
        DenateHTTPResponse httpResponse;
        DenateAchievementDetails localachievements;
        bool achievementUnlocked;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            replaceSubstring(playerName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("achievement/unlockorlockachievementbyachievementID"));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            std::string unlockString = Unlock ? "false" : "true";

            std::string json = R"({"appID": ")" + appID + "\"" + "," + R"("userID": ")" + userID + "\"" + "," + R"("player_name": ")" + playerName + "\"" + "," + R"("appuserID": ")" + userDetails.appUserID + "\"" + "," + R"("achievement_name": ")" + "" + "\"" + "," + R"("achievementID": ")" + achievementID + "\"" + "," + R"("locked": )" + unlockString + R"(})";
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());

            struct curl_slist* headers = NULL;
            //headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("appID: " + std::string(appID)).c_str());
            headers = curl_slist_append(headers, ("userID: " + std::string(userID)).c_str());

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
            achievementUpdated = true;

            localachievements.achievementDescription = !jsonResponse["response"]["achievement_description"].is_null() ? jsonResponse["response"]["achievement_description"] : "";
            localachievements.achievementName = !jsonResponse["response"]["achievement_name"].is_null() ? jsonResponse["response"]["achievement_name"] : "";
            localachievements.lockedAchievementName = !jsonResponse["response"]["locked_achievement_name"].is_null() ? jsonResponse["response"]["locked_achievement_name"] : "";
            localachievements.lockedAchievementDescription = !jsonResponse["response"]["locked_achievement_description"].is_null() ? jsonResponse["response"]["locked_achievement_description"] : "";
            localachievements.unlockedAchievementName = !jsonResponse["response"]["unlocked_achievement_name"].is_null() ? jsonResponse["response"]["unlocked_achievement_name"] : "";
            localachievements.unlockedAchievementDescription = !jsonResponse["response"]["unlocked_achievement_description"].is_null() ? jsonResponse["response"]["unlocked_achievement_description"] : "";
            localachievements.achievementID = !jsonResponse["response"]["achievementID"].is_null() ? jsonResponse["response"]["achievementID"] : "";
            if (!jsonResponse["response"]["attachment"].is_null())
            {
                localachievements.lockedAchievementImageURL = !jsonResponse["response"]["attachment"]["locked_icon_URL"].is_null() ? jsonResponse["response"]["attachment"]["locked_icon_URL"] : "";
                localachievements.unlockedAchievementImageURL = !jsonResponse["response"]["attachment"]["unlocked_icon_URL"].is_null() ? jsonResponse["response"]["attachment"]["unlocked_icon_URL"] : "";
            }
            else {
                localachievements.lockedAchievementImageURL = "";
                localachievements.unlockedAchievementImageURL = "";
            }
            achievementUnlocked = jsonResponse["response"]["locked"];
        }

        result.httpResponse = httpResponse;
        result.achievementUpdated = achievementUpdated;
        result.achievementDetails = localachievements;
        result.achievementUnlocked = !achievementUnlocked;

        curl_global_cleanup();

        return result;
    }

    GetAllPlayerAchievementsResult DenateAchievement::DOS_Achievement::GetAllPlayerAchievements(std::string playerName)
    {
        CURL* curl;
        CURLcode res;
        GetAllPlayerAchievementsResult result;
        std::string readBuffer;
        bool gottenAchievement = false;
        DenateHTTPResponse httpResponse;
        std::vector<DenatePlayerAchievementDetails> localachievements;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            replaceSubstring(playerName, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("achievement/getallplayerachievements/") + std::string(playerName));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            //headers = curl_slist_append(headers, "Content-Type: application/json");
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
            gottenAchievement = true;

            if (jsonResponse["response"].is_array())
            {
                for (const auto& achievement : jsonResponse["response"])
                {
                    DenatePlayerAchievementDetails localachievementdetail;

                    localachievementdetail.achievementDescription = !achievement["achievement_description"].is_null() ? achievement["achievement_description"] : "";
                    localachievementdetail.achievementName = !achievement["achievement_name"].is_null() ? achievement["achievement_name"] : "";
                    localachievementdetail.lockedAchievementName = !achievement["locked_achievement_name"].is_null() ? achievement["locked_achievement_name"] : "";
                    localachievementdetail.lockedAchievementDescription = !achievement["locked_achievement_description"].is_null() ? achievement["locked_achievement_description"] : "";
                    localachievementdetail.unlockedAchievementName = !achievement["unlocked_achievement_name"].is_null() ? achievement["unlocked_achievement_name"] : "";
                    localachievementdetail.unlockedAchievementDescription = !achievement["unlocked_achievement_description"].is_null() ? achievement["unlocked_achievement_description"] : "";
                    localachievementdetail.achievementID = !achievement["achievementID"].is_null() ? achievement["achievementID"] : "";
                    if (!achievement["attachment"].is_null())
                    {
                        localachievementdetail.lockedAchievementImageURL = !achievement["attachment"]["locked_icon_URL"].is_null() ? achievement["attachment"]["locked_icon_URL"] : "";
                        localachievementdetail.unlockedAchievementImageURL = !achievement["attachment"]["unlocked_icon_URL"].is_null() ? achievement["attachment"]["unlocked_icon_URL"] : "";
                    }
                    else {
                        localachievementdetail.lockedAchievementImageURL = "";
                        localachievementdetail.unlockedAchievementImageURL = "";
                    }
                    localachievementdetail.unlocked = !achievement["locked"];

                    localachievements.push_back(localachievementdetail);
                }
            }

        }

        result.httpResponse = httpResponse;
        result.gottenAchievement = gottenAchievement;
        result.achievementDetails = localachievements;

        curl_global_cleanup();

        return result;
    }

}