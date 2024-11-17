#include "DOS_Connection.h"
#include <iostream>

namespace DenateConnection
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

    void DOS_Connection::replaceSubstring(std::string& original, const std::string& toReplace, const std::string& replacement)
    {
        size_t pos = 0;
        while ((pos = original.find(toReplace, pos)) != std::string::npos)
        {
            original.replace(pos, toReplace.length(), replacement);
            pos += replacement.length();
        }
    }

    DOS_Connection::DOS_Connection(std::string userID, std::string appID, bool dedicatedServer, std::string token, DenateUserDetails userDetails)
    {
        this->userID = userID;
        this->appID = appID;
        this->dedicatedServer = dedicatedServer;
        this->token = token;
        this->userDetails = userDetails;
    }

    std::vector<DenateFilterResult> DOS_Connection::BreakDenateFilter(std::string Filter)
    {
        std::vector<DenateFilterResult> result;
        std::vector<std::string> brokendownfilters;
        brokendownfilters = parseIntoVector(Filter, ",");
        for (int i = 0; i < brokendownfilters.size(); i++)
        {
            std::vector<std::string> titleandvalue;
            titleandvalue = parseIntoVector(brokendownfilters[i], "=");

            DenateFilterResult localresult;
            localresult.title = titleandvalue[0];
            localresult.value = titleandvalue[1];

            result.push_back(localresult);

        }
        return result;
    }

    std::string DOS_Connection::MakeDenateFilter(std::vector<DenateFilterResult> filters)
    {
        std::string result;
        for (int i = 0; i < filters.size(); i++)
        {
            if (i == filters.size() - 1)
            {
                result = result + filters[i].title + "=" + filters[i].value;
            }
            else {
                result = result + filters[i].title + "=" + filters[i].value + ",";
            }

        }
        return result;
    }

}