#include <chrono>
#include <iostream>
#include <vector>


#include "STUNServer.h"
#include "STUNMessage.cpp"

std::vector<__uint8_t> udpBuffer(20 * 1024);

STUNServer::STUNServer(std::string address, unsigned short port)
    : running(false), addressStringIPv4(address), port(port)
{
    this->cacheCounter = 0; //The cache starts at value zero
    this->cacheSize = 20; //Sets cache size to 20 entries, hardcoded for now
    this->cachedRequests.reserve(this->cacheSize);
    for (int i = 0; i < this->cacheSize; i++)
    {
        this->cachedRequests.emplace_back(CacheRegister(false)); //Fills the cache with dummy data
    }
}

STUNServer::~STUNServer()
{
    stop();
}

bool STUNServer::start()
{
    //TODO: implement threads that run and listen to UDP IPv6, TCP IPv4 and IPv6 aswell
    this->running = true;
    this->socket.open(asio::ip::udp::v4());
    this->socket.bind(asio::ip::udp::endpoint(asio::ip::address::from_string(this->addressStringIPv4), this->port));
    asio::io_context::work idleWork(eventLoop);
    this->threadContext = std::thread([&]()
    {
        this->eventLoop.run();
    });

    if (this->socket.is_open())
    {
        std::cout << "Socket is open!" << std::endl;
        
        listenForClientUDP(this->socket); //Starts an infinite loop of waiting for data on UDP
    }
    else
    {
        return false;
    }
    return true;
}

void STUNServer::stop()
{
    std::cout << "Server stopped!" << std::endl;
    this->running = false;
}

bool STUNServer::isRunning()
{
    return this->running;
}

void STUNServer::listenForClientUDP(asio::ip::udp::socket& socket)
{
    socket.async_receive_from(asio::buffer(udpBuffer.data(), udpBuffer.size()), this->sendersEndpointUDPIPv4,
    [&](std::error_code errorCode, std::size_t length)
    {
        if (!errorCode)
        {
            std::cout << "\n\nRead " << length << " bytes\n\n";

            std::thread t([this, length]
            {
                STUNServer::handleMessage(udpBuffer, length); //Handles the message in anthor thread, so this one can return to listen for more incomeing messages
            });

            listenForClientUDP(socket); //This returns immediately
            t.join();
        }
        else
        {
            std::cout << errorCode.message() << std::endl;
        }
    });
}

void STUNServer::handleMessage(std::vector<__uint8_t> messageBuffer, std::size_t length)
{
    unsigned short messageLength;
    for (int i = 0; i < length; i++)
    {
        //Used for testing
        printf("\\x%.2x", messageBuffer[i]); //hex print this is for testing
        //printf("%d", messageBuffer[i]); //decimal print
    }

    if (length < 20)
    {
        std::cout << "Header MUST be 20 bytes, not a STUN message!" << std::endl;
    }
    //TODO: Implement so the server accepts more types of messages from clients
    else if (isRequestBinding(messageBuffer)) //Checking if it is a STUN binding request 
    {
        //Read the length of the message
        messageLength = ((messageBuffer[2] << 8) | messageBuffer[3]);
        printf("\nMessage length: %d \n", messageLength);

        if (messageLength == 0) //There are no attributes
        {
            sendAnswerToClient(messageBuffer, messageLength, STUNMessage::MessageClass::successResponse);
        }
        else
        {
            //TODO: implement handling of attributes
        }   
    }
    else
    {
        std::cout << "Other type of message" << std::endl;
    }
}

bool STUNServer::isRequestBinding(std::vector<__uint8_t>& messageBuffer)
{
    //Check if the first two bits are zero
    if ((messageBuffer[0] & 0b11000000) != 0b00000000)
    {
        return false;
    }
    //Check if it is a binding method
    else if ((messageBuffer[0] & 0b00111110) != 0b00000000 && (messageBuffer[1] & 0b11101111) != 0b00000001)
    {
        return false;
    }
    //Check if it is a request class
    //TODO: Accept indications 
    else if ((messageBuffer[0] & 0b00000001) != 0b00000000 || (messageBuffer[1] & 0b00010000) != 0b00000000)
    {
        return false;
    }
    //Check if the magic cookie is the right value (0x2112A442)
    else
    {
        if (messageBuffer[4] != 0x21
            || messageBuffer[5] != 0x12
            || messageBuffer[6] != 0xa4
            || messageBuffer[7] != 0x42)
        {
            return false;
        }
    }
    return true;
}

