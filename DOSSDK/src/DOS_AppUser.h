// Copyright 2024, David Olashege, All rights reserved

#pragma once

#include "DenateTypes.h"
#include <fstream>
#include "json.hpp"
#include "DenateTypes.h"
#include <stdio.h>

struct LoginAppUserResult
{
	DenateHTTPResponse httpResponse;
	bool loggedIn;
	std::string token;
	DenateUserDetails userDetails;
};

struct SignUpAppUserResult
{
	DenateHTTPResponse httpResponse;
	bool signedUp;
	DenateUserDetails userDetails;
};

struct UpdateAppUserResult
{
	DenateHTTPResponse httpResponse;
	bool updated;
	DenateUserDetails userDetails;
};

struct AddDenateAppUserVerificationResult
{
	DenateHTTPResponse httpResponse;
	bool codesent;
};

struct VerifyDenateAppUserResult
{
	DenateHTTPResponse httpResponse;
	bool verified;
};

struct ChangeDenateAppUserPasswordResult
{
	DenateHTTPResponse httpResponse;
	bool passwordChanged;
};

struct DoesDenateUserExistResult
{
	DenateHTTPResponse httpResponse;
	bool userExist;
};

struct GetPlayerDetailsResult
{
	DenateHTTPResponse httpResponse;
	bool gottenDetails;
	DenateUserDetails userDetails;
};

namespace DenateAppUser {



	class DOS_AppUser 
	{
	private:

		/** userID credential */
		std::string userID;

		/** appID credential */
		std::string appID;

		/** Details of the user after logging in */
		DenateUserDetails userDetails;

		/** token after logging in */
		std::string token;

		/** True if the current instance is a dedicated server */
		bool dedicatedServer = false;

		/** Replaces a substring with another*/
		static void replaceSubstring(std::string& original, const std::string& toReplace, const std::string& replacement);

	public:

		/** Returns the users token after logging in */
		std::string getToken();

		/** Returns the users details after logging in */
		DenateUserDetails getUserDetails();

		/** DOS_AppUser constructor*/
		DOS_AppUser(std::string userID, std::string appID, bool dedicatedServer);
		
		/** Log's in a user. Email or username can be accepted as EmailorUsername
		* @param EmailOrIdOrUsername Email/ID or Username of the denate user to be logged in
		* @param Password Password for the denate account of the user to be logged in
		* @return LoginAppUserResult
		*/
		LoginAppUserResult LoginDenateAppUser(std::string EmailOrIdOrUsername, std::string Password);

		/** Log's in a user only by their Email/Id or username. Email/ID or username can be accepted
		* @param EmailOrIdOrUsername Email/ID or Username of the denate user to be logged in
		* @return LoginAppUserResult
		*/
		LoginAppUserResult LoginDenateAppUserById(std::string EmailOrIdOrUsername);

		/** Signs up a user. Email, Username and Password are COMPULSORY fields and must be UNIQUE while First name, Last name, phone number, picture are optional fields. Note: We recommend Email address should be verified before creating an account
		* @param EmailOrId Email or a unique id for example steam ID of a player you would like to sign up.
		So its recommended to check the existence of the Email/ID using the Does Denate User Exist function before proceeding.
		* @param Username A unique name for the player being signed up. Note it has to be unique, meaning no two player must have the same username.
		So its recommended to check the existence of the username using the Does Denate User Exist function before proceeding.
		* @param FirstName Firstname of the player you would like to sign up (OPTIONAL)
		* @param LastName LastName of the player you would like to sign up (OPTIONAL)
		* @param Password Password for the player you would like to sign up (OPTIONAL)
		* @param PhoneNumber Phone Number of the player you would like to sign up (OPTIONAL)
		* @param Photo Link to the players Photo you would like to sign up (OPTIONAL)
		* @return SignUpAppUserResult
		*/
		SignUpAppUserResult SignUpDenateAppUser(std::string Email, std::string Username, std::string FirstName = "", std::string LastName = "", std::string Password = "", std::string PhoneNumber = "", std::string Photo = "");

		/** Update a user account. Username is a COMPULSORY field and must be UNIQUE while First name, Last name, phone number, picture are optional fields. Note: This node should only be called after the user has LOGGED IN
		* @param Username A unique name for the player account being updated. Note it has to be unique, meaning no two player must have the same username.
		So its recommended to check the existence of the username using the Does Denate User Exist function before proceeding.
		* @param FirstName Firstname to be updated (OPTIONAL)
		* @param LastName LastName to be updated (OPTIONAL)
		* @param Password Password to be updated (OPTIONAL)
		* @param PhoneNumber Phone Number to be updated (OPTIONAL)
		* @param Photo Link to the players Photo to be updated (OPTIONAL)
		* @return UpdateAppUserResult
		*/
		UpdateAppUserResult UpdateDenateAppUser(std::string appUserID, std::string Username = "", std::string FirstName = "", std::string LastName = "", std::string Password = "", std::string PhoneNumber = "", std::string Photo = "");

		/** Start the verification process of a user. Email represents the users email. Note: A verification code would be sent to the email and would be confirmed using the Verify Denate App User node
		* @param Email Email of the denate user to be verified
		* @return AddDenateAppUserVerificationResult
		*/
		AddDenateAppUserVerificationResult AddDenateAppUserVerification(std::string Email);

		/** Verifies Denate App User. Verification code should be sent to an email after calling Add denate app user verification function
		* @param Email Email of the user to verify their email provided they signed up or intent to sign up using an email address and not an ID such as their steam ID
		* @param VerificationCode Verification code gotten from the email sent after starting email verification
		* @return VerifyDenateAppUserResult
		*/
		VerifyDenateAppUserResult VerifyDenateAppUser(std::string Email, std::string VerificationCode);

		/** Should be used when an app user forgets their password. A new password should be sent to the users email after the user can change their password using the Change Denate App User Password node
		* @param Email Email of the user to reset their password provided they signed up using an email address and not an ID such as their steam ID
		* @return AddDenateAppUserVerificationResult
		*/
		AddDenateAppUserVerificationResult ForgotDenateAppUserPassword(std::string Email);

		/** Changes an App User's password. When password has been forgotten, there should be a node called Forgot Denate app Password being called first then an email would be sent before calling Change Denate App user password function
		* @param OldPassword The players old password
		* @param NewPassword The players new password to be changed to
		* @return ChangeDenateAppUserPasswordResult
		*/
		ChangeDenateAppUserPasswordResult ChangeDenateAppUserPassword(std::string Email, std::string OldPassword, std::string NewPassword);

		/** Checks if a user exists on the denate online service
		* @param EmailOrIdOrUsername Email/ID or username of the player to check if the denate user exist
		* @return DoesDenateUserExistResult
		*/
		DoesDenateUserExistResult DoesDenateUserExist(std::string EmailOrIdOrUsername);

		/** gets a player details by their player name
		* @param EmailOrIdOrUsername Email/ID or username of the player to get their details
		* @return GetPlayerDetailsResult
		*/
		GetPlayerDetailsResult GetPlayerDetails(std::string EmailOrIdOrUsername);

	};

}