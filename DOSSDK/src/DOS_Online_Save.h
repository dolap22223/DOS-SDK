#pragma once

#include <stdio.h>
#include <string>
#include <curl/curl.h>
#include "json.hpp"
#include <vector>
#include "DenateTypes.h"

struct AddOnlineSaveResult
{
    DenateHTTPResponse httpResponse;
    bool saved;
    DenateOnlineSaveDetail saveDetail;
};

struct GetOnlineSaveResult
{
    DenateHTTPResponse httpResponse;
    bool saveGotten;
    DenatePlayerOnlineSaveDetail saveDetail;
};

struct GetAllPlayerSavesResult
{
    DenateHTTPResponse httpResponse;
    bool saveGotten;
    std::vector<DenatePlayerOnlineSaveDetail> saveDetails;
};

struct DeleteSaveResult
{
    DenateHTTPResponse httpResponse;
    bool saveDeleted;
};

namespace DenateOnlineSave 
{

	class DOS_Online_Save
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
        DOS_Online_Save(std::string userID, std::string appID, bool dedicatedServer, std::string token, DenateUserDetails userDetails);

        /** Adds an online save
        * @param SaveGameObject Save game object to save
        * @param Tag A custom unique tag to use to identify a save
        * @param filters used to specify extra info about the leaderboard for example (score=50,name=john)
        * @return AddOnlineSaveResult
        */
        AddOnlineSaveResult AddOnlineSave(std::vector<int> OutSaveData, std::string Tag, std::string Filters);

        /** Gets a save by tag
        * @param Tag A custom unique tag to use to identify a save
        * @param playerName Name of player whose save to get. Leave empty to use the user detail specified in the constructor
        * @return GetOnlineSaveResult
        */
        GetOnlineSaveResult GetSaveByTag(std::string Tag, std::string playerName = "");

        /** Gets a save by a saveID
        * @param saveID A unique ID representing each online save
        * @param playerName Name of player whose save to get. Leave empty to use the user detail specified in the constructor
        * @return GetOnlineSaveResult
        */
        GetOnlineSaveResult GetSaveBySaveID(std::string saveID, std::string playerName = "");

        /** Gets the most recent save by the currently logged in player
        * @param playerName Name of player whose save to get. Leave empty to use the user detail specified in the constructor
        * @return GetOnlineSaveResult
        */
        GetOnlineSaveResult GetLatestPlayerSave(std::string playerName = "");

        /** Gets all saves associated to a player
        * @param playerName Name of player whose save to get. Leave empty to use the user detail specified in the constructor
        * @return GetAllPlayerSavesResult
        */
        GetAllPlayerSavesResult GetAllPlayerSaves(std::string playerName = "");

        /** Deletes a save by tag
        * @param Tag A custom unique tag associated to the save you would like to delete
        * @param playerName Name of player whose save should be deleted. Leave empty to use the user detail specified in the constructor
        * @return DeleteSaveResult
        */
        DeleteSaveResult DeleteSaveByTag(std::string Tag, std::string playerName = "");

        /** Deletes a save by saveID
        * @param SaveID A unique ID representing the online save you would like deleted
        * @param playerName Name of player whose save should be deleted. Leave empty to use the user detail specified in the constructor
        * @return DeleteSaveResult
        */
        DeleteSaveResult DeleteSaveBySaveID(std::string saveID, std::string playerName = "");

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

