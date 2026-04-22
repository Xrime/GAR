#include <iostream>
#include <curl/curl.h>
#include <string>
#include <memory>
#include "../include/anonymity/tor_manager.h"
#include "../include/anonymity/tor_connector.h"
#include "core/http_client.h"
#include <vector>
#include  <thread>
#include <chrono>

// void initializeTor() {
//     std::cout<<"Initializing Tor Connection..."<<std::endl;
//
// }
// void startTerminalUI() {
//     std::cout<<"Starting Terminal UI..."<<std::endl;
// }

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
    std::cout<<"Connected to Tor successfully!!"<<std::endl;
    std::cout<<"SOCKS5 Address:"<< tor_connector ->getSocks5Address()<<std::endl;
    std::cout<< "Connection Status: "<<(tor_connector->isConnected() ? "connected": "Disconnnected")<<std::endl;
    std::cout<<"\n";
// i must note this next time whenever i am creating something like this
    std::this_thread::sleep_for(std::chrono::seconds(10));

    std::cout << "Step 3: Making anonymous HTTP requests...\n" << std::endl;

    gar::core::HttpClient http_client("127.0.0.1", 9050);

    std::vector<std::string> test_urls = {
         "http://httpbin.org/get",
         "http://ipv4.icanhazip.com/",
        "http://www.google.com",
        "http://example.com"
    };

    for (const auto& url : test_urls) {
        std::cout << "Testing: " << url << std::endl;

        gar::core::HttpResponse response = http_client.get(url);

        if (response.success) {
            std::cout << " SUCCESS!" << std::endl;
            std::cout << "Status: " << response.status_code << std::endl;
            std::cout << "Body size: " << response.body.length() << " bytes" << std::endl;

            if (response.body.length() > 0) {
                std::string preview = response.body.substr(0, 50);
                std::cout << "Preview: " << preview << "..." << std::endl;
            }
            break;  // Found a working URL!
        } else {
            std::cout << " FAILED - " << response.error_message << std::endl;
        }

    // std::cout<<"Making an httprequest..."<<std::endl<<std::endl;
    //
    // gar::core::HttpClient http_client("127.0.0.1", 9050);

    // std::cout<<"Requesting to example.com"<<std::endl;
    //
    // gar::core::HttpResponse response1 =http_client.get("http:example.com");
    // if (response1.success) {
    //     std::cout<<"request successful"<<std::endl;
    //     std::cout<<"Status"<<response1.status_code<<" "<<response1.status_message<<std::endl;
    //     std::cout<<"Response size:"<<response1.body.length()<<"bytes"<<std::endl;
    //
    //     if (response1.body.length()>0) {
    //         std::string preview = response1.body.substr(0,100);
    //         for (char c :preview) {
    //             if (c =='\n' || c=='r') {
    //                 std::cout<<" ";
    //
    //             }else {
    //                 std::cout<<c;
    //             }
    //         }
    //     } else
    //     {
    //         std::cout<<"request failed"<<std::endl;
    //         std::cout<< "Error: "<<response1.error_message<<"\n"<<std::endl;
    //     }
    //     std::cout<<"Checking IP address through Tor"<<std::endl;

    // gar::core::HttpResponse response2 =http_client.get("http://icanhazip.com");
    //
    // if (response2.success) {
    //     std::cout<<"IP check successful!"<<std::endl;
    //     std::cout<<"Your IP through tor :"<<response2.body<<std::endl;
    // }else {
    //     std::cout<<"Error : "<<response2.error_message<<"\n"<<std::endl;
    //
    // }
    // std::map<std::string, std::string> post_headers;
    // post_headers["Content-type"] = "application/x-www-form-urlencoded";

    // gar::core::HttpResponse response3 = http_client.post(
    //     "http://httpbin.org/post",
    //     "username=test&password=secret",post_headers);
    //
    // if (response3.success) {
    //     std::cout<<"Status: "<<response3.status_code<<" "<<response3.status_message<<std::endl;
    //     std::cout<<"response size "<<response3.body.length()<<"bytes"<<std::endl;
    // }else {
    //     std::cout<<"Error"<<response3.error_message<<std::endl;
    // }
    std::cout << "====================================================" << std::endl;
    std::cout << " GAR is running with Tor successfully!" << std::endl;
    std::cout << "====================================================" << std::endl;
    std::cout<<"\n";
}
    return 0;
}

    //
    // initializeTor();
    //
    // startTerminalUI();
    // std::cout<<"GAR Shutting Down..."<<std::endl;
    // return 0;