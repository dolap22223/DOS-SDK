#pragma once

#include <stdio.h>
#include <string>
#include <curl/curl.h>
#include <json.hpp>
#include <vector>
#include "DenateTypes.h"

namespace DenateConnection
{

	class DOS_Connection
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
        DOS_Connection(std::string userID, std::string appID, bool dedicatedServer, std::string token, DenateUserDetails userDetails);

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
