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

    gar::terminal_ui::TerminalUI ui(http_client);
    ui.run();

    // std::vector<std::string> test_urls = {
    //     "https://api.ipify.org"
    //     // "https://www.google.com"
    //      // "https://httpbin.org/get"
    //     //  "http://ipv4.icanhazip.com/",
    //     // "http://example.com"
    // };
    //
    // std::string url= "https://httpbin.org/get";
    //
    // std::cout << "Testing: " <<  url << std::endl;
    //
    // gar::core::HttpResponse response = http_client.get(url);
    //
    // if (response.success) {
    //     std::cout << " SUCCESS!" << std::endl;
    //     std::cout << "Status: " << response.status_code << std::endl;
    //     std::cout << "Body size: " << response.body.length() << " bytes" << std::endl;
    //
    //     if (response.body.length() > 0) {
    //         std::string preview = response.body.substr(0, 50);
    //         std::cout << "Preview: " << preview << "..." << std::endl;
    //     }
    // }else {
    //         std::cout << " FAILED - " << response.error_message << std::endl;
    //     }
    // gar::core::HttpRequest post_request;
    // post_request.url = "https://httpbin.org/post";
    // post_request.method ="POST";
    // post_request.headers["HOST"] = "httpbin.org";
    // post_request.headers["User-Agent"] = "GAR";
    // post_request.headers["Accept"] = "*/*";
    // post_request.headers["Connection"]="close";
    // post_request.headers["Content-types"] = "application/json";
    // post_request.body = "{\"name\":\"gar\",\"mode\":\"tor\"}";;
    //
    // gar::core::HttpResponse post_response= http_client.performRequest(post_request);
    // if (post_response.success) {
    //     std::cout<<"Status: "<<post_response.status_code<<std::endl;
    //     std::cout<<"POST preview"<<post_response.body.substr(0,120)<< std::endl;
    // }else {
    //     std::cout<<"POST failed "<<post_response.error_message<<std::endl;
    // }

    std::cout << "====================================================" << std::endl;
    std::cout << " GAR is running with Tor successfully!" << std::endl;
    std::cout << "====================================================" << std::endl;
    std::cout<<"\n";
    return 0;
}