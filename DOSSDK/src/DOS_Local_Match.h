#pragma once

#include <stdio.h>
#include <string>
#include <curl/curl.h>
#include <json.hpp>
#include <vector>
#include <sio_client.h>
#include "DenateTypes.h"
#include "DOS_Connection.h"

struct HostDenateLocalMatchResult
{
    DenateHTTPResponse httpResponse;
    bool matchCreated;
    DenateMatchDetails matchDetails;
};

struct UpdateDenateLocalMatchResult
{
    DenateHTTPResponse httpResponse;
    bool matchUpdated;
    DenateMatchDetails matchDetails;
};

struct UpdateDenatePrivateLocalMatchResult
{
    DenateHTTPResponse httpResponse;
    bool matchUpdated;
    DenatePrivateMatchDetails privateMatchDetails;
};

struct HostDenatePrivateLocalMatchResult
{
    DenateHTTPResponse httpResponse;
    bool matchCreated;
    DenatePrivateMatchDetails privateMatchDetails;
};

struct JoinDenateLocalMatchResult
{
    DenateHTTPResponse httpResponse;
    bool matchJoined;
    DenateMatchDetails matchDetails;
};

struct JoinDenatePrivateLocalMatchResult
{
    DenateHTTPResponse httpResponse;
    bool matchJoined;
    DenatePrivateMatchDetails matchDetails;
};

struct FindDenateLocalMatchResult
{
    DenateHTTPResponse httpResponse;
    bool matchFound;
    DenateMatchDetails matchDetails;
};

struct FindDenateLocalMatchesResult
{
    DenateHTTPResponse httpResponse;
    bool matchFound;
    std::vector<DenateMatchDetails> matchDetails;
};

struct CreateTeamResult
{
    DenateHTTPResponse httpResponse;
    bool teamCreated;
    std::string teamID;
};

struct JoinTeamResult
{
    DenateHTTPResponse httpResponse;
    bool teamJoined;
    DenateTeamDetails teamDetails;
};

struct LeaveTeamResult
{
    DenateHTTPResponse httpResponse;
    bool leftTeam;
};

struct DestroyTeamResult
{
    DenateHTTPResponse httpResponse;
    bool teamDestroyed;
};

struct GetPlayersInTeamResult
{
    DenateHTTPResponse httpResponse;
    bool gottenPlayers;
    std::vector<DenateTeamPlayersDetails> teamPlayers;
};

struct GetTeamsInMatchResult
{
    DenateHTTPResponse httpResponse;
    bool gottenTeams;
    std::vector<DenateTeamMatchDetail> teamsDetails;
};

struct GetPlayersTeamResult
{
    DenateHTTPResponse httpResponse;
    bool gottenTeam;
    DenateTeamMatchDetail teamDetail;
};

struct ReportPlayerResult
{
    DenateHTTPResponse httpResponse;
    bool playerReported;
};

struct InviteFriendResult
{
    DenateHTTPResponse httpResponse;
    bool inviteSent;
};

namespace DenateLocalMatch
{

	class DOS_Local_Match
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

        /** Denate connection */
        DenateConnection::DOS_Connection& internalDenateConnection;

        /** Replaces a substring with another*/
        void replaceSubstring(std::string& original, const std::string& toReplace, const std::string& replacement);

    public:

        /** Constructor */
        DOS_Local_Match(std::string userID, std::string appID, bool dedicatedServer, std::string token, DenateUserDetails userDetails, DenateConnection::DOS_Connection& denateConnection);

        /** True if the current instance is part of a private match*/
        bool isPrivateMatch = false;

        /** Tue if the current instance hosted the match*/
        bool isServer = false;

        /** Details of the match the intance is a part of */
        DenateMatchDetails currentMatchDetail;

        /** Details of the match the intance is a part of provided they are in a private match */
        DenatePrivateMatchDetails currentPrivateMatchDetail;

        /** Details of the team the instance is a part of, provided they've joined a team */
        DenateTeamDetails currentTeamDetail;

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

        /** Hosts a Denate local match. This node hosts a match locally and also hosts a match on the denate API. Server name, Player name and filters are OPTIONAL field. Map name and max players are COMPULSORY
        Map name specifies the level to be open, server name is optional but is required if clients are joining by server
        name, player name specifies the name of the player hosting the game filters control extra information about the hosted match,
        max players specifies the maximum amount of players that can be allowed to join. Note when using the filter, it should contain
        a title and a value and in a format such like {title}={value},{title}={value}. An example is: (city=NY,country=USA,match type=battle royale) and whitepace should only be used
        within the title name or value
        * @param map_name the map name of the private match
        * @param server_name the server name of the private match
        * @param player_name the name of the host provided its not a dedicated server match (OPTIONAL)
        * @param Filters used to specify extra info about the leaderboard for example (score=50,name=john). Its recommended you use the make filter function to make the filter(OPTIONAL)
        * @param max_players the maximum amount of players allowed to join the match
        * @return HostDenateLocalMatchResult
        */
        HostDenateLocalMatchResult HostDenateLocalMatch(std::string mapName, std::string filters, int maxPlayers, std::string serverName, std::string playerName = "");

