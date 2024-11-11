#include "DOS_Achievement.h"
#include "curl/curl.h"
#include "DenateTypes.h"
#include <stdio.h>
#include <iostream>
#include <cstring>

using namespace DenateAchievement;

DOS_Achievement::DOS_Achievement(std::string userID, std::string appID)
{
    this->userID = userID;
    this->appID = appID;
}
    
double DOS_Achievement::Add(double a, double b)
{
    ///*
    CURL* curl;
    CURLcode res;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
        
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, (DenateTypes::DOS_DenateTypes::getdenateapiURL().c_str() + std::string("achievement/getallgameachievements")));
        curl_easy_setopt(curl, CURLOPT_HTTPGET);
        //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "name=John&age=30");

        struct curl_slist* headers = NULL;
        //headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, (std::string("appID: ") + std::string(appID)).c_str() );
        headers = curl_slist_append(headers, (std::string("userID: ") + std::string(userID)).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            fprintf(stderr, "Request Failed . Curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
     
    return 0.1;
}

double DOS_Achievement::Subtract(double a, double b)
{
    return a - b;
}

double DOS_Achievement::Multiply(double a, double b)
{
    return a * b;
}

double DOS_Achievement::Divide(double a, double b)
{
    return a / b;
}
