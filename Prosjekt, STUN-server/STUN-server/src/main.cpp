//g++ main.cpp -o main -pthreads -std=c++1y && ./main
#include <chrono>
#include <iostream>
#include <vector>
#include <asio.hpp>

#include "STUNServer.cpp"


int main()
{   
    std::cout << "Starting..." << std::endl;

    //STUNServer server("127.0.0.1", 8000); //Local test IP
    STUNServer server("192.168.0.125", 8000);
    if (server.start())
    {
        std::cout << "Server started!" << std::endl;
    }
    else
    {
        std::cout << "Server failed to start!" << std::endl;
    }

    while (server.isRunning())
    {
        //Looping till the server is stopped
    }

    return 0;
}