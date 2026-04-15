#include <iostream>
#include <curl/curl.h>
#include <string>


void initializeTor() {
    std::cout<<"Initializing Tor Connection..."<<std::endl;

}
void startTerminalUI() {
    std::cout<<"Starting Terminal UI..."<<std::endl;
}

int main() {
    std::cout<<"==============================="<<std::endl;
    std::cout<<"  GAR -Ghost Anonymous Router"<<std::endl;
    std::cout<<"==============================="<<std::endl;



    initializeTor();

    startTerminalUI();

    std::cout<<"GAR Shutting Down..."<<std::endl;
    return 0;
}