        /** Hosts a Denate local private match. Server name, Player name and filters are OPTIONAL field. Map name and max players are COMPULSORY
        Map name specifies the level to be open, server name is optional but is required if clients are joining by server
        name, player name specifies the name of the player hosting the game filters control extra information about the hosted match,
        max players specifies the maximum amount of players that can be allowed to join. Note when using the filter, it should contain
        a title and a value and in a format such like {title}={value},{title}={value}. An example is: (city=NY,country=USA,match type=battle royale) and whitepace should only be used
        within the title name or value.
        * @param map_name the map name of the private match
        * @param server_name the server name of the private match
        * @param player_name the name of the host provided its not a dedicated server match (OPTIONAL)
        * @param Filters used to specify extra info about the leaderboard for example (score=50,name=john). Its recommended you use the make filter function (OPTIONAL)
        * @param max_players the maximum amount of players allowed to join the match
        * @return HostDenatePrivateLocalMatchResult
        */
        HostDenatePrivateLocalMatchResult HostDenatePrivateLocalMatch(std::string mapName, std::string filters, int maxPlayers, std::string serverName, std::string playerName = "");

        /** Joins one available match by server name. Player name is an OPTIONAL field
        * @param server_name name of the server to join its match
        * @param player_name the name(Denate Username) of the user joining a match, leave empty to specify yourself (OPTIONAL)
        * @return JoinDenateLocalMatchResult
        */
        JoinDenateLocalMatchResult JoinDenateLocalMatchByServerName(std::string serverName, std::string playerName = "");

        /** Joins one available match by a filters. Player name is an OPTIONAL field
        * @param filter used to specify extra info about the leaderboard for example (score=50,name=john). Its recommended you use the make filter function to make the filter.
        * @param player_name the name(Denate Username) of the user joining a match, leave empty to specify yourself (OPTIONAL)
        * @return JoinDenateLocalMatchResult
        */
        JoinDenateLocalMatchResult JoinDenateLocalMatchByPlayerName(std::string playerToFind, std::string playerName = "");

        /** Joins one available match by a filters. Player name is an OPTIONAL field
        * @param filter used to specify extra info about the leaderboard for example (score=50,name=john). Its recommended you use the make filter function to make the filter.
        * @param player_name the name(Denate Username) of the user joining a match, leave empty to specify yourself (OPTIONAL)
        * @return JoinDenateLocalMatchResult
        */
        JoinDenateLocalMatchResult JoinDenateLocalMatchByFilter(std::string filters, std::string playerName = "");

        /** Joins one random available match. Player name is an OPTIONAL field
        * @param player_name the name(Denate Username) of the user joining a match, leave empty to specify yourself (OPTIONAL)
        * @return JoinDenateLocalMatchResult
        */
        JoinDenateLocalMatchResult JoinDenateLocalMatch(std::string playerName = "");

        /** Joins a Denate private match. Returns the details of the match being joined. Player name is an OPTIONAL field
        * @param player_name the name(Denate Username) of the user joining a match
        * @return JoinDenateLocalMatchResult
        */
        JoinDenatePrivateLocalMatchResult JoinDenatePrivateMatch(std::string gameCode, std::string playerName = "");

        /** Finds one available match. OnFindDenateMatchProcessed delegates returns the result
        * @return FindDenateLocalMatchResult
        */
        FindDenateLocalMatchResult FindDenateLocalMatch();

        /** Finds all available matches which includes servers that are full.
        * @return FindDenateLocalMatchesResult
        */
        FindDenateLocalMatchesResult FindDenateLocalMatches();

        /** Finds one available match by server name.
        * @param server_name the name of the server to find
        * @return FindDenateLocalMatchResult
        */
        FindDenateLocalMatchResult FindDenateLocalMatchByServerName(std::string serverName);

        /** Finds one available match by player name. 
        * @param player_name the name(Denate Username) of the player's match to find
        * @return FindDenateLocalMatchResult
        */
        FindDenateLocalMatchResult FindDenateLocalMatchByPlayerName(std::string playerName);

        /** Finds one available match by map name. 
        * @param map_name name of the map to find.
        * @return FindDenateLocalMatchResult
        */
        FindDenateLocalMatchResult FindDenateLocalMatchByMapName(std::string mapName);

        /** Finds one available match by filters. Note that more than one filter can be passed and it should contain
        a title and a value and in a format such like {title}={value},{title}={value}. An example is:
        (city=NY,country=USA,match type=battle royale) and whitepace should only be used
        within the title name or value . OnFindDenateLocalMatchByFilter delegates returns the result
        * @param filters used to specify extra info about the leaderboard for example (score=50,name=john). Its recommended you use the make filter function to make the filter.
        * @return FindDenateLocalMatchResult
        */
        FindDenateLocalMatchResult FindDenateLocalMatchByFilter(std::string filters);

