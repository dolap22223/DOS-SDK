#include "DOS_Connection.h"
#include <iostream>
#include <chrono>
#include <thread>
#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

namespace DenateConnection
{

    std::string getHostName() {
        char hostname[256];

#if defined(_WIN32) || defined(_WIN64)
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            return "Error initializing Winsock";
        }
        if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) {
            WSACleanup();
            return "Error getting hostname";
        }
        WSACleanup();
#else
        if (gethostname(hostname, sizeof(hostname)) != 0) {
            return "Error getting hostname";
        }
#endif

        return std::string(hostname);
    }

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

    void on_connected() 
    { 
        std::cout << "Connected to namespace!" << std::endl; 
    } 
    
    void on_message_received(const std::string& name, const sio::message::ptr& data, bool isAck, sio::message::ptr& ack_resp) 
    { 
        std::cout << "Message received in namespace: " << name << std::endl; 
    }

    //void on_close(sio::client::close_reason const& reason) 
    //{
    //    std::cout << "Connection closed: " << reason << std::endl; // Attempt to reconnect 
    //    sioClient.connect(url, query, headers); 
    //}

    void DOS_Connection::EstablishDenateConnection(bool isDedicatedServer)
    {
        
        if (!sioClient.opened())
        {
            std::map<std::string, std::string> query = {};
            std::map<std::string, std::string> headers = {};

            headers.insert(std::make_pair("Authorization", "Bearer " + token));
            headers.insert(std::make_pair("dedicatedServer", isDedicatedServer ? "True" : "False"));
            headers.insert(std::make_pair("appID", appID));
            headers.insert(std::make_pair("userID", userID));
            std::cout << DenateTypes::DOS_DenateTypes::denateapigatewayURL().c_str() + std::string("denateconnectiongateway") << std::endl;
            std::string url = DenateTypes::DOS_DenateTypes::denateapigatewayURL();

            sioClient.set_open_listener([]() { std::cout << "Denate Connection opened" << std::endl; });

            sioClient.set_close_listener([&](sio::client::close_reason const& reason) {

                std::cout << "Disconnected from the server" << std::endl;

                isDenateOnlineServiceConnected = false;

                if (internalOnlineServiceDisconnected)
                {
                    internalOnlineServiceDisconnected();
                }

            });
            
            sioClient.set_fail_listener([&]() { 

                std::cout << "Disconnected from the server" << std::endl; 

                isDenateOnlineServiceConnected = false;

                if (internalOnlineServiceDisconnected)
                {
                    internalOnlineServiceDisconnected();
                }
                
                });

            sioClient.connect(url, query, headers);
            namespaceSocket = sioClient.socket("/denateconnectiongateway");

            namespaceSocket->on("playerconnected", [&](sio::event& ev) {

                isDenateOnlineServiceConnected = true;
                std::cout << "Connected to Denate Online Service" << std::endl;

                if (internalOnlineserviceConnected) 
                {
                    internalOnlineserviceConnected();
                }

                });

            namespaceSocket->on("playerinvited", [&](sio::event& ev) {

                std::cout << "Incoming Invitation" << std::endl;

                std::string playerName;

                auto data = ev.get_message();

                if (data->get_flag() == sio::message::flag_object)
                {
                    auto jsonObj = data->get_map();
                    if (jsonObj.find("player_name") != jsonObj.end())
                    {
                        playerName = jsonObj["player_name"]->get_string();
                    }
                }
                if (internalInvitationRecieved)
                {
                    internalInvitationRecieved(playerName);
                }

                });

            namespaceSocket->on("playeradded", [&](sio::event& ev) {

                std::cout << "Incoming Friend Request" << std::endl;

                std::string playerName;

                auto data = ev.get_message();

                if (data->get_flag() == sio::message::flag_object)
                {
                    auto jsonObj = data->get_map();
                    if (jsonObj.find("player_name") != jsonObj.end())
                    {
                        playerName = jsonObj["player_name"]->get_string();
                    }
                }

                if (internalFriendRequestRecieved)
                {
                    internalFriendRequestRecieved(playerName);
                } 

                });

            namespaceSocket->on("friendrequestdenied", [&](sio::event& ev) {

                std::cout << "Friend Request Denied Message Incoming" << std::endl;

                std::string playerName;

                auto data = ev.get_message();

                if (data->get_flag() == sio::message::flag_object)
                {
                    auto jsonObj = data->get_map();
                    if (jsonObj.find("player_name") != jsonObj.end())
                    {
                        playerName = jsonObj["player_name"]->get_string();
                    }
                }

                if (internalFriendRequestDenied)
                {
                    internalFriendRequestDenied(playerName);
                }

                });

            namespaceSocket->on("friendrequestaccepted", [&](sio::event& ev) {

                std::cout << "Friend Request Accepted Message Incoming" << std::endl;

                std::string playerName;

                auto data = ev.get_message();

                if (data->get_flag() == sio::message::flag_object)
                {
                    auto jsonObj = data->get_map();
                    if (jsonObj.find("player_name") != jsonObj.end())
                    {
                        playerName = jsonObj["player_name"]->get_string();
                    }
                }

                if (internalFriendRequestAccepted)
                {
                    internalFriendRequestAccepted(playerName);
                }

                });

            sio::message::ptr jsonMessage = sio::object_message::create();
            if (isDedicatedServer)
            {
                const std::string& hostName = getHostName();

                jsonMessage->get_map()["email"] = sio::string_message::create(hostName);
                jsonMessage->get_map()["player_name"] = sio::string_message::create(hostName);
                jsonMessage->get_map()["appuserID"] = sio::string_message::create(hostName);
                jsonMessage->get_map()["dedicatedServer"] = sio::bool_message::create(isDedicatedServer);
            }
            else {
                jsonMessage->get_map()["email"] = sio::string_message::create(userDetails.emailOrId);
                jsonMessage->get_map()["player_name"] = sio::string_message::create(userDetails.username);
                jsonMessage->get_map()["appuserID"] = sio::string_message::create(userDetails.appUserID);
            }

            jsonMessage->get_map()["appID"] = sio::string_message::create(appID);
            jsonMessage->get_map()["userID"] = sio::string_message::create(userID);

            namespaceSocket->emit("connectplayer", jsonMessage, [&](sio::message::list const& ack_msg) {

                isDenateOnlineServiceConnected = true;
                std::cout << "Establishing Denate Online Service Connection" << std::endl;

                });

            std::this_thread::sleep_for(std::chrono::seconds(2));

            /*while (!sioClient.opened()) 
            { 
                std::this_thread::sleep_for(std::chrono::seconds(1)); 
            }*/

        }
        
        std::cout << "Denate Connection Established" << std::endl;

    }


    void DOS_Connection::OnDenateOnlineServiceConnected(onlineServiceConnectedListener const& onlineServiceConnected)
    {
        if (onlineServiceConnected)
        {
            internalOnlineserviceConnected = onlineServiceConnected;
        }
    }

    void DOS_Connection::OnDenateOnlineServiceDisconnected(onlineServiceDisconnectedListener const& onlineServiceDisconnected)
    {
        if (onlineServiceDisconnected)
        {
            internalOnlineServiceDisconnected = onlineServiceDisconnected;
        }
    }

    void DOS_Connection::OnInvitationRecieved(invitationRecievedListener const& invitationRecieved)
    {
        if (invitationRecieved)
        {
            internalInvitationRecieved = invitationRecieved;
        }
        
    }

    void DOS_Connection::OnFriendRequestRecieved(friendRequestRecievedListener const& friendRequestRecieved)
    {
        if (friendRequestRecieved)
        {
            internalFriendRequestRecieved = friendRequestRecieved;
        }
    }

    void DOS_Connection::OnFriendRequestDenied(friendRequestDeniedListener const& friendRequestDenied)
    {
        if (friendRequestDenied)
        {
            internalFriendRequestDenied = friendRequestDenied;
        }
    }

    void DOS_Connection::OnFriendRequestAccepted(friendRequestAcceptedListener const& friendRequestAccepted)
    {
        if (friendRequestAccepted)
        {
            internalFriendRequestAccepted = friendRequestAccepted;
        }
    }


}