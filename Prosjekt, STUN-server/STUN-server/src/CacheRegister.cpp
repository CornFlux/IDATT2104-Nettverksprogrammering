#include "CacheRegister.h"


CacheRegister::CacheRegister(bool valid)
    : valid(valid)
{
}

CacheRegister::CacheRegister(std::vector<__uint8_t> transactionID,  STUNMessage response)
    : transactionID(transactionID), response(response), valid(true)
{

}

CacheRegister::~CacheRegister()
{
}


std::vector<__uint8_t> CacheRegister::getTransactionID()
{
    return this->transactionID;
}

bool CacheRegister::isValid()
{
    return this->valid;
}

STUNMessage CacheRegister::getResponse()
{
    return this->response;
}