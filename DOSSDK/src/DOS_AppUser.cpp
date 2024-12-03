// Copyright 2024, David Olashege, All rights reserved

#include "DOS_AppUser.h"
#include "curl/curl.h"
#include "DenateTypes.h"
#include <stdio.h>
#include <iostream>
#include <cstring>

namespace DenateAppUser {

    void DOS_AppUser::replaceSubstring(std::string& original, const std::string& toReplace, const std::string& replacement)
    {
        size_t pos = 0; 
        while ((pos = original.find(toReplace, pos)) != std::string::npos)
        {
            original.replace(pos, toReplace.length(), replacement);
            pos += replacement.length();
        }
    }

    std::string DOS_AppUser::getToken()
    {
        return token;
    }

    DenateUserDetails DOS_AppUser::getUserDetails()
    {
        /*DenateUserDetails localuserdetails; 
        localuserdetails.emailOrId = userDetails.emailOrId;
        localuserdetails.username = userDetails.username;
        localuserdetails.firstName = userDetails.firstName;
        localuserdetails.lastName = userDetails.lastName;
        localuserdetails.picture = userDetails.picture;
        localuserdetails.appUserID = userDetails.appUserID;*/
        
        return userDetails;
    }

    DOS_AppUser::DOS_AppUser(std::string userID, std::string appID, bool dedicatedServer)
    {
        this->userID = userID;
        this->appID = appID;
        this->dedicatedServer = dedicatedServer;
    }

