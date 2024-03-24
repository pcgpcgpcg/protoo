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
#include "MessageQueue.h"
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

struct ConnectionInfo {
    lws_sorted_usec_list_t	sul;	     /* schedule connection retry */
    struct lws* wsi;	     /* related wsi if any */
    uint16_t		retry_count; /* count of consequetive retries */
    void* user;		     /* pointer to user data */
};

class WebSocketTransport
{
public:
    class TransportListener
    {
    public:
        virtual void onConnected() = 0;
        virtual void onReConnected() = 0;
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
    void InvokeTask(std::function<void()> task);
    void scheduleTask(int afterMs, std::function<void()> task);
    void cancelScheduledTask();
    void handleMessages(std::string message);

public:
    lws_context* m_context{ nullptr };
    std::string m_url;
    std::string m_path;
    std::string m_prefix;
    int m_port;
    struct lws_protocols m_protocols[2];
    ConnectionInfo m_connectionInfo{ nullptr };
    // Thread exit flag stop && noMsg
    volatile bool m_stopped{ false };
    std::string m_receivedMsg;
    volatile bool m_noMsg{ true };
    // Pending messages queue
    std::queue<std::string> m_msgQueue;
    TransportListener* m_listener{ nullptr };

private:
    bool m_closed;
    std::chrono::time_point<std::chrono::system_clock> m_lastPingTime;
    std::string m_msg_to_send;
    std::thread *m_pWsThread;
    int m_pingTimeout{0};
    int m_pongTimeout{0};
    int m_timeCount{0};
    int m_reconnectDuration{3};

private:
    //lws *m_wsClient{nullptr};
    // noMsg is used to ensure that messages leaveing the room are sent out before the thread exits
    std::function<void()> m_task;
    lws_sorted_usec_list_t m_sul;
    //MessageQueue m_messageQueue;
    // 线程安全的任务队列
    std::queue<std::function<void()>> tasks;
    std::mutex tasks_mutex;
    int m_nServiceRet{0};
};

#endif