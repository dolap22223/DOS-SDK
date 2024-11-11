#pragma once

#include "DenateTypes.h"
#include <fstream>
#include <json.hpp>
#include "DenateTypes.h"
#include <stdio.h>

struct LoginAppUserResult
{
	DenateHTTPResponse httpResponse;
	bool loggedIn;
	DenateUserDetails userDetails;
};

struct SignUpAppUserResult
{
	DenateHTTPResponse httpResponse;
	bool signedUp;
	DenateUserDetails userDetails;
};

namespace DenateAppUser {



	class DOS_AppUser 
	{
	private:

		std::string userID;

		std::string appID;

		DenateUserDetails userDetails;

		std::string token;

		bool dedicatedServer = false;

	public:

		std::string getToken();

		DenateUserDetails getUserDetails();

		DOS_AppUser(std::string userID, std::string appID, bool dedicatedServer);
		
		LoginAppUserResult LoginDenateAppUser(std::string EmailOrIdOrUsername, std::string Password);

		LoginAppUserResult LoginDenateAppUserById(std::string EmailOrIdOrUsername);

		SignUpAppUserResult SignUpDenateAppUser(std::string Email, std::string Username, std::string FirstName = "", std::string LastName = "", std::string Password = "", std::string PhoneNumber = "", std::string Photo = "");

	};

}