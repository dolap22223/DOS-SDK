#pragma once
#include <string>
#include <curl/curl.h>




typedef enum class DenateRequestResult: UINT8 
{
	RequestSuccessful,
	RequestFailed
};

typedef enum class EServerHostResult : UINT8
{
	HostingSuccessful ,
	HostingFailed
};

	
typedef	enum class EFindMatchResult : UINT8
{
	MatchFound ,
	MatchNotFound
};

typedef struct DenateLeaderboardResult
{
	DenateLeaderboardResult()
	{
		playerName = "";
		filters = "";
		LeaderboardName = "";
	}

public:

	std::string playerName;

	std::string filters;

	std::string LeaderboardName;
};

typedef	struct DenateFilterResult
{

	DenateFilterResult()
	{
		title = "";
		value = "";
	}

public:

	std::string title;

	std::string value;
};

typedef	struct DenateFriendDetails
{

	DenateFriendDetails()
	{
		playerName = "";
		picture = "";
		online = false;
		inGame = false;
		appUserID = "";
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

	DenatePlayerAchievementDetails()
	{
		achievementName = "";
		achievementDescription = "";
		lockedAchievementName = "";
		lockedAchievementDescription = "";
		unlockedAchievementName = "";
		unlockedAchievementDescription = "";
		achievementID = "";
		unlockedAchievementImageURL = "";
		unlocked = false;
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

	DenateAchievementDetails()
	{
		achievementName = "";
		achievementDescription = "";
		lockedAchievementName = "";
		lockedAchievementDescription = "";
		unlockedAchievementName = "";
		unlockedAchievementDescription = "";
		achievementID = "";
		lockedAchievementImageURL = "";
		unlockedAchievementImageURL = "";
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

	DenateRoomDetails()
	{
		roomId = "";
		clientId = "";
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

	DenatePlayerVoiceInfo()
	{
		microphoneDisabled = false;
		muted = false;
		playerName = "";
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


	DenateVoiceChannelDetails()
	{
		channelId = "";
		clientId = "";
		playerName = "";
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

	DenateSearchUserDetails()
	{
		playerName = "";
		picture = "";
		appUserID = "";
		online = false;
		inGame = false;
		friendRequestSent = false;
		friendRequestRecieved = false;
		is_a_Friend = false;
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
		
	DenateHTTPResponse()
	{
		status_code = 0;
		message = "";
		requestSuccessful = false;
	}

public:
		
	int status_code;

		
	std::string message;

		
	bool requestSuccessful;
};

	
typedef	struct DenateTeamPlayersDetails
{

	DenateTeamPlayersDetails()
	{
		playerName = "";
		clientId = "";
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

	DenateOnlineSaveDetail()
	{
		SaveID = "";
		filters = "";
		Tag = "";
	}

public:

	std::string SaveID;

	std::string filters;

	std::string Tag;
};

	
typedef	struct DenatePlayerOnlineSaveDetail
{

	DenatePlayerOnlineSaveDetail()
	{
		objectBytes = "";
		SaveID = "";
		filters = "";
		Tag = "";
	}

public:

		
	std::string objectBytes;

		
	std::string SaveID;

		
	std::string filters;

		
	std::string Tag;
};

	
typedef	struct DenateTeamMatchDetail
{

	DenateTeamMatchDetail()
	{
		filters = "";
		TeamID = "";
	}

public:

		
	std::string filters;

		
	std::string TeamID;
};

	
typedef	struct DenateTeamDetails
{

	DenateTeamDetails()
	{
		matchId = 0;
		serverName = "";
		maxPlayers = 0;
		filters = "";
		isPrivateMatch = false;
		teamId = "";
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

	DenatePrivateMatchDetails()
	{
		matchId = 0;
		serverName = "";
		maxPlayers = 0;
		filters = "";
		ipAddress = "";
		mapName = "";
		playerName = "";
		gameCode = "";
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
		
	DenateMatchDetails()
	{
		matchId = 0;
		serverName = "";
		maxPlayers = 0;
		filters = "";
		ipAddress = "";
		mapName = "";
		playerName = "";
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
		
	DenateUserDetails()
	{
		emailOrId = "";
		username = "";
		firstName = "";
		lastName = "";
		picture = "";
		appUserID = "";
	}

public:

		
	std::string emailOrId;

		
	std::string username;

		
	std::string firstName;

		
	std::string lastName;

		
	std::string picture;

		
	std::string appUserID;
};

	
typedef struct DenatePlayersMatchDetails
{
		
	DenatePlayersMatchDetails()
	{
		playerName = "";
		serverName = "";
		joinedDate = "";
		maxPlayers = 0;
		isServer = false;
		isPrivateMatch = false;
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

		DOS_DenateTypes();

		static const std::string denateapiURL(){
			return "http://localhost:3000/api/v1/";
		}

		static std::string getdenateapiURL();

	protected:
		

	private:

	};
}