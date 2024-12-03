// Copyright 2024, David Olashege, All rights reserved

#pragma once

#include <stdio.h>
#include <string>
#include <curl/curl.h>
#include "json.hpp"
#include <vector>
#include "DenateTypes.h"
#include <sio_client.h>
//#include "../ThirdParty/socket.io-client-cpp-3.1.0/src/sio_socket.h"
//#include "../ThirdParty/socket.io-client-cpp-3.1.0/src/sio_message.h"

namespace DenateConnection
{

	class DOS_Connection
	{
    protected:

        typedef std::function<void(void)> onlineServiceConnectedListener;

        typedef std::function<void(void)> onlineServiceDisconnectedListener;

        typedef std::function<void(std::string playerName)> invitationRecievedListener;

        typedef std::function<void(std::string playerName)> friendRequestRecievedListener;

        typedef std::function<void(std::string playerName)> friendRequestDeniedListener;

        typedef std::function<void(std::string playerName)> friendRequestAcceptedListener;

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

        /** Internal use only*/
        onlineServiceConnectedListener internalOnlineserviceConnected ;

        /** Internal use only*/
        onlineServiceDisconnectedListener internalOnlineServiceDisconnected ;

        /** Internal use only*/
        invitationRecievedListener internalInvitationRecieved;

        /** Internal use only*/
        friendRequestRecievedListener internalFriendRequestRecieved;

        /** Internal use only*/
        friendRequestDeniedListener internalFriendRequestDenied;

        /** Internal use only*/
        friendRequestAcceptedListener internalFriendRequestAccepted;

    public:

        /** Called when the denate online service is successfully connected*/
        void OnDenateOnlineServiceConnected(onlineServiceConnectedListener const& onlineServiceConnected);

        /** Called when the denate online service is successfully connected*/
        void OnDenateOnlineServiceDisconnected(onlineServiceDisconnectedListener const& onlineServiceDisconnected);

        /** Called when the current player instance gets an invite from another player*/
        void OnInvitationRecieved(invitationRecievedListener const& invitationRecieved);

        /** Called when the current player instance recieves a friend request from another player*/
        void OnFriendRequestRecieved(friendRequestRecievedListener const& friendRequestRecieved);

        /** Called when the current player instance friend request is denied by the player the request was sent to*/
        void OnFriendRequestDenied(friendRequestDeniedListener const& friendRequestDenied);

        /** Called when the current player instance friend request is accepted by the player the request was sent to*/
        void OnFriendRequestAccepted(friendRequestAcceptedListener const& friendRequestAccepted);

        /** Constructor */
        DOS_Connection(std::string userID, std::string appID, bool dedicatedServer, std::string token, DenateUserDetails userDetails);

        /** If the current instance of this game is connected to denate online service. */
        bool isDenateOnlineServiceConnected;

        /** Socket IO client */
        sio::client sioClient;
        
        /** Namespace socket for the denate connection */
        sio::socket::ptr namespaceSocket;

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

        /** Establishes a connection between the current instance of this game and denate online service. Leave world empty when establishing connection manually. By default a connection should be established automatically. Always check using bisDenateOnlineServiceConnected before establishing another connection.
        * @param dedicatedServer If the current instance trying to establish a connection is a dedicated server (OPTIONAL)
        */
        void EstablishDenateConnection(bool isDedicatedServer);

	};

}
