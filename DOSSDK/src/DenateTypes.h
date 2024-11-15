#pragma once
#include <string>
#include <curl/curl.h>
#include <vector>



enum class DenateRequestResult: UINT8 
{
	RequestSuccessful,
	RequestFailed
};

enum class EServerHostResult : UINT8
{
	HostingSuccessful ,
	HostingFailed
};

	
typedef	enum class EFindMatchResult : UINT8
{
	MatchFound ,
	MatchNotFound
};

struct DenateLeaderboardResult
{
	DenateLeaderboardResult(std::string playerName = "", std::string filters = "", std::string LeaderboardName = "")
	{
		this->playerName = playerName;
		this->filters = filters;
		this->LeaderboardName = LeaderboardName;
	}

public:

	std::string playerName;

	std::string filters;

	std::string LeaderboardName;
};

typedef	struct DenateFilterResult
{

	DenateFilterResult(std::string title = "", std::string value = "")
	{
		this->title = title;
		this->value = value;
	}
public:

	std::string title;

	std::string value;
};

typedef	struct DenateFriendDetails
{

	DenateFriendDetails(std::string playerName = "", std::string picture = "", bool online = false, bool inGame = false, std::string appUserID = "")
	{
		this->playerName = playerName;
		this->picture = picture;
		this->online = online;
		this->inGame = inGame;
		this->appUserID = appUserID;
	}

public:
	
	std::string playerName;

	std::string picture;

	bool online;

	bool inGame;

	std::string appUserID;
};

	
typedef	struct DenatePlayerAchievementDetails
{

	DenatePlayerAchievementDetails(std::string achievementName = "", std::string achievementDescription = "", std::string lockedAchievementName = "", std::string lockedAchievementDescription = "", std::string unlockedAchievementName = "", std::string unlockedAchievementDescription = "", std::string achievementID = "", std::string unlockedAchievementImageURL = "", std::string lockedAchievementImageURL = "", bool unlocked = false)
	{
		this->achievementName = achievementName;
		this->achievementDescription = achievementDescription;
		this->lockedAchievementName = lockedAchievementName;
		this->lockedAchievementDescription = lockedAchievementDescription;
		this->unlockedAchievementName = unlockedAchievementName;
		this->unlockedAchievementDescription = unlockedAchievementDescription;
		this->achievementID = achievementID;
		this->unlockedAchievementImageURL = unlockedAchievementImageURL;
		this->lockedAchievementImageURL = lockedAchievementImageURL;
		this->unlocked = unlocked;
	}

public:
		
	std::string achievementName;

		
	std::string achievementDescription;

		
	std::string lockedAchievementName;

		
	std::string lockedAchievementDescription;

		
	std::string unlockedAchievementName;

		
	std::string unlockedAchievementDescription;

		
	std::string achievementID;

		
	std::string lockedAchievementImageURL;

		
	std::string unlockedAchievementImageURL;

		
	bool unlocked;

};

	
typedef	struct DenateAchievementDetails
{

	DenateAchievementDetails(std::string achievementName = "", std::string achievementDescription = "", std::string lockedAchievementName = "", std::string lockedAchievementDescription = "", std::string unlockedAchievementName = "", std::string unlockedAchievementDescription = "", std::string achievementID = "", std::string unlockedAchievementImageURL = "", std::string lockedAchievementImageURL = "", bool unlocked = false)
	{
		this->achievementName = achievementName;
		this->achievementDescription = achievementDescription;
		this->lockedAchievementName = lockedAchievementName;
		this->lockedAchievementDescription = lockedAchievementDescription;
		this->unlockedAchievementName = unlockedAchievementName;
		this->unlockedAchievementDescription = unlockedAchievementDescription;
		this->achievementID = achievementID;
		this->lockedAchievementImageURL = lockedAchievementImageURL;
		this->unlockedAchievementImageURL = unlockedAchievementImageURL;
	}

public:
		
	std::string achievementName;

		
	std::string achievementDescription;

		
	std::string lockedAchievementName;

		
	std::string lockedAchievementDescription;

		
	std::string unlockedAchievementName;

		
	std::string unlockedAchievementDescription;

		
	std::string achievementID;

		
	std::string lockedAchievementImageURL;

		
	std::string unlockedAchievementImageURL;

};

	
typedef	struct DenateRoomDetails
{

	DenateRoomDetails(std::string roomId = "", std::string clientId = "")
	{
		this->roomId = roomId;
		this->clientId = clientId;
	}

public:
		
	std::string roomId;

	std::string clientId;


};

	
typedef	struct DenatePlayerVoiceInfo
{

public:

		
	std::string playerName;

		
	bool microphoneDisabled;

		
	bool muted;

	DenatePlayerVoiceInfo(bool microphoneDisabled = false, bool muted = false, std::string playerName = "")
	{
		this->microphoneDisabled = microphoneDisabled;
		this->muted = muted;
		this->playerName = playerName;
	}

	bool operator==(const DenatePlayerVoiceInfo& Other)
	{
		return microphoneDisabled == Other.microphoneDisabled && muted == Other.muted && playerName == Other.playerName;
	}

	friend bool operator==(const DenatePlayerVoiceInfo& A, const DenatePlayerVoiceInfo& B)
	{
		return A.microphoneDisabled == B.microphoneDisabled && A.muted == B.muted && A.playerName == B.playerName;
	}


};


	
typedef	struct DenateVoiceChannelDetails
{

public:
		
	std::string channelId;

		
	std::string clientId;

		
	std::string playerName;


	DenateVoiceChannelDetails(std::string channelId = "", std::string clientId = "", std::string playerName = "" )
	{
		this->channelId = channelId;
		this->clientId = clientId;
		this->playerName = playerName;
	}

	bool operator==(const DenateVoiceChannelDetails& Other)
	{
		return channelId == Other.channelId && clientId == Other.clientId && playerName == Other.playerName;
	}

	friend bool operator==(const DenateVoiceChannelDetails& A, const DenateVoiceChannelDetails& B)
	{
		return A.channelId == B.channelId && A.clientId == B.clientId && A.playerName == B.playerName;
	}

};


	
typedef	struct DenateSearchUserDetails
{

	DenateSearchUserDetails(std::string playerName = "", std::string picture = "", std::string appUserID = "", bool online = false, bool inGame = false, bool friendRequestSent = false, bool friendRequestRecieved = false, bool is_a_Friend = false)
	{
		this->playerName = playerName;
		this->picture = picture;
		this->appUserID = appUserID;
		this->online = online;
		this->inGame = inGame;
		this->friendRequestSent = friendRequestSent;
		this->friendRequestRecieved = friendRequestRecieved;
		this->is_a_Friend = is_a_Friend;
	}

public:
		
	std::string playerName;

		
	std::string picture;

		
	std::string appUserID;

		
	bool online;

		
	bool inGame;

		
	bool friendRequestSent;

		
	bool friendRequestRecieved;

		
	bool is_a_Friend;
};

	
typedef	struct DenateHTTPResponse
{
		
	DenateHTTPResponse(int status_code = 0, std::string message = "", bool requestSuccessful = false)
	{
		this->status_code = status_code;
		this->message = message;
		this->requestSuccessful = requestSuccessful;
	}

public:
		
	int status_code;

		
	std::string message;

		
	bool requestSuccessful;
};

	
typedef	struct DenateTeamPlayersDetails
{

	DenateTeamPlayersDetails(std::string playerName = "", std::string clientId = "")
	{
		this->playerName = playerName;
		this->clientId = clientId;
	}

public:

		
	std::string playerName;

		
	std::string clientId;

	bool operator==(const DenateTeamPlayersDetails& Other)
	{
		return clientId == Other.clientId && playerName == Other.playerName;
	}

	friend bool operator==(const DenateTeamPlayersDetails& A, const DenateTeamPlayersDetails& B)
	{
		return A.clientId == B.clientId && A.playerName == B.playerName;
	}

};

	
typedef	struct DenateOnlineSaveDetail
{

	DenateOnlineSaveDetail(std::string saveID = "", std::string filters = "", std::string Tag = "")
	{
		this->SaveID = saveID;
		this->filters = filters;
		this->Tag = Tag;
	}

public:

	std::string SaveID;

	std::string filters;

	std::string Tag;
};

	
typedef	struct DenatePlayerOnlineSaveDetail
{

	DenatePlayerOnlineSaveDetail(std::vector<int> objectBytes = {}, std::string saveID = "", std::string filters = "", std::string Tag = "")
	{

		this->objectBytes = objectBytes;
		this->SaveID = saveID;
		this->filters = filters;
		this->Tag = Tag;
	}

public:

		
	std::vector<int> objectBytes;

		
	std::string SaveID;

		
	std::string filters;

		
	std::string Tag;
};

	
typedef	struct DenateTeamMatchDetail
{

	DenateTeamMatchDetail(std::string filters = "", std::string TeamID = "")
	{
		this->filters = filters;
		this->TeamID = TeamID;
	}

public:

		
	std::string filters;

		
	std::string TeamID;
};

	
typedef	struct DenateTeamDetails
{

	DenateTeamDetails(int matchId = 0, std::string serverName = "", int maxPlayers = 0, std::string filters = "", bool isPrivateMatch = false, std::string teamId = "")
	{
		this->matchId = matchId;
		this->serverName = serverName;
		this->maxPlayers = maxPlayers;
		this->filters = filters;
		this->isPrivateMatch = isPrivateMatch;
		this->teamId = teamId;
	}

public:

		
	int matchId;

		
	std::string serverName;

		
	int maxPlayers;

		
	std::string filters;

		
	bool isPrivateMatch;

		
	std::string teamId;

	bool operator==(const DenateTeamDetails& Other)
	{
		return matchId == Other.matchId && serverName == Other.serverName && maxPlayers == Other.maxPlayers && filters == Other.filters && teamId == Other.teamId;
	}

	friend bool operator==(const DenateTeamDetails& A, const DenateTeamDetails& B)
	{
		return A.matchId == B.matchId && A.serverName == B.serverName && A.maxPlayers == B.maxPlayers && A.filters == B.filters && A.teamId == B.teamId;
	}

};

	
typedef	struct DenatePrivateMatchDetails
{

	DenatePrivateMatchDetails(int matchId = 0, std::string serverName = "", int maxPlayers = 0, std::string filters = "", std::string ipAddress = "", std::string mapName = "", std::string playerName = "", std::string gameCode = "")
	{
		this->matchId = matchId;
		this->serverName = serverName;
		this->maxPlayers = maxPlayers;
		this->filters = filters;
		this->ipAddress = ipAddress;
		this->mapName = mapName;
		this->playerName = playerName;
		this->gameCode = gameCode;
	}

public:

		
	int matchId;

		
	std::string serverName;

		
	std::string playerName;

		
	std::string mapName;

		
	std::string ipAddress;

		
	int maxPlayers;

		
	std::string filters;

		
	std::string gameCode;

};

	
typedef	struct DenateMatchDetails
{
		
	DenateMatchDetails(int matchId = 0, std::string serverName = "", int maxPlayers = 0, std::string filters = "", std::string ipAddress = "", std::string mapName = "", std::string playerName = "")
	{
		this->matchId = matchId;
		this->serverName = serverName;
		this->maxPlayers = maxPlayers;
		this->filters = filters;
		this->ipAddress = ipAddress;
		this->mapName = mapName;
		this->playerName = playerName;
	}

public:

		
	int matchId;

		
	std::string serverName;

		
	std::string playerName;

		
	std::string mapName;

	std::string ipAddress;

		
	int maxPlayers;

		
	std::string filters;


};

	
typedef	struct DenateUserDetails
{
		
	DenateUserDetails(std::string emailOrId = "", std::string username = "", std::string firstName = "", std::string lastName = "", std::string picture = "", std::string appUserID = "")
	{
		this->emailOrId = emailOrId;
		this->username = username;
		this->firstName = firstName;
		this->lastName = lastName;
		this->picture = picture;
		this->appUserID = appUserID;
	}

public:

		
	std::string emailOrId;

		
	std::string username;

		
	std::string firstName;

		
	std::string lastName;

		
	std::string picture;

		
	std::string appUserID;
};

	
struct DenatePlayersMatchDetails
{
		
	DenatePlayersMatchDetails(std::string playerName, std::string serverName, std::string joinedDate, int maxPlayers = 0, bool isServer = false, bool isPrivateMatch = false )
	{
		this->playerName = playerName;
		this->serverName = serverName;
		this->joinedDate = joinedDate;
		this->maxPlayers = maxPlayers;
		this->isServer = isServer;
		this->isPrivateMatch = isPrivateMatch;
	}

public:

		
	std::string playerName;

		
	std::string serverName;

		
	std::string joinedDate;

		
	int maxPlayers;

		
	bool isServer;

		
	bool isPrivateMatch;

};

namespace DenateTypes {

	class DOS_DenateTypes
	{
	public:

		/** Constructor */
		DOS_DenateTypes();

		/** Base url of the denate API */
		static const std::string denateapiURL(){
			return "http://localhost:3000/api/v1/";
		}

		/** returns the base url of the denate API*/
		static std::string getdenateapiURL();

		

	protected:
		

	private:

	};
}