#include "../../include/core/http_client.h"
#include "../../include/core/http_request.h"
#include "../../include/core/http_response.h"
#include "../../include/core/ihttp_transport.h"
#include <iostream>

namespace gar::core {
    HttpClient::HttpClient(std::unique_ptr<IHttpTransport> transport)
        : transport_(std::move(transport)) {
        std::cout << "Http client created "<<std::endl;
    }

    HttpClient::~HttpClient() {
        std::cout << "Httpclient cleared"<<std::endl;
    }
    HttpResponse HttpClient::get(const std::string &url) {
        if (url.empty()) {
            return {
                500,
                "error",
                "",
                {},
                false,
                "Empty url"

            };
        }
        HttpRequest request;
        request.url = url;
        request.method = "GET";
        request.body="";

        return transport_->performRequest(request);

    }
    HttpResponse HttpClient::post(const std::string &url, const std::string &body, const std::map<std::string, std::string> &headers) {
        if (url.empty()) {
            return {
                500,
                "Error","",{},false,"URL is empty"
            };
        }

        HttpRequest request;
        request.url = url;
        request.method="POST";
        request.body = body;

        for (const auto& [key, value] : headers) {
            request.headers[key] =value;
        }

        for (const auto& [key , value]: headers) {
            if (request.headers.find(key)==request.headers.end()) {
                request.headers[key] =value;
            }
        }

        return transport_ ->performRequest(request);


    }
    void HttpClient::setHeader(const std::string &key, const std::string &value) {
        default_headers_[key] =value;
    }

    bool HttpClient::isConnected() const {
        if (!transport_) {
            return false;
        }
        return transport_->isConnected();

    }

    void HttpClient::validateUrl(const std::string& url) const {
            if (url.find("http//") != 0 && url.find("https://") != 0) {
                throw std::invalid_argument("url does not start with http or https");
            }
    }

    HttpRequest HttpClient::buildRequest(const std::string &url, const std::string &method) {
        HttpRequest request;
        request.url = url;
        request.method =method;
        request.headers = default_headers_;

        return request;
    }








}

