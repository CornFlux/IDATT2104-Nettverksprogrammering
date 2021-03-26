#pragma once

#include <vector>
#include <iostream>
#include "STUNMessageAttribute.cpp"



//These are med up of the C1 and C0 bits in the 'message type field' 
#define MESSAGE_TYPE_CLASS_REQUEST = 0b00000000000000;
#define MESSAGE_TYPE_CLASS_INDICATION = 0b00000000010000;
#define MESSAGE_TYPE_CLASS_SUCCESS_RESPONSE = 0b00000100000000;
#define MESSAGE_TYPE_CLASS_ERROR_RESPONSE = 0b00000100010000;

#define MESSAGE_TYPE_METHOD_BINDING = 0b00000000000001;

#define MAGIC_COOKIE = 0x2112A442;

class STUNMessage
{
    private:
        std::vector<__uint8_t> transactionID; //MUST be cryptographically random
        unsigned short messageLength; //This always end in 00 (two zero bits at the end) because of the 4 byte padding on STUN attributes. 576 - 20 (header) - 8 (UDP) bytes is the max length if MTU (Maximum transmission unit) is unknown
        __uint16_t messageClass, messageMethod;
        std::vector<STUNMessageAttribute> attributes;
    public:
        enum MessageClass { request, indication, successResponse, errorResponse };
        enum MessageMethod { binding };

    public:
        STUNMessage();
        STUNMessage(std::vector<__uint8_t> transactionID, unsigned short messageLength, MessageClass messageClass, MessageMethod messageMethod);
        ~STUNMessage();
        std::vector<__uint8_t> getMessageInBytes();
        void addAttribute(STUNMessageAttribute attribute);
};
