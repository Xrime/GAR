//
// Created by xint2 on 12/05/2026.
//
#include "../../include/security/tls_inspector.h"
#include <winsock.h>
#include <ws2tcpip.h>
#include <openssl/ssl.h>
#include  <openssl/x509.h>
#include <openssl/asn1.h>
#include <sstream>
#include <vector>

namespace gar::security {
    static::std::string asn1_time_to_string(const ASN1_TIME* t) {
        BIO* bio = BIO_new(BIO_s_mem());
        ASN1_TIME_print(bio, t);
        char* data = nullptr;
        long len = BIO_get_mem_data(bio, &data);
        std::string out(data, len);
        BIO_free(bio);
        return out;
    }
    static int socks5_connect(const std::string& socks_host, int socks_port, const std::string& dst_host, int dst_port,std::string& err) {
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
            err = "WSAStartup failed";
            return -1;
        }
        SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (s == INVALID_SOCKET) {
            err = "socket failed";
            return -1;
        }
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons((u_short)socks_port);
        inet_pton(AF_INET, socks_host.c_str(), &addr.sin_addr);

        if (connect(s, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            err = "connect to SOCKS FAILED";
            closesocket(s);
            return -1;

        }
        unsigned char greet[3] = {0x05,0x01,0x00};
        send(s,(const char*)greet, 3,0);
        unsigned char resp[2];
        recv(s,(char*) resp, 2,0);
        if (resp[1] != 0x00) {
            err = "socks auth failed";
            closesocket(s);
            return -1;
        }
        std::vector<unsigned char> req;
        req.push_back(0x05);
        req.push_back(0x01);
        req.push_back(0x00);
        req.push_back(0x03);
        req.push_back((unsigned char)dst_host.size());
        for (char c : dst_host) req.push_back((unsigned char)c);
        req.push_back((dst_port>> 8) & 0xFF);
        req.push_back(dst_port & 0xFF);

        send(s, (const char*)req.data(), (int)req.size(), 0);

        unsigned char rep[10]{0};
        recv(s, (char*)rep,10,0);
        if (rep[1] != 0x00) {
            err = "SOCKS connect failed";
            closesocket(s);
            return -1;
        }
        return (int)s;



    }
    TLSReport TLSInspector::inspect(const std::string &host, int port, const std::string &socks_host, int socks_port) {
        TLSReport r{};
        r.success = false;
        std::string err;
        int sock = socks5_connect(socks_host, socks_port, host, port,err);
        if (sock<0) {
            r.error = err;
            return r;
        }

        SSL_library_init();
        SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
        if (!ctx) {
            r.error = "SSL_CTX failed";
            closesocket((SOCKET)sock);
            return r;
        }
        SSL* ssl = SSL_new(ctx);
        SSL_set_tlsext_host_name(ssl, host.c_str());
        SSL_set_fd(ssl,sock);

        if (SSL_connect(ssl) !=1) {
            r.error = "SSL_connect failed";
            SSL_CTX_free(ctx);
            closesocket((SOCKET)sock);
            return r;
        }
        X509* cert = SSL_get_peer_certificate(ssl);
        if (!cert) {
            r.error = "No certificate";
        }else {
            char buf[1024];

            X509_NAME_oneline(X509_get_subject_name(cert), buf,sizeof(buf));
            r.subject = buf;
            X509_NAME_oneline(X509_get_issuer_name(cert), buf, sizeof(buf));
            r.issuer = buf;

            ASN1_TIME* nb = X509_get_notBefore(cert);
            ASN1_TIME* na = X509_get_notAfter(cert);
            r.not_before = asn1_time_to_string(nb);
            r.not_after = asn1_time_to_string(na);

            int days =0;
            int secs = 0;
            ASN1_TIME_diff(&days, &secs, nullptr, na);
            r.days_left = days;
            r.success = true;
            X509_free(cert);
        }
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        closesocket((SOCKET)sock);
        return r;
    }

}
