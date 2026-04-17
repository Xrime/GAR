#include <iostream>
#include <curl/curl.h>
#include <string>
#include <memory>
#include "../include/anonymity/tor_manager.h"
#include "../include/anonymity/tor_connector.h"


void initializeTor() {
    std::cout<<"Initializing Tor Connection..."<<std::endl;

}
void startTerminalUI() {
    std::cout<<"Starting Terminal UI..."<<std::endl;
}

int main() {
    //welcome
    std::cout << "\n";
    std::cout << "#====================================================#" << std::endl;
    std::cout << "#            GAR - Ghost Anonymous Router            #" << std::endl;
    std::cout << "#                                                    #" << std::endl;
    std::cout << "#====================================================#" << std::endl;
    std::cout << "\n";

    // //create Tor connector
    // std::cout<<"{Main} Testing Tor connection..."<<std::endl;
    // std::cout << "{Main} ";

    std::cout<<"1: Starting embedded Tor...\n"<<std::endl;

    auto tor_manager = std::make_unique<gar::anonymity::TorManager>();

    if (!tor_manager->startTor()) {
        std::cout<<"\n";
        std::cout<<"Failed : Could not start Tor"<<std::endl;
        std::cout<<"Error :"<<tor_manager ->getLastError()<<std::endl;
        std::cout<<"\n";
        std::cout<<"Troubleshooting:"<<std::endl;
        std::cout<<"1. Check antivirus isn't blocking Tor"<<std::endl;
        std::cout<<"\n";
        return 1;
    }
    std::cout<<"\n";
    std::cout<<"Tor started successfully!!!"<<std::endl;
    std::cout<<"\n";


    std::cout << "2: Connecting through Tor...\n"<<std::endl;

    auto tor_connector = std::make_unique<gar::anonymity::TorConnector>("127.0.0.1",9050);

    if (!tor_connector->testconnection()) {
        std::cout<<"\n";
        std::cout<<"Failed: Could not connect to Tor"<< std::endl;
        std::cout<<"Error: "<<tor_connector->getLastError()<<std::endl;
        std::cout<<"\n";
        return 1; //to exit error code 1

    }
    std::cout<<"\n";
    std::cout<<"Connected tp Tor successfully!!"<<std::endl;
    std::cout<<"SOCKS5 Address:"<< tor_connector ->getSocks5Address()<<std::endl;
    std::cout<< "Connection Status: "<<(tor_connector->isConnected() ? "connected": "Disconnnected")<<std::endl;
    std::cout<<"\n";

    std::cout << "====================================================" << std::endl;
    std::cout << " GAR is running with Tor successfully!" << std::endl;
    std::cout << "====================================================" << std::endl;
    std::cout<<"\n";
    return 0;






    initializeTor();

    startTerminalUI();

    std::cout<<"GAR Shutting Down..."<<std::endl;
    return 0;
}
