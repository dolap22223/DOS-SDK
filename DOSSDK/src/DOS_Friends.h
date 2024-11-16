#pragma once

#include <stdio.h>
#include <string>
#include <curl/curl.h>
#include <json.hpp>
#include <vector>
#include "DenateTypes.h"

struct AddFriendResult
{
    DenateHTTPResponse httpResponse;
    bool friendRequestSent;
    
};

struct GetFriendRequestsResult
{
    DenateHTTPResponse httpResponse;
    bool gottenFriendRequest;
    std::vector< DenateFriendDetails> friendDetails;
};

struct GetFriendsResult
{
    DenateHTTPResponse httpResponse;
    bool foundFriends;
    std::vector< DenateFriendDetails> friendDetails;
};

struct DeclineFriendRequestResult
{
    DenateHTTPResponse httpResponse;
    bool friendRequestDeclined;
};

struct UnfriendPlayerResult
{
    DenateHTTPResponse httpResponse;
    bool unfriended;
};

struct AcceptFriendRequestResult
{
    DenateHTTPResponse httpResponse;
    bool accepted;
};

struct SearchPlayerResult
{
    DenateHTTPResponse httpResponse;
    bool found;
    std::vector<DenateSearchUserDetails> searchUserDetails;
};

struct CheckPlayerOnlineStatusResult
{
    DenateHTTPResponse httpResponse;
    bool requestSuccessful;
    bool inGame;
    bool online;
};

struct AcceptInviteResult
{
    DenateHTTPResponse httpResponse;
    bool inviteAccepted;
    DenateMatchDetails matchDetails;
};

struct DeclineInviteResult
{
    DenateHTTPResponse httpResponse;
    bool inviteDeclined;
};

struct GetInvitesResultResult
{
    DenateHTTPResponse httpResponse;
    bool gottenInvites;
    std::vector<DenateFriendDetails> inviteDetails;
};

namespace DenateFriends
{

	class DOS_Friends
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
        DOS_Friends(std::string userID, std::string appID, bool dedicatedServer, std::string token, DenateUserDetails userDetails);

        /** Sends a friend request to a player
        * @param friend_name Name of friend to add
        * @return AddFriendResult
        */
        AddFriendResult AddFriend(std::string friendName);

        /** Gets all friend request
        * @param player_name Name of player to get their friend requests, leave empty to specify yourself (OPTIONAL)
        * @return GetFriendRequestsResult
        */
        GetFriendRequestsResult GetFriendRequests(std::string playerName = "");

        /** Gets all friends
        * @param player_name Name of player to get their friends, leave empty to specify yourself (OPTIONAL)
        * @return GetFriendsResult
        */
        GetFriendsResult GetFriends(std::string playerName = "");

        /** Declines a friends request
        * @param friend_name Name of player the player would like to deny their friend request
        * @return DeclineFriendRequestResult
        */
        DeclineFriendRequestResult DeclineFriendRequest(std::string friendName);

        /** Unfriends a player
        * @param friend_name Name of player you would like to unfriend
        * @return UnfriendPlayerResult
        */
        UnfriendPlayerResult UnfriendPlayer(std::string friendName);

        /** Accepts friend request provided they've sent you a friend request
        * @param friend_name Name of player you would like to accept their friend request, provided they've sent you a friend request
        * @return AcceptFriendRequestResult
        */
        AcceptFriendRequestResult AcceptFriendRequest(std::string friendName);

        /** Searches for a user
        * @param Query Query for a players name/Username
        * @return SearchPlayerResult
        */
        SearchPlayerResult SearchPlayer(std::string query);

        /** checks a player online status
        * @param player_name Name of player you would like to get their online status
        * @return CheckPlayerOnlineStatusResult
        */
        CheckPlayerOnlineStatusResult CheckPlayerOnlineStatus(std::string playerName);

        /** Accepts a friends invite provided they've invited you to a match
        * @param friend_name Name of player/friend you would like to accept their invite, provided they've invited you to a match
        * @return AcceptInviteResult
        */
        AcceptInviteResult AcceptInvite(std::string friendName);

        /** Declines a friends invite
        * @param friend_name Name of player/friend you would like to decline their invite, provided they've invited you to a match
        * @return DeclineInviteResult
        */
        DeclineInviteResult DeclineInvite(std::string friendName);

        /** Gets all friends invite
        * @return GetInvitesResultResult
        */
        GetInvitesResultResult GetInvites();

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

