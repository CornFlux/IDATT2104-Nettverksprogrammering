#pragma once

#include <asio.hpp>
#include <thread>

#include "STUNMessage.cpp"
#include "CacheRegister.cpp"

#define IPADDRESS "0.0.0.0"
#define UDP_PORT 8000

class STUNServer
{
    private:
        std::string addressStringIPv4;
        unsigned short port;
        bool running;
        short cacheSize, cacheCounter;
        asio::error_code errorCode;
        asio::io_context eventLoop;
        std::thread threadContext;
        asio::ip::udp::socket socket{eventLoop};
        asio::ip::address_v4 addressIPv4();
        asio::ip::udp::endpoint sendersEndpointUDPIPv4;
        std::vector<CacheRegister> cachedRequests;


        //asio::ip::tcp::acceptor acceptor; //Setter TCP/IP på vent da ICE https://developer.mozilla.org/en-US/docs/Glossary/ICE bruker UDP når den bruker STUN serveren
        
    public:
        STUNServer(std::string address, unsigned short port);
        ~STUNServer();
        bool start();
        void stop();
        bool isRunning();
        void listenForClientUDP(asio::ip::udp::socket& socket);
        void handleMessage(std::vector<__uint8_t> messageBuffer, std::size_t length);
        bool isRequestBinding(std::vector<__uint8_t>& messageBuffer);
        void sendAnswerToClient(std::vector<__uint8_t>& messageBuffer, std::size_t length, STUNMessage::MessageClass messageClass);
        CacheRegister checkCache(std::vector<__uint8_t>& transactionID);
        void addCacheEntry(CacheRegister newCache);
};
