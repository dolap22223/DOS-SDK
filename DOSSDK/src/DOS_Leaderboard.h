#pragma once

#include <stdio.h>
#include <string>
#include <curl/curl.h>
#include <json.hpp>
#include <vector>
#include "DenateTypes.h"

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

	};
}

