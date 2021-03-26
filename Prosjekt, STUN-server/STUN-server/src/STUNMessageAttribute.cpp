#include "STUNMessageAttribute.h"

__int16_t magicCookieTop16 = 0x2112;
__int32_t magicCookie = 0x2112A442;

STUNMessageAttribute::STUNMessageAttribute()
{

}

STUNMessageAttribute::~STUNMessageAttribute()
{
}

std::vector<__uint8_t> STUNMessageAttribute::getInBytes()
{
    std::vector<__uint8_t> attributeBytes;

    //Setting the Type
    attributeBytes.emplace_back((__uint8_t)(this->type >> 8));
    attributeBytes.emplace_back((__uint8_t)this->type);

    //Setting the Length
    __uint16_t length = this->value.size();
    attributeBytes.emplace_back((__uint8_t)(length >> 8));
    attributeBytes.emplace_back((__uint8_t)length);

    //Setting the Value
    for (auto byte : value)
    {
        attributeBytes.emplace_back(byte);
    }
    while (attributeBytes.size() % 4 != 0) //Adding the padding required by the attribute definition; each attribute has a total size equal to a multiple of 4 bytes
    {
        attributeBytes.emplace_back(0); //The padding can be any value, it is ignored by the client, here it is just zeroes
    }
    //Finished setting the Value

    return attributeBytes;
}

void STUNMessageAttribute::setType(Attribute attribute)
{
    this->attribute = attribute;
    switch (attribute)
    {
    case Attribute::MAPPED_ADDRESS:
        this->type = 0x0001; //0x0001: MAPPED-ADDRESS
        break;
    case Attribute::XOR_MAPPED_ADDRESS:
        this->type = 0x0020; //0x0020: XOR-MAPPED-ADDRESS
        break;
    case Attribute::USERNAME:
        this->type = 0x0006; //0x0006: USERNAME
        break;
    case Attribute::MESSAGE_INTEGRITY:
        this->type = 0x0008; //0x0008: MESSAGE-INTEGRITY
        break;
    case Attribute::FINGERPRINT:
        this->type = 0x8028; //0x8028: FINGERPRINT
        break;
    case Attribute::ERROR_CODE:
        this->type = 0x0009; //0x0009: ERROR-CODE
        break;
    case Attribute::REALM:
        this->type = 0x0014; //0x0014: REALM
        break;
    case Attribute::NONCE:
        this->type = 0x0015; //0x0015: NONCE
        break;
    case Attribute::UNKNOWN_ATTRIBUTES:
        this->type = 0x000A; //0x000A: UNKNOWN-ATTRIBUTES
        break;
    case Attribute::SOFTWARE:
        this->type = 0x8022; //0x8022: SOFTWARE
        break;
    case Attribute::ALTERNATE_SERVER:
        this->type = 0x8023; //0x8023: ALTERNATE-SERVER
        break;
    default:
        break;
    }
}

void STUNMessageAttribute::setValue(std::vector<__uint8_t> value)
{
    this->value = value;
}


std::vector<__uint8_t> STUNMessageAttribute::generateXORMAPPEDADDRESValue(asio::ip::udp::endpoint family, __uint16_t port, __uint128_t address)
{
    std::vector<__uint8_t> attributeBytes;   
    int j = 0;
    attributeBytes.emplace_back(0); //Starts with 8bit set to zero
    if (family.address().is_v4()) //IPv4: 0x01:IPv4
    {
        attributeBytes.emplace_back(0x01); //Familiy
        j = 3;
    }
    else if (family.address().is_v6()) //IPv4: 0x02:IPv6
    {
        attributeBytes.emplace_back(0x02); //Familiy 
        j = 15;
    }
    else
    {
        //It's neither IPv4 nor IPv6
        attributeBytes.clear(); //Returning an empty vector to handle non implemented values, this way the server doesn't crash
        return attributeBytes;
    }
    
    attributeBytes.emplace_back((__uint8_t)((port >> 8) ^ (magicCookieTop16 >> 8))); //Port
    attributeBytes.emplace_back((__uint8_t)(port ^ magicCookieTop16)); //Port

    for (int i = j; i >= 0; i--)
    {
        attributeBytes.emplace_back((__uint8_t)((address >> (i * 8))) ^ (magicCookie >> (i * 8))); //Address
    } 
    return attributeBytes;
}


std::vector<__uint8_t> STUNMessageAttribute::generateMAPPEDADDRESValue(asio::ip::udp::endpoint family, __uint16_t port, __uint128_t address)
{
    std::vector<__uint8_t> attributeBytes;
    int j = 0;
    attributeBytes.emplace_back(0); //Starts with 8bit set to zero
    if (family.address().is_v4()) //IPv4: 0x01:IPv4
    {
        attributeBytes.emplace_back(0x01); //Familiy
        j = 3;
    }
    else if (family.address().is_v6()) //IPv4: 0x02:IPv6
    {
        attributeBytes.emplace_back(0x02); //Familiy 
        j = 15;
    }
    else
    {
        //It's neither IPv4 nor IPv6
        attributeBytes.clear(); //Returning an empty vector to handle non implemented values, this way the server doesn't crash
        return attributeBytes;
    }
    
    attributeBytes.emplace_back((__uint8_t)(port >> 8)); //Port
    attributeBytes.emplace_back((__uint8_t)(port)); //Port

    for (int i = j; i >= 0; i--)
    {
        attributeBytes.emplace_back((__uint8_t)((address >> (i * 8)))); //Address
    }
    return attributeBytes;
}

std::vector<__uint8_t> STUNMessageAttribute::generateERRORCODEValue(ErrorClass errorClass, ErrorNumber errorNumber)
{
    std::vector<__uint8_t> attributeBytes;
    attributeBytes.emplace_back(0); //The first 21 bit SHOULD be zero
    attributeBytes.emplace_back(0); //Setting the first 16, the remaining 5 comes with the error class

    attributeBytes.emplace_back(errorClass); //Adding the error class
    attributeBytes.emplace_back(errorNumber); //Adding the error number
    std::string message;
    switch (((__uint16_t)errorClass << 8) | errorNumber)
    {
    case 300: //Try Alternate
        message = "The client should contact an alternate server for this request.";
        break;
    case 400: //Bad Request
        message = "The request was malformed.";
        break;
    case 401: //Unauthorized
        message = "The request did not contain the correct credentials to proceed.";
        break;
    case 420: //Unknown Attribute
        message = "The server received a STUN packet containing a comprehension-required attribute that it did not understand.";
        break;
    case 438: //Stale Nonce
        message = "The NONCE used by the client was no longer valid.";
        break;
    case 500: //Server Error
        message = "The server has suffered a temporary error.";
        break;
    default:
        message = "Not valid.";
        break;
    }

    if (message != "Not valid.")
    {
        std::vector<__uint8_t> messageBytes(message.begin(), message.end());
        for (auto byte : messageBytes)
        {
            attributeBytes.emplace_back(byte);
        }
    }
    return attributeBytes;
}