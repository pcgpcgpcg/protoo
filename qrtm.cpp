//the qrtm sdk source file
#include "qrtm.h"

RTM::RTM(std::string appId, std::string userId, const RTMConfig &config, RTMListener *listener){
    m_appId = appId;
    m_userId = userId;
    m_config = config;
    m_pListener = listener;
    //构造的时候就起一个线程，用于处理rtm消息交互和异步请求
    // m_pThread.reset(new std::thread([this](){
    m_bRunning.store(true);
    //m_WorkerThread = std::thread(&RTM::WorkerFunction, this);
    m_WorkerThread = std::thread([&] {
        m_WorkerThreadId = std::this_thread::get_id(); // 保存workerThread的线程ID
        WorkerFunction();
        });
}


RTM::~RTM(){
    // 在析构函数中停止并释放线程
    m_bRunning.store(false);
    if (m_WorkerThread.joinable()) {
        m_WorkerThread.join();
    }
}

std::future<LoginResponse> RTM::login() {
	//无论如何确保对于rtm相关的所有调用都运行到workerThread上
	if (IsRunningOnWorkerThread()) {
		return loginInternal();
	}

    // 创建一个Promise来桥接异步执行结果
    auto promise = std::make_shared<std::promise<LoginResponse>>();
    std::future<LoginResponse> future = promise->get_future();
	this->InvokeTask([this, promise]() {
        try {
            // 等待loginInternal返回的future完成，并将结果设置给promise
            promise->set_value_at_thread_exit(loginInternal().get());
        }
        catch (...) {
            // 如果有异常，传递异常给promise
            promise->set_exception_at_thread_exit(std::current_exception());
        }
		});
    return future;
}

std::future<LogoutResponse> RTM::logout() {
    if (IsRunningOnWorkerThread()) {
        	return logoutInternal();
	}

    // 创建一个Promise来桥接异步执行结果
    auto promise = std::make_shared<std::promise<LogoutResponse>>();
    std::future<LogoutResponse> future = promise->get_future();
    this->InvokeTask([this, promise]() {
        try {
            // 等待loginInternal返回的future完成，并将结果设置给promise
            promise->set_value_at_thread_exit(logoutInternal().get());
        }
        catch (...) {
            // 如果有异常，传递异常给promise
            promise->set_exception_at_thread_exit(std::current_exception());
        }
        });

    return future;
}

std::future<SubscribeResponse> RTM::subscribe(const std::string& channelName) {
    if (IsRunningOnWorkerThread()) {
        return subscribeInternal(channelName);
	}

    // 创建一个Promise来桥接异步执行结果
    auto promise = std::make_shared<std::promise<SubscribeResponse>>();
    std::future<SubscribeResponse> future = promise->get_future();
    this->InvokeTask([this, promise, channelName]() {
        try {
            // 等待loginInternal返回的future完成，并将结果设置给promise
            promise->set_value_at_thread_exit(subscribeInternal(channelName).get());
        }
        catch (...) {
            // 如果有异常，传递异常给promise
            promise->set_exception_at_thread_exit(std::current_exception());
        }
        });
    return future;
}

std::future<UnsubscribeResponse> RTM::unsubscribe(const std::string& channelName) {
    if (IsRunningOnWorkerThread()) {
        return unsubscribeInternal(channelName);
        }

    // 创建一个Promise来桥接异步执行结果
    auto promise = std::make_shared<std::promise<UnsubscribeResponse>>();
    std::future<UnsubscribeResponse> future = promise->get_future();
    this->InvokeTask([this, promise, channelName]() {
        try {
            // 等待loginInternal返回的future完成，并将结果设置给promise
            promise->set_value_at_thread_exit(unsubscribeInternal(channelName).get());
        }
        catch (...) {
            // 如果有异常，传递异常给promise
            promise->set_exception_at_thread_exit(std::current_exception());
        }
        });
    return future;
}

void RTM::publish(const std::string& channelName, const std::string& message){
    if (IsRunningOnWorkerThread()) {
        publishInternal(channelName, message);
		}
    this->InvokeTask([this, channelName, message]() {
        this->publishInternal(channelName, message);
        });
}

