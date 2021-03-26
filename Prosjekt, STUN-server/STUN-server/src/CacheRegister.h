#pragma once

#include <iostream>
#include <vector>


#include "STUNMessage.cpp"



class CacheRegister
{
private:
    std::vector<__uint8_t> transactionID;
    STUNMessage response;
    bool valid;
public:
    CacheRegister(bool valid);
    CacheRegister(std::vector<__uint8_t> transactionID,  STUNMessage response);
    ~CacheRegister();
    std::vector<__uint8_t> getTransactionID();
    bool isValid();
    STUNMessage getResponse();
};