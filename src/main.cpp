#include <iostream>
#include <curl/curl.h>
#include <string>
#include <memory>
#include "../include/anonymity/tor_manager.h"
#include "../include/anonymity/tor_connector.h"
#include "../include/ui/terminal_ui.h"
#include "core/http_client.h"
#include <vector>
#include  <thread>
#include <chrono>
#include "../include/anonymity/secure_memory.h"
#include  <windows.h>

// void initializeTor() {
//     std::cout<<"Initializing Tor Connection..."<<std::endl;
//
// }
// void startTerminalUI() {
//     std::cout<<"Starting Terminal UI..."<<std::endl;
// }

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
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
    std::cout<<"Connected to Tor successfully!!"<<std::endl;
    std::cout<<"SOCKS5 Address:"<< tor_connector ->getSocks5Address()<<std::endl;
    std::cout<< "Connection Status: "<<(tor_connector->isConnected() ? "connected": "Disconnnected")<<std::endl;
    std::cout<<"\n";
// i must note this next time whenever i am creating something like this
    std::this_thread::sleep_for(std::chrono::seconds(10));

    std::cout << "Step 3: Making anonymous HTTP requests...\n" << std::endl;

    gar::core::HttpClient http_client("127.0.0.1", 9050);

    gar::terminal_ui::TerminalUI ui(http_client);
    ui.run();

    std::cout << "====================================================" << std::endl;
    std::cout << " GAR is running with Tor successfully!" << std::endl;
    std::cout << "====================================================" << std::endl;
    std::cout<<"\n";

    gar::secure_memory::Securebuffer buf(32);
    memcpy(buf.data(), "SECRET-TOKEN-123",17);

    gar::secure_memory::SecureString s ("my_password");
    s.set("new_password");
    return 0;
}