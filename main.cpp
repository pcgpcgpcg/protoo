#include <iostream>
#include "Message.h"
#include "WebSocketTransport.h"
#include "Peer.h"
#include <memory>
#include "uv.h"
// #ifdef __cplusplus
// extern "C" {
// #endif
// #include "libwebsockets.h"
// #include "uv.h"
// #ifdef __cplusplus
// }
// #endif

class MyTransportListener : public WebSocketTransport::TransportListener
{
public:
    MyTransportListener() {}
    ~MyTransportListener() {}

    void onOpen() override
    {
        // Your implementation here
    }

    void onClosed() override
    {
        // Your implementation here
    }

    void onMessage(json message) override
    {
        // Your implementation here
    }

    void onDisconnected() override
    {
        // Your implementation here
    }

    void onFailed() override
    {
        // Your implementation here
    }
};

void sigint_handler(int sig)
{
    //interrupted = 1;
}

int main(int argc, char *argv[]){
    signal(SIGINT, sigint_handler);
    std::cout<<"hello world" << std::endl;
    std::unique_ptr<protoo::Peer> peer(new protoo::Peer("ws://152.136.16.141:8080/websocket"));
    // MyTransportListener listener;
    // auto transport = new WebSocketTransport("ws://152.136.16.141:8080/websocket", &listener);
    // listener.onOpen();
    // transport->send(json::parse("{\"type\":\"login\",\"data\":{\"username\":\"test\",\"password\":\"test\"}}"));
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return 0;
}


