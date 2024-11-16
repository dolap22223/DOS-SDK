#include "DOS_Leaderboard.h"

namespace DenateLeaderboard
{

    std::vector<std::string> parseIntoVector(const std::string& str, const std::string& delimiter) {
        std::vector<std::string> filters;
        size_t start = 0;
        size_t end = str.find(delimiter);

        while (str.find(delimiter))
        {
            filters.push_back(str.substr(start, end));
            start = end + delimiter.length();
            end = str.find(delimiter, start);
        }
        filters.push_back(str.substr(start));
        return filters;
    }

    size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
    {

        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;

    }

    void DOS_Leaderboard::replaceSubstring(std::string& original, const std::string& toReplace, const std::string& replacement)
    {
        size_t pos = 0;
        while ((pos = original.find(toReplace, pos)) != std::string::npos)
        {
            original.replace(pos, toReplace.length(), replacement);
            pos += replacement.length();
        }
    }

    DOS_Leaderboard::DOS_Leaderboard(std::string userID, std::string appID, bool dedicatedServer, std::string token, DenateUserDetails userDetails)
    {
        this->userID = userID;
        this->appID = appID;
        this->dedicatedServer = dedicatedServer;
        this->token = token;
        this->userDetails = userDetails;
    }

}