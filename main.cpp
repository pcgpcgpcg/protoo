#include <iostream>
#include "Message.h"
#include "WebSocketTransport.h"
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
    auto transport = new WebSocketTransport("ws://1.1.1.1/websocket", nullptr);

    return 0;
}