    size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) 
    {

        ((std::string*)userp)->append((char*)contents, size * nmemb); 
        return size * nmemb;

    }

    LoginAppUserResult DOS_AppUser::LoginDenateAppUserById(std::string EmailOrIdOrUsername)
	{
        CURL* curl;
        CURLcode res;
        LoginAppUserResult result;
        std::string readBuffer;
        bool loggedIn = false;
        DenateHTTPResponse httpResponse;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {
            
            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("appuser/appuserloginbyid"));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            
            //std::string postfield = "email_or_username=" + EmailOrIdOrUsername;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str()); //+ "&" + "password=" + Password);

            std::string json = R"({"email_or_username": ")" + EmailOrIdOrUsername + R"("})";
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());

            struct curl_slist* headers = NULL;
            //headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("appID: " + std::string(appID)).c_str());
            headers = curl_slist_append(headers, ("userID: " + std::string(userID)).c_str());
            headers = curl_slist_append(headers, ("appuserID: " + std::string(userDetails.appUserID)).c_str());

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
            loggedIn = true;

            userDetails.emailOrId = jsonResponse["response"]["email"];
            userDetails.username = jsonResponse["response"]["username"];
            userDetails.firstName = jsonResponse["response"]["first_name"];
            userDetails.lastName = jsonResponse["response"]["last_name"];
            userDetails.picture = jsonResponse["response"]["picture"];
            userDetails.appUserID = jsonResponse["response"]["appuserID"];

            token = jsonResponse["token"];
            result.token = jsonResponse["token"];
        }

        result.httpResponse = httpResponse;
        result.loggedIn = loggedIn;
        result.userDetails = userDetails;

        curl_global_cleanup();

        return result;
	}

    LoginAppUserResult DOS_AppUser::LoginDenateAppUser(std::string EmailOrIdOrUsername, std::string Password)
    {
        CURL* curl;
        CURLcode res;
        LoginAppUserResult result;
        std::string readBuffer;
        bool loggedIn = false;
        DenateHTTPResponse httpResponse;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("appuser/appuserlogin"));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            std::string postfield = "email_or_username=" + EmailOrIdOrUsername + "&password=" + Password;
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str());

            /*std::string json = R"({"email_or_username": ")" + EmailOrIdOrUsername + R"("})";
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());*/

            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            //headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("appID: " + std::string(appID)).c_str());
            headers = curl_slist_append(headers, ("userID: " + std::string(userID)).c_str());
            headers = curl_slist_append(headers, ("appuserID: " + std::string(userDetails.appUserID)).c_str());

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
            loggedIn = true;

            userDetails.emailOrId = jsonResponse["response"]["email"];
            userDetails.username = jsonResponse["response"]["username"];
            userDetails.firstName = jsonResponse["response"]["first_name"];
            userDetails.lastName = jsonResponse["response"]["last_name"];
            userDetails.picture = jsonResponse["response"]["picture"];
            userDetails.appUserID = jsonResponse["response"]["appuserID"];

            token = jsonResponse["token"];
            result.token = jsonResponse["token"];
        }

        result.httpResponse = httpResponse;
        result.loggedIn = loggedIn;
        result.userDetails = userDetails;

        curl_global_cleanup();

        return result;
    }

    SignUpAppUserResult DOS_AppUser::SignUpDenateAppUser(std::string Email, std::string Username, std::string FirstName, std::string LastName, std::string Password, std::string PhoneNumber, std::string Photo)
    {
        CURL* curl;
        CURLcode res;
        SignUpAppUserResult result;
        std::string readBuffer;
        bool signedUp = false;
        DenateHTTPResponse httpResponse;
        DenateUserDetails localuserdetails;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("appuser/addappuseraccount"));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            std::string postfield = "email=" + Email + "&username=" + Username + "&first_name=" + FirstName + "&last_name=" + LastName + "&password=" + Password + "&phone_number=" + PhoneNumber + "&picture=" + Photo + "&appID=" + appID + "&userID=" + userID;
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str());

            /*std::string json = R"({"email_or_username": ")" + EmailOrIdOrUsername + R"("})";
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());*/

            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            //headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("appID: " + std::string(appID)).c_str());
            headers = curl_slist_append(headers, ("userID: " + std::string(userID)).c_str());
            headers = curl_slist_append(headers, ("appuserID: " + std::string(userDetails.appUserID)).c_str());

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
            signedUp = true;

            localuserdetails.emailOrId = jsonResponse["response"]["email"];
            localuserdetails.username = jsonResponse["response"]["username"];
            localuserdetails.firstName = jsonResponse["response"]["first_name"];
            localuserdetails.lastName = jsonResponse["response"]["last_name"];
            localuserdetails.picture = jsonResponse["response"]["picture"];
            localuserdetails.appUserID = jsonResponse["response"]["appuserID"];

        }

        result.httpResponse = httpResponse;
        result.signedUp = signedUp;
        result.userDetails = localuserdetails;

        curl_global_cleanup();

        return result;
    }

    UpdateAppUserResult DOS_AppUser::UpdateDenateAppUser(std::string appUserID, std::string Username, std::string FirstName, std::string LastName, std::string Password, std::string PhoneNumber, std::string Photo)
    {
        CURL* curl;
        CURLcode res;
        UpdateAppUserResult result;
        std::string readBuffer;
        bool updated = false;
        DenateHTTPResponse httpResponse;
        DenateUserDetails localuserdetails;
        nlohmann::json jsonResponse;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {
            
            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("appuser/updateappuserbyappuserID/") + std::string(appUserID));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            std::string postfield = "username=" + Username + "&first_name=" + FirstName + "&last_name=" + LastName + "&password=" + Password + "&phone_number=" + PhoneNumber + "&picture=" + Photo;
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str());

            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");

            /*std::string json = R"({"email_or_username": ")" + EmailOrIdOrUsername + R"("})";
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());*/

            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            //headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("appID: " + std::string(appID)).c_str());
            headers = curl_slist_append(headers, ("userID: " + std::string(userID)).c_str());
            headers = curl_slist_append(headers, ("appuserID: " + std::string(userDetails.appUserID)).c_str());

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

        jsonResponse = nlohmann::json::parse(readBuffer);
        
        httpResponse.status_code = jsonResponse["status"];
        httpResponse.message = jsonResponse["message"];

        if (jsonResponse.contains("response"))
        {
            updated = true;

            localuserdetails.emailOrId = jsonResponse["response"]["email"];
            localuserdetails.username = jsonResponse["response"]["username"];
            localuserdetails.firstName = jsonResponse["response"]["first_name"];
            localuserdetails.lastName = jsonResponse["response"]["last_name"];
            localuserdetails.picture = jsonResponse["response"]["picture"];
            localuserdetails.appUserID = jsonResponse["response"]["appuserID"];

        }

        result.httpResponse = httpResponse;
        result.updated = updated;
        result.userDetails = localuserdetails;

        curl_global_cleanup();

        return result;
    }

    AddDenateAppUserVerificationResult DOS_AppUser::AddDenateAppUserVerification(std::string Email)
    {
        CURL* curl;
        CURLcode res;
        AddDenateAppUserVerificationResult result;
        std::string readBuffer;
        bool codesent = false;
        DenateHTTPResponse httpResponse;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("appuser/addappuseremailverification"));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email=" + Email + "&verification_code=" + std::to_string(0) + "&appID=" + appID;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str());

            std::string json = R"({"email": ")" + Email + "\"" + ", " + R"("verification_code":)" + std::to_string(0) + ", " + R"("appID": ")" + appID + R"("})";
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
            
            struct curl_slist* headers = NULL;
            //headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("appID: " + std::string(appID)).c_str());
            headers = curl_slist_append(headers, ("userID: " + std::string(userID)).c_str());
            headers = curl_slist_append(headers, ("appuserID: " + std::string(userDetails.appUserID)).c_str());

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

        if (jsonResponse["status"] >= 200 && jsonResponse["status"] < 300)
        {
            codesent = true;
        }

        result.httpResponse = httpResponse;
        result.codesent = codesent;

        curl_global_cleanup();

        return result;
    }

    VerifyDenateAppUserResult DOS_AppUser::VerifyDenateAppUser(std::string Email, std::string VerificationCode)
    {
        CURL* curl;
        CURLcode res;
        VerifyDenateAppUserResult result;
        std::string readBuffer;
        bool verified = false;
        DenateHTTPResponse httpResponse;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("appuser/verifyappuseremail"));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            //std::string postfield = "email=" + Email + "&verification_code=" + VerificationCode;
            //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str());

            std::string json = R"({"email": ")" + Email + "\"" + ", " + R"("verification_code":)" + VerificationCode + R"(})";
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());

            struct curl_slist* headers = NULL;
            //headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("appID: " + std::string(appID)).c_str());
            headers = curl_slist_append(headers, ("userID: " + std::string(userID)).c_str());
            headers = curl_slist_append(headers, ("appuserID: " + std::string(userDetails.appUserID)).c_str());

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
            verified = true;
        }

        result.httpResponse = httpResponse;
        result.verified = verified;

        curl_global_cleanup();

        return result;
    }

    AddDenateAppUserVerificationResult DOS_AppUser::ForgotDenateAppUserPassword(std::string Email)
    {
        CURL* curl;
        CURLcode res;
        AddDenateAppUserVerificationResult result;
        std::string readBuffer;
        bool codesent = false;
        DenateHTTPResponse httpResponse;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("appuser/forgetappuserpassword"));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            std::string postfield = "email=" + Email;
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str());

            /*std::string json = R"({"email_or_username": ")" + EmailOrIdOrUsername + R"("})";
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());*/

            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            //headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("appID: " + std::string(appID)).c_str());
            headers = curl_slist_append(headers, ("userID: " + std::string(userID)).c_str());
            headers = curl_slist_append(headers, ("appuserID: " + std::string(userDetails.appUserID)).c_str());

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
            codesent = true;
        }

        result.httpResponse = httpResponse;
        result.codesent = codesent;

        curl_global_cleanup();

        return result;
    }

    ChangeDenateAppUserPasswordResult DOS_AppUser::ChangeDenateAppUserPassword(std::string Email, std::string OldPassword, std::string NewPassword)
    {
        CURL* curl;
        CURLcode res;
        ChangeDenateAppUserPasswordResult result;
        std::string readBuffer;
        bool passwordChanged = false;
        DenateHTTPResponse httpResponse;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            replaceSubstring(Email, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("appuser/changeappuserpasswordbyemail/") + std::string(Email));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            std::string postfield = "old_password=" + OldPassword + "&new_password=" + NewPassword;
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str());

            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");

            /*std::string json = R"({"email_or_username": ")" + EmailOrIdOrUsername + R"("})";
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());*/

            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            //headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("appID: " + std::string(appID)).c_str());
            headers = curl_slist_append(headers, ("userID: " + std::string(userID)).c_str());
            headers = curl_slist_append(headers, ("appuserID: " + std::string(userDetails.appUserID)).c_str());

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
            passwordChanged = true;
        }

        result.httpResponse = httpResponse;
        result.passwordChanged = passwordChanged;

        curl_global_cleanup();

        return result;
    }

    DoesDenateUserExistResult DOS_AppUser::DoesDenateUserExist(std::string EmailOrIdOrUsername)
    {
        CURL* curl;
        CURLcode res;
        DoesDenateUserExistResult result;
        std::string readBuffer;
        bool userExist = false;
        DenateHTTPResponse httpResponse;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            replaceSubstring(EmailOrIdOrUsername, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("appuser/doesplayerexist/") + std::string(EmailOrIdOrUsername));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            //headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("appID: " + std::string(appID)).c_str());
            headers = curl_slist_append(headers, ("userID: " + std::string(userID)).c_str());
            headers = curl_slist_append(headers, ("appuserID: " + std::string(userDetails.appUserID)).c_str());

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
            userExist = true;
        }

        result.httpResponse = httpResponse;
        result.userExist = userExist;

        curl_global_cleanup();

        return result;
    }

    GetPlayerDetailsResult DOS_AppUser::GetPlayerDetails(std::string EmailOrIdOrUsername)
    {
        CURL* curl;
        CURLcode res;
        GetPlayerDetailsResult result;
        std::string readBuffer;
        bool gottenDetails = false;
        DenateHTTPResponse httpResponse;
        DenateUserDetails localuserdetails;

        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {

            replaceSubstring(EmailOrIdOrUsername, " ", "%20");

            std::string url = (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("appuser/getuserdetails/") + std::string(EmailOrIdOrUsername));
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            //headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("appID: " + std::string(appID)).c_str());
            headers = curl_slist_append(headers, ("userID: " + std::string(userID)).c_str());
            headers = curl_slist_append(headers, ("appuserID: " + std::string(userDetails.appUserID)).c_str());

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
            gottenDetails = true;

            localuserdetails.emailOrId = jsonResponse["response"]["email"];
            localuserdetails.username = jsonResponse["response"]["username"];
            localuserdetails.firstName = jsonResponse["response"]["first_name"];
            localuserdetails.lastName = jsonResponse["response"]["last_name"];
            localuserdetails.picture = jsonResponse["response"]["picture"];
            localuserdetails.appUserID = jsonResponse["response"]["appuserID"];

        }

        result.httpResponse = httpResponse;
        result.gottenDetails = gottenDetails;
        result.userDetails = localuserdetails;

        curl_global_cleanup();

        return result;
    }

}