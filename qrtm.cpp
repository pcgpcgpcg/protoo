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
    std::string defaultUrl = "ws://49.232.122.245:8002/?roomId=&peerId="+m_userId;
    std::string defaultUrl2 = "ws://152.136.16.141:8080/?roomId=&peerId="+m_userId;
    m_pPeer.reset(new protoo::Peer(m_config.serverUrl.value_or(defaultUrl)));
    //定义promise
    auto promise = std::make_shared<std::promise<LoginResponse>>();

    //连接成功
    m_pPeer->onPeerConnected = [this, &promise]() {
        //连接成功后，再发送login请求
        //TODO: 这里一旦get,就会导致websockets线程阻塞住了。。 所以要想办法解决这个问题
        try
        {
            json requestData = {
                {"displayName", "pc"},
                {"device",{
                    {"name", "pc"},
                    {"version", "1.0.0"}
                }},
                {"accessToken", this->m_config.token.value_or("")},
                {"appId", m_appId} };
            auto retJson = m_pPeer->request("login", requestData).get();
            std::cout << "[RTM] login response: " << retJson.dump() << std::endl;
            m_pListener->onConnected();
            promise->set_value({ 0 });
        }
        catch (const std::exception& e)
        {
            std::cout << "[RTM] Peer::onPeerOpen error" << std::endl;
            promise->set_exception(std::make_exception_ptr(std::runtime_error("request timeout")));
        } 
    };

    m_pPeer->onPeerReConnected = [this]() {
        //自动订阅之前保存的channel
		m_pListener->onReconnected();
        if (!m_channelName.empty()) {
			subscribe(m_channelName);
		}
	};

    //连接失败
    m_pPeer->onPeerClosed = [this]() {
        //连接失败后，不做任何处理，等待自动重连
        std::cout << "[RTM] Peer::onPeerClosed" << std::endl;
        m_pListener->onClosed();
        //TODO
    };

    //连接断开
    m_pPeer->onPeerDisconnected = [this]() {
        //连接断开后，不做任何处理，等待自动重连
        std::cout << "[RTM] Peer::onPeerDisconnected" << std::endl;
        m_pListener->onDisconnected();
        //TODO
    };

    //连接失败
    m_pPeer->onPeerFailed = [this, promise]() {
        std::cout << "[RTM] Peer::onPeerFailed" << std::endl;
        //TODO
        promise->set_exception(std::make_exception_ptr(std::runtime_error("request timeout")));
    };

    //收到notification
    m_pPeer->onServerNotification = [this](const json& data) {
        std::cout << "[RTM] Peer::onReceiveNotification" << std::endl;
        //解析method
        auto method = data["method"].get<std::string>();
        auto msgData = data["data"];
        auto channelId = msgData["channelId"].get<std::string>();
        auto peerId = msgData["peerId"].get<std::string>();
        if (method == "channelMsgText") {
            //解析接收到的json,解析到channelId, peerId和msg
            auto msg = msgData["msg"].get<std::string>();
            m_pListener->onReceiveMessage(this, channelId, peerId, msg);
        }
        else if (method == "memberJoin") {
            m_pListener->onReceivePresence(this, channelId, { QRTMPresenceEventType::RemoteJoinChannel, channelId, peerId });

        }
        else if (method == "memberLeave") {
            m_pListener->onReceivePresence(this, channelId, { QRTMPresenceEventType::RemoteLeaveChannel, channelId, peerId });
        }
        else {

        }
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
        m_channelName = channelName;
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