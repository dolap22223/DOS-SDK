// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <DOS_Achievement.h>
#include <DenateTypes.h>
#include <DOS_Online_Save.h>
#include <DOS_AppUser.h>
#include <DOS_Friends.h>
#include <DOS_Leaderboard.h>
#include <DOS_Voice_Chat.h>
#include <DOS_Local_Match.h>
#include <curl/curl.h>

int main()
{
    std::cout << "Hello World!\n";

    CURL* curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    DenateAppUser::DOS_AppUser appuser("userID", "appID", false);
    
    //DenateAchievement::DOS_Achievement denatea("", "");
    std::cout << "Hello World!\n";

    LoginAppUserResult localone;
    localone = appuser.LoginDenateAppUserById("dpp");

    DenateLocalMatch::DOS_Local_Match denatefriend("userID", "appID", false, localone.token, localone.userDetails);

    //DenateFilterResult value1 = { "ebe", "value1" }; DenateFilterResult value2 = { "dgbd", "value2" }; DenateFilterResult value3 = { "dbdb","fsfb" };
    //std::vector<DenateFilterResult> myvector = { value1, value2, value3 };
    /*std::vector<DenateFilterResult> localresult;
    std::string localfilter = "dss=wrrw,iiwr=wwrpppp,mmmm=iiiii";
    std::string onlinesaveresult;
    localresult = onlinesave.BreakDenateFilter(localfilter);*/

    HostDenatePrivateLocalMatchResult createdmatch;

    createdmatch = denatefriend.HostDenatePrivateLocalMatch("ssfv", "svs=sdds,wicd=wwcs", 44, "rrvbb");

    CreateTeamResult hostedteam;

    hostedteam = denatefriend.CreateTeam("svs=sdds,wicd=wwcs");

    JoinTeamResult localfriend;
    
    localfriend = denatefriend.JoinTeam(hostedteam.teamID);
    
    InviteFriendResult leftteam;

    leftteam = denatefriend.InviteFriend("dpp");

    std::cout << leftteam.httpResponse.message << std::endl;

    //std::cout << DenateAchievement::DOS_Achievement::Subtract(5.2, 3.3) << std::endl;

    //std::cout << DenateAchievement::DOS_Achievement::Add(5.2, 3.3) << std::endl;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
