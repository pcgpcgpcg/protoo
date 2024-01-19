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
    m_pPeer.reset(new protoo::Peer(m_config.serverUrl));
    //定义promise
    auto promise = std::make_shared<std::promise<LoginResponse>>();
    //连接成功
    m_pPeer->onPeerOpen = [this]() {
        std::cout << "[RTM] Peer::onPeerOpen" << std::endl;
        //发送login请求
        try {
            json requestData = {
		  {"displayName", "pc"},
		  {"device", "pc"},
		  {"accessToken", this->m_config.accessToken},
		  {"appId", m_appId}
		};
            auto retJson = m_pPeer->request("login", requestData).get();
            promise.set_value({0});    
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
    m_pPeer->onPeerFailed = [this]() {
        std::cout << "[RTM] Peer::onPeerFailed" << std::endl;
        //TODO
        promise.set_exception(std::make_exception_ptr(std::runtime_error("request timeout")));
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

    //
}

std::future<LogoutResponse> RTM::logout(){
    auto requestData = json({
        {"peerId", m_userId}
    });
    try{
        auto retJson = m_pPeer->request("logout", requestData).get();
        promise.set_value({0});
    } catch (const std::exception &e){
        std::cout << "[RTM] Peer::logout error" << std::endl;
        promise.set_exception(std::make_exception_ptr(std::runtime_error("request timeout")));
    }
}

std::future<SubscribeResponse> RTM::subscribe(const std::string &channelName){

}

std::future<UnsubscribeResponse> RTM::unsubscribe(const std::string &channelName){

}

std::future<PublishResponse> RTM::publish(const std::string &channelName, const std::string &message){

}