void STUNServer::sendAnswerToClient(std::vector<__uint8_t>& messageBuffer, std::size_t length, STUNMessage::MessageClass messageClass)
{
    bool addToCache = false;
    switch (messageClass)
    {
        case STUNMessage::MessageClass::successResponse:
        {
            std::vector<__uint8_t> transactionID;
            
            //The transaction ID is 96bit = 12 byte long, it starts at the 9th byte, the loop reads it into the transactionID vector
            for (int i = 8; i < 20; i++)
            {
                transactionID.emplace_back(messageBuffer[i]);
            }
            
            CacheRegister cache = checkCache(transactionID);
            if (cache.isValid())
            {
                this->socket.send_to(asio::buffer(cache.getResponse().getMessageInBytes().data(), cache.getResponse().getMessageInBytes().size()), this->sendersEndpointUDPIPv4);
                std::cout << "Found in cache!" << std::endl;
                break;
            }
            addToCache = true;

            if (length == 0)
            {
                STUNMessage response((std::vector<__uint8_t>)transactionID , (unsigned short)0, STUNMessage::MessageClass::successResponse, STUNMessage::MessageMethod::binding);
                //TODO: add a SOFTWARE attribute to the response

                std::cout << "Socket address: " << this->sendersEndpointUDPIPv4.address() << "\nSocket port: " << this->sendersEndpointUDPIPv4.port() << std::endl; //Testing

                
                __uint128_t address;
                asio::ip::address_v4::bytes_type addressbytes;
                if (this->sendersEndpointUDPIPv4.address().is_v4())
                {
                    for (auto byte : this->sendersEndpointUDPIPv4.address().to_v4().to_bytes())
                    {
                        address = (address << 8) | byte;    
                    }
                }
                else if (this->sendersEndpointUDPIPv4.address().is_v6())
                {
                    for (auto byte : this->sendersEndpointUDPIPv4.address().to_v6().to_bytes())
                    {
                        address = (address << 8) | byte;    
                    }
                }

                //Add attributes (just XOR-MAPPED-ADDRESS for now)
                STUNMessageAttribute xorMappedAddress;
                xorMappedAddress.setType(STUNMessageAttribute::Attribute::XOR_MAPPED_ADDRESS);
                std::vector<__uint8_t> value = STUNMessageAttribute::generateXORMAPPEDADDRESValue(this->sendersEndpointUDPIPv4, (__uint16_t)this->sendersEndpointUDPIPv4.port(), address);
                if (!value.empty()) //If this is empty that means something in the attribute generating went wrong
                {
                    xorMappedAddress.setValue(value);
                    response.addAttribute(xorMappedAddress);
                }

                //Send the answer to the client
                //this->socket.async_send_to(asio::buffer(response.getMessageInBytes().data(), response.getMessageInBytes().size()), sendersEndpointUDPIPv4);
                this->socket.send_to(asio::buffer(response.getMessageInBytes().data(), response.getMessageInBytes().size()), this->sendersEndpointUDPIPv4);
                CacheRegister newCache(transactionID, response); //Creating new cache entry
                addCacheEntry(newCache); //Adding new cache entry
            }
            else
            {
                //TODO: Implement handling of attributes
            }
            break;
        }

        case STUNMessage::MessageClass::errorResponse:
        {
            //TODO: Implemnet the error response, this one MUST have the ERROR-CODE attribute with the given error code
            break;
        }

        case STUNMessage::MessageClass::indication:
            //TODO: Implement logic for sending indications
            break;

        default:
            break;
    }
}

CacheRegister STUNServer::checkCache(std::vector<__uint8_t>& transactionID)
{
    for (CacheRegister cache : this->cachedRequests)
    {
        if (cache.getTransactionID() == transactionID)
        {
            return cache; //If there is a cache hit return the cache entry
        }
    }
    return CacheRegister(false); //If there is a cache miss, return a dummy cache entry with 'valid' set to false
}

void STUNServer::addCacheEntry(CacheRegister newCache)
{
    //TODO: This is only a concept cache, a real one would have a spizified size and an algorithem to decide which element to replace
    if (this->cacheCounter >= this->cacheSize)
    {
        this->cacheCounter = 0; //restes it, so it doesnt go out of bound
    }
    this->cachedRequests.at(this->cacheCounter++) = newCache;
}