        /** Finds all available match by filters. Note that more than one filter can be passed and it should contain
        a title and a value and in a format such like {title}={value},{title}={value}. An example is:
        (city=NY,country=USA,match type=battle royale) and whitepace should only be used
        within the title name or value . OnFindDenateLocalMatchesByFilter delegates returns the result
        * @param filters used to specify extra info about the leaderboard for example (score=50,name=john). Its recommended you use the make filter function to make the filter.
        * @return FindDenateLocalMatchesResult
        */
        FindDenateLocalMatchesResult FindDenateLocalMatchesByFilter(std::string filters);

        /** Updates a Denate local match. Only the specified field will be updated, the rest should be left as an empty string
        Map name specifies the level to be open, server name is optional but is required if clients are joining by server
        name, player name specifies the name of the player hosting the game filters control extra information about the hosted match,
        max players specifies the maximum amount of players that can be allowed to join. Note when using the filter, it should contain
        a title and a value and in a format such like {title}={value},{title}={value}. An example is: (city=NY,country=USA,match type=battle royale) and whitepace should only be used
        within the title name or value.
        * @param map_name the new map name of the current match (OPTIONAL)
        * @param server_name the new server name of the current match (OPTIONAL)
        * @param player_name the name of the new host provided its not a dedicated server match (OPTIONAL)
        * @param Filters used to specify extra info about the match for example (score=50,name=john). Its recommended you use the make filter function (OPTIONAL)
        * @param max_players the maximum amount of players allowed to join the match (OPTIONAL)
        * @return UpdateDenateLocalMatchResult
        */
        UpdateDenateLocalMatchResult UpdateDenateLocalMatch(std::string mapName = "", std::string filters = "", int maxPlayers = 0, std::string serverName = "", std::string playerName = "");

        /** Updates a Denate local private match. Only the specified field will be updated, the rest should be left as an empty string
        Map name specifies the level to be open, server name represents the name you would like to update the match to
        name, player name specifies the name of the player hosting the game filters control extra information about the hosted match,
        max players specifies the maximum amount of players that can be allowed to join. Note when using the filter, it should contain
        a title and a value and in a format such like {title}={value},{title}={value}. An example is: (city=NY,country=USA,match type=battle royale) and whitepace should only be used
        within the title name or value. Its recommended to use the make filter function when passing in filters.
        * @param map_name the new map name of the current match (OPTIONAL)
        * @param server_name the new server name of the current match (OPTIONAL)
        * @param player_name the name of the new host provided its not a dedicated server match (OPTIONAL)
        * @param Filters used to specify extra info about the match for example (score=50,name=john). Its recommended you use the make filter function (OPTIONAL)
        * @param max_players the maximum amount of players allowed to join the match (OPTIONAL)
        * @return UpdateDenatePrivateLocalMatchResult
        */
        UpdateDenatePrivateLocalMatchResult UpdateDenateLocalPrivateMatch(std::string mapName = "", std::string filters = "", int maxPlayers = 0, std::string serverName = "", std::string playerName = "");

        /** Creates a Team which players can join
        * @param Filters used to specify extra info about the leaderboard for example (score=50,name=john). Its recommended you use the make filter function
        * @return CreateTeamResult
        */
        CreateTeamResult CreateTeam(std::string filters);

        /** joins a Team
        * @param TeamID Unique ID of the team you would like to join
        * @return JoinTeamResult
        */
        JoinTeamResult JoinTeam(std::string teamID);

        /** Leave a Team. If teamID is not specified, the current instance will leave the team they are currently a part of
        * @param TeamID Unique ID of the team you would like to leave (OPTIONAL)
        * @return LeaveTeamResult
        */
        LeaveTeamResult LeaveTeam(std::string teamID = "");

        /** Destroy a Team.
        * @param TeamID Unique ID of the team you would like to destroy
        * @return DestroyTeamResult
        */
        DestroyTeamResult DestroyTeam(std::string teamID);

        /** gets all players within a team. If you wish to get the players in your current team, leave TeamID empty
        * @param TeamID Unique ID of the team you would like to get all the players within it
        * @return GetPlayersInTeamResult
        */
        GetPlayersInTeamResult GetPlayersInTeam(std::string teamID);

        /** gets all teams within a match
        * @return GetTeamsInMatchResult
        */
        GetTeamsInMatchResult GetTeamsInMatch();

        /** gets a players team. If you wish to get the team of the current player, leave player name empty
        * @param player_name name of the player you would like to get all the channels they are connected to, leave empty to specify yourself (OPTIONAL)
        * @return GetPlayersTeamResult
        */
        GetPlayersTeamResult GetPlayersTeam(std::string playerName = "");

        /** Reports a player
        * @param name_of_player_to_report name(Denate Username) of the player to report
        * @return ReportPlayerResult
        */
        ReportPlayerResult ReportPlayer(std::string nameOfPlayerToReport, std::string reason = "");

        /** Invites a friend to a match
        * @param friend_name name(Denate Username) of the friend to invite
        * @return InviteFriendResult
        */
        InviteFriendResult InviteFriend(std::string friendName);

        /** Namespace socket for the denate voice chat */
        sio::socket::ptr namespaceSocket;

	};

}

