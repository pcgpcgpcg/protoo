#ifndef HTTP_TRANSPORT_H
#define HTTP_TRANSPORT_H

#include <cstring>
#include <string>
#include <iostream>
#include <thread>
#include <future>
#ifdef __cplusplus
extern "C"{
#include "libwebsockets.h"
#endif
#ifdef __cplusplus
}
#endif

struct ClientData {
    std::string path;
    std::string host;
    std::string postData;
};

class HTTPTransport {
    public:
    HTTPTransport();
    ~HTTPTransport();
    void sendPostRequest(std::string url, std::string path, std::string body);
    void sendGetRequest(const std::string& url);

    private:
    void runHTTP();

    private:
    struct lws_protocols m_protocols[2];
    std::thread *m_pWsThread;
    lws_context *m_context{nullptr};
    volatile bool m_stopped{false};
    lws* m_wsi;

};
#endif