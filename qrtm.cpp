//the qrtm sdk source file
#include "qrtm.h"

RTM::RTM(std::string appId, std::string userId, const RTMConfig &config, RTMListener *listener){
    m_appId = appId;
    m_userId = userId;
    m_config = config;
    m_pListener = listener;
}


RTM::~RTM(){

}

std::future<LoginResponse> RTM::login(){
    //先建立websocket链接
    m_pPeer.reset(new protoo::Peer(m_config.serverUrl.value_or("wss://superrtc.com/websocket")));
    //定义promise
    auto promise = std::make_shared<std::promise<LoginResponse>>();
    //连接成功
    m_pPeer->onPeerOpen = [this, &promise]() {
        std::cout << "[RTM] Peer::onPeerOpen" << std::endl;
        //发送login请求
        try {
            json requestData = {
		  {"displayName", "pc"},
		  {"device", "pc"},
		  {"accessToken", this->m_config.token.value_or("")},
		  {"appId", m_appId}
		};
            auto retJson = m_pPeer->request("login", requestData).get();
            promise->set_value({0});    
        } catch (const std::exception &e){
            std::cout << "[RTM] Peer::onPeerOpen error" << std::endl;
        }
    };
    //连接失败
    m_pPeer->onPeerClosed = [this]() {
        std::cout << "[RTM] Peer::onPeerClosed" << std::endl;
        //TODO
    };
    //连接断开
    m_pPeer->onPeerDisconnected = [this]() {
        std::cout << "[RTM] Peer::onPeerDisconnected" << std::endl;
        //TODO
    };
    //连接失败
    m_pPeer->onPeerFailed = [this, &promise]() {
        std::cout << "[RTM] Peer::onPeerFailed" << std::endl;
        //TODO
        promise->set_exception(std::make_exception_ptr(std::runtime_error("request timeout")));
    };
    //收到notification
    m_pPeer->onServerNotification = [this](const json &data) {
        std::cout << "[RTM] Peer::onReceiveNotification" << std::endl;
        //TODO
    };
    //收到request
    m_pPeer->onServerRequest = [this](const json &data, AcceptCallback accept, RejectCallback reject) {
        std::cout << "[RTM] Peer::onReceiveRequest" << std::endl;
        //TODO
    };

    return promise->get_future();
}

std::future<LogoutResponse> RTM::logout(){
    //定义promise
    auto promise = std::make_shared<std::promise<LogoutResponse>>();
    auto requestData = json({
        {"peerId", m_userId}
    });
    try{
        auto retJson = m_pPeer->request("logout", requestData).get();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        promise->set_value({timestamp});
    } catch (const std::exception &e){
        std::cout << "[RTM] Peer::logout error" << std::endl;
        promise->set_exception(std::make_exception_ptr(std::runtime_error("request timeout")));
    }

    return promise->get_future();
}

std::future<SubscribeResponse> RTM::subscribe(const std::string &channelName){
    //定义promise
    auto promise = std::make_shared<std::promise<SubscribeResponse>>();
    auto requestData = json({
        {"channelId", channelName}
    });
    try{
        auto retJson = m_pPeer->request("joinChannel", requestData).get();
        //获取int64的当前时间的时间戳
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        promise->set_value({timestamp, channelName});
    } catch (const std::exception &e){
        std::cout << "[RTM] Peer::logout error" << std::endl;
        promise->set_exception(std::make_exception_ptr(std::runtime_error("request timeout")));
    }
    return promise->get_future();
}

std::future<UnsubscribeResponse> RTM::unsubscribe(const std::string &channelName){
    //定义promise
    auto promise = std::make_shared<std::promise<UnsubscribeResponse>>();
auto requestData = json({
        {"channelId", channelName}
    });
    try{
        auto retJson = m_pPeer->request("leaveChannel", requestData).get();
        //获取int64的当前时间的时间戳
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        promise->set_value({timestamp, channelName});
    } catch (const std::exception &e){
        std::cout << "[RTM] Peer::logout error" << std::endl;
        promise->set_exception(std::make_exception_ptr(std::runtime_error("request timeout")));
    }
    return promise->get_future();
}

void RTM::publish(const std::string &channelName, const std::string &message)
{
    // 定义promise
    auto promise = std::make_shared<std::promise<PublishResponse>>();
    auto requestData = json({{"channelId", channelName},
                             {"msg", message}});

    m_pPeer->notify("channelMsgText", requestData);
                   // 获取int64的当前时间的时间戳
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    promise->set_value({timestamp, channelName});
}