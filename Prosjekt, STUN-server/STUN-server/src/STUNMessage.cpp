#pragma once

#include "STUNMessage.h"

#include <vector>
#include <iostream>

STUNMessage::STUNMessage()
{

}

STUNMessage::STUNMessage(std::vector<__uint8_t> transactionID, unsigned short messageLength, MessageClass messageClass, MessageMethod messageMethod)
    : transactionID(transactionID), messageLength(messageLength)
{
    switch (messageClass)
    {
    case MessageClass::successResponse:
    {
        this->messageClass = 0b0000000100000000;
        break;
    }
    case MessageClass::errorResponse:
    {
        this->messageClass = 0b0000000100010000;
        break;
    }
    case MessageClass::indication:
    {
        this->messageClass = 0b0000000000010000;
        break;
    }
    default:
        break;
    }

    switch (messageMethod)
    {
    case MessageMethod::binding:
    {
        this->messageMethod = 0b0000000000000001;
        break;
    }
    //TODO: Add more methods, if needed
    default:
        break;
    }
}

STUNMessage::~STUNMessage()
{
}

std::vector<__uint8_t> STUNMessage::getMessageInBytes()
{
    //TODO: return the STUN message in byte form
    std::vector<__uint8_t> bytes;
    __uint16_t messageTypeBytes = this->messageMethod | this->messageClass;
    bytes.emplace_back((__uint8_t)(messageTypeBytes >> 8));
    bytes.emplace_back((__uint8_t)messageTypeBytes);

    std::cout << bytes.at(0) << "  " << bytes.at(1) << std::endl;

    __uint16_t messageSizeBytes = 0;

    for (int i = 0; i < this->attributes.size(); i++)
    {
        //We assume that the attributs are self padded, and therefore can use the size of their byte array directly
        messageSizeBytes += this->attributes.at(i).getInBytes().size();
    }

    //Adding the length bytes 
    bytes.emplace_back((__uint8_t)(messageSizeBytes >> 8));
    bytes.emplace_back((__uint8_t)messageSizeBytes);


    //Adding the magic cookie
    __int32_t magicCookie = 0x2112A442;
    for (int i = 3; i >= 0; i--)
    {
        bytes.emplace_back((__uint8_t)(magicCookie >> i * 8));
    }

    //Adding the transaction ID
    for (auto byte : this->transactionID)
    {
        bytes.emplace_back(byte);
    }

    //Adding the attributes
    for (auto attribute : this->attributes)
    {
        for (auto byte : attribute.getInBytes())
        {
            bytes.emplace_back(byte);
        }
    }
    

    return bytes;
}

void STUNMessage::addAttribute(STUNMessageAttribute attribute)
{
    attributes.emplace_back(attribute);
}