void RTM::connect(std::string serverUrl) {
    //先建立websocket链接
    std::string fullUrl = serverUrl + std::string("/?roomId=&peerId=") + m_userId;
    //std::string defaultUrl2 = "ws://152.136.16.141:8080/?roomId=&peerId=" + m_userId;
    m_pPeer.reset(new protoo::Peer(m_config.serverUrl.value_or(fullUrl)));
    //定义promise
    auto promise = std::make_shared<std::promise<LoginResponse>>();

    //连接成功
	m_pPeer->onPeerConnected = [this, &promise]() {
        //将所有的回调函数都放到workerThread上执行
		this->InvokeTask([this]() {
			m_pListener->onConnected(this);
			});
	};

    m_pPeer->onPeerReConnected = [this]() {
        //自动订阅之前保存的channel
		this->InvokeTask([this]() {
            //做三件事
            //1. 重新login
            this->login().get();
            //2. 重新订阅之前订阅的频道
            this->subscribe(m_channelName).get();
            //3.通知给上层已经重连成功
			m_pListener->onReconnected(this);
			});
    };

	//连接失败
	m_pPeer->onPeerClosed = [this]() {
		//连接失败后，不做任何处理，等待自动重连
		std::cout << "[RTM] Peer::onPeerClosed" << std::endl;
		this->InvokeTask([this]() {
			m_pListener->onClosed(this);
			});
		//TODO
	};

    //连接断开
    m_pPeer->onPeerDisconnected = [this]() {
        //连接断开后，不做任何处理，等待自动重连
        std::cout << "[RTM] Peer::onPeerDisconnected" << std::endl;
        this->InvokeTask([this]() {
            m_pListener->onDisconnected(this);
            });
        //TODO
    };

    //连接失败
    m_pPeer->onPeerFailed = [this]() {
        std::cout << "[RTM] Peer::onPeerFailed" << std::endl;
        //TODO
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
            this->InvokeTask([this, channelId, peerId, msg]() {
                m_pListener->onReceiveMessage(this, channelId, peerId, msg);
                });
        }
        else if (method == "memberJoin") {
            this->InvokeTask([this, channelId, peerId]() {
                m_pListener->onReceivePresence(this, channelId, { QRTMPresenceEventType::RemoteJoinChannel, channelId, peerId });
                });

        }
		else if (method == "memberLeave") {
			this->InvokeTask([this, channelId, peerId]() {
				m_pListener->onReceivePresence(this, channelId, { QRTMPresenceEventType::RemoteLeaveChannel, channelId, peerId });
				});
		}
    };
    //收到request
    m_pPeer->onServerRequest = [this](const json& data, AcceptCallback accept, RejectCallback reject) {
        std::cout << "[RTM] Peer::onReceiveRequest" << std::endl;
        //TODO
    };
}

std::future<LoginResponse> RTM::loginInternal() {
    //定义promise
    auto promise = std::make_shared<std::promise<LoginResponse>>();
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
        promise->set_value({ 0 });
    }
    catch (const std::exception& e)
    {
        std::cout << "[RTM] Peer::onPeerOpen error" << std::endl;
        promise->set_exception(std::make_exception_ptr(std::runtime_error("request timeout")));
    }
    return promise->get_future();
}

std::future<LogoutResponse> RTM::logoutInternal() {
    //定义promise
    auto promise = std::make_shared<std::promise<LogoutResponse>>();
    auto requestData = json({
        {"peerId", m_userId}
        });
    try {
        auto retJson = m_pPeer->request("logout", requestData).get();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        promise->set_value({ timestamp });
    }
    catch (const std::exception& e) {
        std::cout << "[RTM] Peer::logout error" << std::endl;
        promise->set_exception(std::make_exception_ptr(std::runtime_error("request timeout")));
    }

    return promise->get_future();
}
std::future<SubscribeResponse> RTM::subscribeInternal(const std::string& channelName) {
    //定义promise
    auto promise = std::make_shared<std::promise<SubscribeResponse>>();
    auto requestData = json({
        {"channelId", channelName}
        });
    try {
        auto retJson = m_pPeer->request("joinChannel", requestData).get();
        //获取int64的当前时间的时间戳
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        m_channelName = channelName;
        promise->set_value({ timestamp, channelName });
    }
    catch (const std::exception& e) {
        std::cout << "[RTM] Peer::logout error" << std::endl;
        promise->set_exception(std::make_exception_ptr(std::runtime_error("request timeout")));
    }
    return promise->get_future();
}
std::future<UnsubscribeResponse> RTM::unsubscribeInternal(const std::string& channelName){
    //定义promise
    auto promise = std::make_shared<std::promise<UnsubscribeResponse>>();
    auto requestData = json({
            {"channelId", channelName}
        });
    try {
        auto retJson = m_pPeer->request("leaveChannel", requestData).get();
        //获取int64的当前时间的时间戳
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        promise->set_value({ timestamp, channelName });
    }
    catch (const std::exception& e) {
        std::cout << "[RTM] Peer::logout error" << std::endl;
        promise->set_exception(std::make_exception_ptr(std::runtime_error("request timeout")));
    }
    return promise->get_future();
}
void RTM::publishInternal(const std::string& channelName, const std::string& message) {
    // 定义promise
    auto promise = std::make_shared<std::promise<PublishResponse>>();
    auto requestData = json({ {"channelId", channelName},
                             {"msg", message} });

    m_pPeer->notify("channelMsgText", requestData);
    // 获取int64的当前时间的时间戳
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    promise->set_value({ timestamp, channelName });
}

void RTM::WorkerFunction() {
    // 这是线程的工作函数
    while (m_bRunning.load()) {
        // 执行一些工作...
        while (!tasks.empty())
        {
            std::function<void()> task;
            {
                std::lock_guard<std::mutex> lock(tasks_mutex);
                if (!tasks.empty())
                {
                    task = tasks.front();
                    tasks.pop();
                }
            }
            if (task)
            {
                task(); // 在线程A的上下文中执行任务
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 仅为示例，避免忙等
    }
}

void RTM::InvokeTask(std::function<void()> task)
{
    {
        std::lock_guard<std::mutex> lock(tasks_mutex);
        tasks.push(task);
    }
}

// 检查当前代码是否运行在workerThread上
bool RTM::IsRunningOnWorkerThread() const {
    return std::this_thread::get_id() == m_WorkerThreadId;
}