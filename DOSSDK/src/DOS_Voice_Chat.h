// Copyright 2024, David Olashege, All rights reserved
#pragma once

#include <stdio.h>
#include <string>
#include <curl/curl.h>
#include "json.hpp"
#include <vector>
#include <sio_client.h>
#include "DenateTypes.h"
#include "DOS_Connection.h"

struct CreateVoiceChannelResult
{
    DenateHTTPResponse httpResponse;
    bool createdChannel;
    std::string channelID;
};

struct JoinVoiceChannelResult
{
    DenateHTTPResponse httpResponse;
    bool joined;
};

struct LeaveVoiceChannelResult
{
    DenateHTTPResponse httpResponse;
    bool leftChannel;
};

struct DestroyVoiceChannelResult
{
    DenateHTTPResponse httpResponse;
    bool destroyedChannel;
};

struct GetAllVoiceConnectedPlayersResult
{
    DenateHTTPResponse httpResponse;
    bool gottenPlayers;
    std::vector<DenateVoiceChannelDetails> voiceChannels;
};

struct GetAllPlayerConnectedChannelsResult
{
    DenateHTTPResponse httpResponse;
    bool gottenChannels;
    std::vector<DenatePlayerChannelDetails> playerChannelDetails;
};

namespace DenateVoiceChat
{

	class DOS_Voice_Chat
	{
    protected:

        typedef std::function<void(DenateVoiceChannelDetails playerChannelDetail)> playerJoinVoiceChannelListener;

        typedef std::function<void(DenateVoiceChannelDetails playerChannelDetail)> playerLeftVoiceChannelListener;

        typedef std::function<void(std::string channel)> playerDestroyVoiceChannelListener;

        typedef std::function<void(std::string playerName, std::vector<int> audioData)> recieveVoiceDataListener;

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

        /** Denate connection */
        DenateConnection::DOS_Connection& internalDenateConnection;

        /** Replaces a substring with another*/
        void replaceSubstring(std::string& original, const std::string& toReplace, const std::string& replacement);

        /** Internal use only*/
        playerJoinVoiceChannelListener internalPlayerJoinVoiceChannel;

        /** Internal use only*/
        playerLeftVoiceChannelListener internalPlayerLeftVoiceChannel;

        /** Internal use only*/
        playerDestroyVoiceChannelListener internalPlayerDestroyVoiceChannel;

        /** Internal use only*/
        recieveVoiceDataListener internalRecieveVoiceData;


    public:

        /** Called when a player joins a channel */
        void OnPlayerJoinVoiceChannel(playerJoinVoiceChannelListener const& playerJoinVoiceChannel);

        /** Called when a player leaves a channel you  are a part of */
        void OnPlayerLeftVoiceChannel(playerLeftVoiceChannelListener const& playerLeftVoiceChannel);

        /** Called when a channel is destroyed */
        void OnPlayerDestroyVoiceChannel(playerDestroyVoiceChannelListener const& playerDestroyVoiceChannel);

        /** Called when audio data is sent */
        void OnRecieveVoiceData(recieveVoiceDataListener const& recieveVoiceData);

        /** Constructor */
        DOS_Voice_Chat(std::string userID, std::string appID, bool dedicatedServer, std::string token, DenateUserDetails userDetails, DenateConnection::DOS_Connection& denateConnection);

        /** Details of the last voice channel you joined */
        DenateVoiceChannelDetails currentVoiceChannel;

        /** Namespace socket for the denate voice chat */
        sio::socket::ptr namespaceSocket;

        /** True if the denate voice chat has been activated*/
        bool voiceChatActivated;

        /** holds all connected channels */
        std::vector<DenateVoiceChannelDetails> allConnectedChannels;

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

        /** Creates a voices channel which players can join and listen and communciate with other players
        * @return CreateVoiceChannelResult
        */
        CreateVoiceChannelResult CreateVoiceChannel();

        /** joins a voices channel which players can join and listen and communciate with other players
        * @param ChannelID Unique Channel ID of the channel to join
        * @param player_name name of the player you would like to get their leaderboard, leave empty to specify yourself (OPTIONAL)
        * @return JoinVoiceChannelResult
        */
        JoinVoiceChannelResult JoinVoiceChannel(std::string channelID, std::string playerName = "");

        /** Leave a voices channel which players can join and listen and communciate with other players
        * @param ChannelID Unique Channel ID of the channel to leave
        * @param player_name name of the player you would like to get their leaderboard, leave empty to specify yourself (OPTIONAL)
        * @return LeaveVoiceChannelResult
        */
        LeaveVoiceChannelResult LeaveVoiceChannel(std::string channelID, std::string playerName = "");

        /** Destroy a voices channel which players can join and listen and communciate with other players
        * @param ChannelID Unique Channel ID of the channel to destroy
        * @return DestroyVoiceChannelResult
        */
        DestroyVoiceChannelResult DestroyVoiceChannel(std::string channelID);

        /** Gets all voice connected players for a channel
        * @param ChannelID Unique Channel ID of the channel to get all connected players
        * @return GetAllVoiceConnectedPlayersResult
        */
        GetAllVoiceConnectedPlayersResult GetAllVoiceConnectedPlayers(std::string channelID);

        /** Gets all player connected channels
        * @param player_name name of the player you would like to get all the channels they are connected to, leave empty to specify yourself (OPTIONAL)
        * @return GetAllPlayerConnectedChannelsResult
        */
        GetAllPlayerConnectedChannelsResult GetAllPlayerConnectedChannels(std::string playerName = "");

        /** Activates the denate voice chat */
        void Activate();

        /** Deactivates the denate voice chat */
        void Deactivate();

        /** Sends the voice audio data */
        void sendAudioData(std::vector<int> audioData, std::vector<std::string> clients);

	};

}

