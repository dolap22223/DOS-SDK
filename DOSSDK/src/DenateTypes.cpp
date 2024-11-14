#include "DenateTypes.h"

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

namespace DenateTypes {

    
	DOS_DenateTypes::DOS_DenateTypes()
	{
		
	}



	std::string DOS_DenateTypes::getdenateapiURL()
	{
		//std::string localurl = denateapiURL;
		return denateapiURL();
	}

}
