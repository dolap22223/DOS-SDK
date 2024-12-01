#pragma once

#include <stdio.h>
#include <string>
#include <curl/curl.h>
#include "json.hpp"
#include <vector>
#include "DenateTypes.h"

struct AddPlayerLeaderboardResult
{
    DenateHTTPResponse httpResponse;
    bool leaderboardAdded;
};

struct GetPlayerLeaderboardResult
{
    DenateHTTPResponse httpResponse;
    bool gottenLeaderboard;
    std::string leaderboardFilters;
};

struct GetAllLeaderboardResult
{
    DenateHTTPResponse httpResponse;
    bool gottenLeaderboard;
    std::vector<DenateLeaderboardResult> leaderboardFilters;
};

struct GetAllPlayerLeaderboardResult
{
    DenateHTTPResponse httpResponse;
    bool gottenLeaderboard;
    std::vector<DenateLeaderboardResult> leaderboardFilters;
};


namespace DenateLeaderboard
{
	class DOS_Leaderboard
	{

    private:
        /** userID credential */
        std::string userID;

        /** appID credential */
        std::string appID;

        /** True if the current instance is a dedicated server */
        bool dedicatedServer;

        /** Generated User token after logging in */
        std::string token;

        /** Details of the user after logging in */
        DenateUserDetails userDetails;

        /** Replaces a substring with another*/
        void replaceSubstring(std::string& original, const std::string& toReplace, const std::string& replacement);

    public:

        /** Constructor */
        DOS_Leaderboard(std::string userID, std::string appID, bool dedicatedServer, std::string token, DenateUserDetails userDetails);
        
        /** create or update leaderboard
        * @param LeaderboardName Name of leaderboard you would like to add the player into
        * @param filters used to specify extra info about the leaderboard for example (score=50,name=john)
        * @return AddPlayerLeaderboardResult
        */
        AddPlayerLeaderboardResult AddPlayerLeaderboard(std::string leaderboardName, std::string filters);

        /** Gets a players leaderboard
        * @param Leaderboard_Name Name of leaderboard you would like to get to access the info
        * @param PlayerName name of the player you would like to get their leaderboard, leave empty to specify yourself (OPTIONAL)
        * @return GetPlayerLeaderboardResult
        */
        GetPlayerLeaderboardResult GetPlayerLeaderboard(std::string leaderboardName, std::string playerName = "");

        /** Gets all player leaderboard associated with a leaderboard
        * @param Leaderboard_Name Name of leaderboard you would like to get all players that are associated with it
        * @return GetAllLeaderboardResult
        */
        GetAllLeaderboardResult GetAllLeaderboard(std::string leaderboardName);

        /** Gets all leaderboard and the player details associated with it
        * @param PlayerName Name of player you would like to get all leaderboards associated with them, leave empty to specify the currently logged in player
        * @return GetAllPlayerLeaderboardResult
        */
        GetAllPlayerLeaderboardResult GetAllPlayerLeaderboard(std::string playerName);

        /** Breaksdown a filter into its title and value
        * @param filter filters you would like to break
        * @return DenateFilterResult
        */
        std::vector<DenateFilterResult> BreakDenateFilter(std::string Filter);

        /** Makes a filter from its title and value
        * @param filter Array of struct FilterResult
        * @return Denate filter you made
        */
        std::string MakeDenateFilter(std::vector<DenateFilterResult> filters);

	};
}

