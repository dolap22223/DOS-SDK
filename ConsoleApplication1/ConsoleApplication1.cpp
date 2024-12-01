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
#include <DOS_Connection.h>
#include <thread>
//#include <curl/curl.h>

int main()
{
    std::cout << "Hello World!\n";

    DenateAppUser::DOS_AppUser appuser("userID", "appID", false);
    
    //DenateAchievement::DOS_Achievement denatea("", "");
    std::cout << "Hello World!\n";

    LoginAppUserResult localone;
    localone = appuser.LoginDenateAppUserById("dpp");

    

    //DenateFilterResult value1 = { "ebe", "value1" }; DenateFilterResult value2 = { "dgbd", "value2" }; DenateFilterResult value3 = { "dbdb","fsfb" };
    //std::vector<DenateFilterResult> myvector = { value1, value2, value3 };
    /*std::vector<DenateFilterResult> localresult;
    std::string localfilter = "dss=wrrw,iiwr=wwrpppp,mmmm=iiiii";
    std::string onlinesaveresult;
    localresult = onlinesave.BreakDenateFilter(localfilter);*/

    /*HostDenatePrivateLocalMatchResult createdmatch;

    createdmatch = denatefriend.HostDenatePrivateLocalMatch("ssfv", "svs=sdds,wicd=wwcs", 44, "rrvbb");

    CreateTeamResult hostedteam;

    hostedteam = denatefriend.CreateTeam("svs=sdds,wicd=wwcs");

    JoinTeamResult localfriend;
    
    localfriend = denatefriend.JoinTeam(hostedteam.teamID);
    
    InviteFriendResult leftteam;

    leftteam = denatefriend.InviteFriend("dpp");

    std::cout << leftteam.httpResponse.message << std::endl;*/

    DenateConnection::DOS_Connection denateconnection("userID", "appID", false, localone.token, localone.userDetails);
    denateconnection.EstablishDenateConnection(false);

    std::this_thread::sleep_for(std::chrono::seconds(3));
    //bool joined = false;

    //denateconnection.OnDenateOnlineServiceConnected([&]() { std::cout << "sfmfsbkmmmmmmmmmmmmmkfssfbsfbsfbsb" << std::endl; });

    //denateconnection.EstablishDenateConnection(false);

    DenateLocalMatch::DOS_Local_Match denatefriend("userID", "appID", false, localone.token, localone.userDetails, denateconnection);



    /*DenateVoiceChat::DOS_Voice_Chat voicechat("userID", "appID", false, localone.token, localone.userDetails, denateconnection);
    voicechat.Activate();
    std::string sesid = denateconnection.sioClient.get_sessionid();

    CreateVoiceChannelResult createvcresult;
    createvcresult = voicechat.CreateVoiceChannel();

    JoinVoiceChannelResult joinvcresult;
    joinvcresult = voicechat.JoinVoiceChannel(createvcresult.channelID);

    std::this_thread::sleep_for(std::chrono::seconds(3));
    voicechat.sendAudioData({ 2,3,24,4 }, { "gwwgwggr", voicechat.allConnectedChannels[0].clientId});*/

    HostDenateLocalMatchResult createdmatch;

    createdmatch = denatefriend.HostDenateLocalMatch("srrrrrrrrrsfv", "svs=sdds,wicd=wwcs", 44, "rqqqqqqqb");

    std::this_thread::sleep_for(std::chrono::seconds(3));

    //std::string filter = denatefriend.MakeDenateFilter({ {"svsvw", "eeeee"}, {"rrrrf", "iiiiii"}, { "mmmmm", "uuuu"}});

    //std::cout << filter << std::endl;

    //std::vector<DenateFilterResult> filterresult = denatefriend.BreakDenateFilter(filter);

    //std::cout << filterresult[0].value + ": " + filterresult[0].title << std::endl;

    std::cout << createdmatch.matchDetails.ipAddress << std::endl;

    //if (joined)
    //{
    //    std::cout << "True" << std::endl;
    //}
    //else {
    //    std::cout << "False" << std::endl;
    //}

    std::this_thread::sleep_for(std::chrono::seconds(10));

    bool yooo = denatefriend.EndDenateMatch();

    if (yooo)
    {
        std::cout << " yooooo is True" << std::endl;
    }
    else {
        std::cout << " yoooo is False" << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::seconds(30));

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
