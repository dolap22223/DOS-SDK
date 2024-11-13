#pragma once

#include <stdio.h>
#include <string>
#include <curl/curl.h>
#include <json.hpp>
#include <vector>
#include "DenateTypes.h"

struct GetAllGameAchievementsResult
{
    DenateHTTPResponse httpResponse;
    bool gottenAchievement;
    std::vector<DenateAchievementDetails> achievementDetails;
};

struct GetAllPlayerAchievementsResult
{
    DenateHTTPResponse httpResponse;
    bool gottenAchievement;
    std::vector<DenatePlayerAchievementDetails> achievementDetails;
};

struct LockOrUnlockAchievementByAchievementResult
{
    DenateHTTPResponse httpResponse;
    bool achievementUpdated;
    bool achievementUnlocked;
    DenateAchievementDetails achievementDetails;
};

namespace DenateAchievement{

	class DOS_Achievement
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
        DOS_Achievement(std::string userID, std::string appID, bool dedicatedServer, std::string token, DenateUserDetails userDetails = DenateUserDetails());

        /** Gets all achievement associated to this game
        * @return GetAllGameAchievementsResult
        */
        GetAllGameAchievementsResult GetAllGameAchievements();

        /** Gets all unlocked achievement associated to the current player.
        * @param playerName Name(Denate Username) of the player you would like to get their unlocked achievement
        * @return GetAllGameAchievementsResult
        */
        GetAllGameAchievementsResult GetAllPlayerUnlockedAchievements(std::string playerName);

        /** Gets all locked achievement associated to the current player
        * @param playerName Name(Denate Username) of the player you would like to get their locked achievement
        * @return GetAllGameAchievementsResult
        */
        GetAllGameAchievementsResult GetAllPlayerLockedAchievements(std::string playerName);

        /** lock or unlock achievement associated to the current player by the achievement name
        * @param playerName Name(Denate Username) of the player you would like to lock/unlock their achievement
        * @param achievementName Name of the achievement to unlock/lock
        * @param Unlock True if you would like to unlock the specified achievement
        * @return LockOrUnlockAchievementByAchievementResult
        */
        LockOrUnlockAchievementByAchievementResult LockOrUnlockAchievementByAchievementName(std::string achievementName, bool Unlock, std::string playerName);
        
        /** lock or unlock achievement associated to the current player by the achievementID
        * @param playerName Name(Denate Username) of the player you would like to lock/unlock their achievement
        * @param achievementID ID of the achievement to unlock/lock
        * @param Unlock True if you would like to unlock the specified achievement
        * @return LockOrUnlockAchievementByAchievementResult
        */
        LockOrUnlockAchievementByAchievementResult LockOrUnlockAchievementByAchievementID(std::string achievementID, bool Unlock, std::string playerName);

        /** Gets all achievement associated to the current player
        * @param playerName Name(Denate Username) of the player you would like to get their achievement
        * @return GetAllPlayerAchievementsResult
        */
        GetAllPlayerAchievementsResult GetAllPlayerAchievements(std::string playerName);

	};

}