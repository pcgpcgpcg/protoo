#include <iostream>
#include "Message.h"
#include "WebsocketTransport.h"
#include "HTTPTransport.h"
#include "Peer.h"
#include <memory>
//#include "uv.h"
#include "qrtm.h"
#include <fmt/core.h>
#include <fmt/color.h>
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

	void onReceiveMessage(RTM* rtm, const std::string& channelId, const std::string& peerId, const std::string& msg) override
	{
        // 解析 JSON 字符串
        nlohmann::json jsonMsgData = nlohmann::json::parse(msg);
        if (jsonMsgData.find("action") == jsonMsgData.end()) {
            return;
        }
        // 访问嵌套数据
        auto actionName = jsonMsgData["action"].get<std::string>();
        // 连接上之后，接收消息
        if (actionName == std::string("call"))
        {
            fmt::print(fg(fmt::color::steel_blue) | fmt::emphasis::italic, "Incoming call from {}, Do you agree? (yes/no): \n", peerId);
            std::string userInput;
            std::cin >> userInput;

            if (userInput == "yes")
            {
                // 处理用户同意的情况
                std::cout << "Call accepted." << std::endl;
                // TODO: 发送 同意 消息给对方
                // 调用rtm发送呼叫信息
                json requestData = {
                    {"action", "response"},
                    {"result", "accept"}};
                rtm->publish("notification", requestData.dump());
            }
            else if (userInput == "no")
            {
                // 处理用户不同意的情况
                std::cout << "Call rejected." << std::endl;
                // TODO: 发送 拒绝 消息给对方
                 json requestData = {
                    {"action", "response"},
                    {"result", "decline"}};
                rtm->publish("notification", requestData.dump());
            }
            else
            {
                // 处理无效输入
                std::cout << "Invalid input." << std::endl;
            }
        }else if(actionName == std::string("response")){
            auto result = jsonMsgData["result"].get<std::string>();
            if(result == "accept"){
                std::cout << "Call accepted." << std::endl;
            }else if(result == "decline"){
                std::cout << "Call rejected." << std::endl;
            }
        }
    }

    void onReceivePresence(RTM*, const std::string&, const QRtmPresenceEvent&) override
    {
        // Your implementation here
    }

    void onConnected(RTM* rtm) override
    {
        //连接上之后，登陆 登陆频道 发送消息
        std::cout << "[MAIN] RTM::onConnected" << std::endl;
        try {
            //打印当前线程id
            std::cout << "[MAIN] main thread id: " << std::this_thread::get_id() << std::endl;
            rtm->login().get();
            rtm->subscribe("notification").get();
        }
        catch (const std::exception& e) {
            std::cout << "login error" << std::endl;
        }
    }

    void onDisconnected(RTM*) override
    {
        // Your implementation here
    }

    void onReconnecting(RTM*) override
    {
        // Your implementation here
    }

    void onReconnected(RTM*) override
    {
        // Your implementation here
    }

    void onClosed(RTM*) override
    {

     }
};

void sigint_handler(int sig)
{
    //interrupted = 1;
}

void processCommand(RTM* rtm, const std::string& command) {
    if (command == "help") {
        fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold,
            "Supported commands:\n");
        fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold,
            "help - Display this help message\n");
        fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold,
            "call - call the room or person\n");
        fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold,
            "exit - Exit the program\n");
    }
    else if (command.substr(0, 4) == "call") {
        std::string toNumber = command.substr(5); // 假设命令格式为"call 8881"
        std::cout << " call to " << toNumber << std::endl;
        //调用rtm发送呼叫信息
        json requestData = {
            {"action", "call"},
            {"room", "12345678"},
            {"data",{
                {"virtualNum", "8880"},
                {"video",{
                    {"code", "H264"},
                    {"resolution", "1920x1080"},
                }},
                {"audio",{
                    {"sampleRate", "44100"},
                    {"channels", "2"},
                }}
            }}
        };
        rtm->publish("notification", requestData.dump());
    }
    else if (command == "exit") {
        std::exit(0);
    }
    else {
        std::cout << "Unknown command: " << command << "\n";
    }
}

int main(int argc, char *argv[]){
    //signal(SIGINT, sigint_handler);
    std::cout<<"hello world" << std::endl;

    MyRTMListener listener;
    std::string defaultUrl = "ws://192.168.31.110:8002";//"ws://49.232.122.245:8002";
    std::string userId = fmt::format("user{}", generateRandomNumber(6));
    std::unique_ptr<RTM> rtm(new RTM("appId", userId, RTMConfig(), &listener));
    rtm->connect(defaultUrl);

    //just wait ws and worker thread log show before interactive
    //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        
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
    std::string input;
    while(true){
        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::getline(std::cin, input); // 读取整行输入
        processCommand(rtm.get(), input); // 处理输入
    }

    rtm.reset();

    return 0;
}


