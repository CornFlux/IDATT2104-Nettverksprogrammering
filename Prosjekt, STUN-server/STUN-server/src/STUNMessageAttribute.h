#pragma once

#include <iostream>
#include <vector>

class STUNMessageAttribute
{
private:
    __uint8_t attribute;
    __uint16_t type;
    std::vector<__uint8_t> value;

public:
    enum Attribute { MAPPED_ADDRESS, XOR_MAPPED_ADDRESS, USERNAME, MESSAGE_INTEGRITY, FINGERPRINT, ERROR_CODE, REALM, NONCE, UNKNOWN_ATTRIBUTES, SOFTWARE, ALTERNATE_SERVER };
    //All the ErrorClass values are the same, but if some other error-class, e.g. in the 600 range, were to be added this is the best setup for easy adding. The ErrorValue enum is commented out, but it shows the relationship between error class and error number
    enum ErrorClass { Try_Alternate_Class = 0b00000001, Bad_Request_Class = 0b00000001, Unauthorized_Class = 0b00000001, Unknown_Attribute_Class = 0b00000001, Stale_Nonce_Class = 0b00000001, Server_Error_Class = 0b00000001 };
    enum ErrorNumber { Try_Alternate = 0b00101100, Bad_Request = 0b10010000, Unauthorized = 0b10010001, Unknown_Attribute = 0b10100100, Stale_Nonce = 0b10110110, Server_Error = 0b11110100 };
    //enum ErrorValue { Try_Alternate = 0b0000000100101100/*300*/, Bad_Request = 0b0000000110010000/*400*/, Unauthorized = 0b0000000110010001/*401*/, Unknown_Attribute = 0b0000000110100100/*420*/, Stale_Nonce = 0b0000000110110110/*438*/, Server_Error = 0b0000000111110100/*500*/ };
public:
    //TODO: implementet so it is possible to take an IPv6 address
    STUNMessageAttribute();
    ~STUNMessageAttribute();
    std::vector<__uint8_t> getInBytes();
    void setType(Attribute attribute);
    void setValue(std::vector<__uint8_t> value);
    static std::vector<__uint8_t> generateXORMAPPEDADDRESValue(asio::ip::udp::endpoint family, __uint16_t port, __uint128_t address);
    static std::vector<__uint8_t> generateMAPPEDADDRESValue(asio::ip::udp::endpoint family, __uint16_t port, __uint128_t address);
    static std::vector<__uint8_t> generateERRORCODEValue(ErrorClass errorClass, ErrorNumber errorNumber);
};
