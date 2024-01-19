#ifndef WEBSOCKET_TRANSPORT_H
#define WEBSOCKET_TRANSPORT_H
#include <thread>
#include <future>
#include "json.hpp"
#include <map>
#include <utility>
#include <iostream>
#include <memory>
#include <queue>
#ifdef __cplusplus
extern "C"{
#include "libwebsockets.h"
#endif
#ifdef __cplusplus
}
#endif

// using namespace web;
// using namespace web::websockets::client;
using namespace std;
using namespace nlohmann;

class WebSocketTransport
{
public:
    class TransportListener
    {
    public:
        virtual void onOpen() = 0;
        virtual void onClosed() = 0;
        virtual void onMessage(json message) = 0;
        virtual void onDisconnected() = 0;
        virtual void onFailed() = 0;
    };

public:
    WebSocketTransport(string url, TransportListener *listener);
    ~WebSocketTransport();

public:
    bool closed();
    void close();
    void sendSync(json message);
    void send(json message);

private:
    void runWebSocket();

public:
    void scheduleTask(int afterMs, std::function<void()> task); 
    void handleMessages(std::string message);

private:
    bool m_closed;
    std::chrono::time_point<std::chrono::system_clock> m_lastPingTime;
    std::string m_url;
    std::string m_path;
    std::string m_prefix;
    int m_port;
    std::string m_msg_to_send;
    std::thread *m_pWsThread;
    int m_pingTimeout{0};
    int m_pongTimeout{0};
    int m_timeCount{0};
    int m_reconnectDuration{3};
    struct lws_protocols m_protocols[2];
    lws_context *m_context{nullptr};

private:
    lws *m_wsClient{nullptr};
    // Thread exit flag stop && noMsg
    volatile bool m_stopped{false};
    // noMsg is used to ensure that messages leaveing the room are sent out before the thread exits
    volatile bool m_noMsg{true};
    std::string m_receivedMsg;
    // Pending messages queue
    std::queue<std::string> m_msgQueue;
    TransportListener *m_listener{nullptr};
    std::function<void()> m_task;
    lws_sorted_usec_list_t m_sul;
};

#endif