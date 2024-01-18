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
    //创建uv事件循环
    uv_loop_t *loop = uv_default_loop();
    uv_loop_init(loop);
    uv_timer_t *timer_req = new uv_timer_t;
    uv_timer_init(loop, timer_req);//初始化定时器
    uv_timer_start(timer_req, [](uv_timer_t *req) {
        std::cout<<"timer"<<std::endl;
    }, 0, 1000); //启动定时器，每隔1秒执行一次回调函数
    //执行事件循环。调用此函数后,libuv会开始处理事件，包括定时器事件
    uv_run(loop, UV_RUN_DEFAULT);
    //清理工作
    uv_loop_close(loop);
    delete loop;
    delete timer_req;
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


