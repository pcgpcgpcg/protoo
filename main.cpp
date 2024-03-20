#include <iostream>
#include "Message.h"
#include "WebsocketTransport.h"
#include "HTTPTransport.h"
#include "Peer.h"
#include <memory>
#include "uv.h"
#include "qrtm.h"
// #ifdef __cplusplus
// extern "C" {
// #endif
// #include "libwebsockets.h"
// #include "uv.h"
// #ifdef __cplusplus
// }
// #endif

class MyRTMListener : public RTMListener
{
public:
    MyRTMListener() {}
    ~MyRTMListener() {}

    void onReceiveMessage(RTM*, const std::string&, const std::string&, const std::string&) override
    {
        //连接上之后，接收消息
    }

    void onReceivePresence(RTM*, const std::string&, const QRtmPresenceEvent&) override
    {
        // Your implementation here
    }

    void onConnected() override
    {
        //连接上之后，登陆 登陆频道 发送消息
        std::cout << "[MAIN] RTM::onConnected" << std::endl;
    }

    void onDisconnected() override
    {
        // Your implementation here
    }

    void onReconnecting() override
    {
        // Your implementation here
    }

    void onReconnected() override
    {
        // Your implementation here
    }
};

void sigint_handler(int sig)
{
    //interrupted = 1;
}

int main(int argc, char *argv[]){
    //signal(SIGINT, sigint_handler);
    std::cout<<"hello world" << std::endl;

    MyRTMListener listener;
    std::unique_ptr<RTM> rtm(new RTM("appId", "user1234567", RTMConfig(), &listener));
    try{
        //打印当前线程id
        std::cout << "[MAIN] main thread id: " << std::this_thread::get_id() << std::endl;
        rtm->login().get();
        rtm->subscribe("notification").get();
        rtm->publish("notification", "hello world");
        }catch(const std::exception &e){
        std::cout << "login error" << std::endl;
    }
        
    //     //进行http post请求
    // auto transport = new HTTPTransport();
    // //请求呼叫规则列表
    // transport->sendPostRequest(std::string("http://192.168.31.16:8200"),"/rules/list", "");
    // //根据某个虚拟号码规则，获取相关用户
    // transport->sendPostRequest(std::string("http://192.168.31.16:8200"),"/rules/search", "");
    // }catch(const std::exception &e){
    //     std::cout << "login error" << std::endl;
    // }

    //std::unique_ptr<protoo::Peer> peer(new protoo::Peer("ws://152.136.16.141:8080/websocket"));
    // MyTransportListener listener;
    // auto transport = new WebSocketTransport("ws://152.136.16.141:8080/websocket", &listener);
    // listener.onOpen();
    // transport->send(json::parse("{\"type\":\"login\",\"data\":{\"username\":\"test\",\"password\":\"test\"}}"));
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return 0;
}


