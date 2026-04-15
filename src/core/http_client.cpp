#include "../../cmake-build-debug/include/core/http_client.h"
// #include "core/http_client"


namespace gar::core {
    HttpClient::HttpClient(std::unique_ptr<IHttpTransport> transport)
        :transport_(std::move(transport)){}
    HttpResponse HttpClient::get(const std::string &url) {
        HttpRequest req;
        req.url = url;
        req.method ="GET";
        return transport_->performRequest(req);
    }
    HttpResponse HttpClient::post(const std::string &url, std::string &body, const std::map<std::string, std::string> &header) {
        HttpRequest req;
        req.url = url;
        req.method ="POST";
        req.body =body;
        return transport_->performRequest(req);
    }


}
