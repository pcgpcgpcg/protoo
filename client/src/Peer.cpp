#include "Peer.h"
#include "Message.h"

using namespace std;
using namespace nlohmann;

namespace protoo {

	Peer::Peer(string url) {
		m_closed = false;
		m_connected = false;
		//自己new一个transport
		m_pTransport.reset(new WebSocketTransport(url, this));
		//m_pTransport.reset(new WebSocketTransport(url, json(), this));

	}
	Peer::~Peer() {
		if(m_pTransport.get()){
			m_pTransport->close();
		}
	}

	bool Peer::closed()
	{
		return m_closed;
	}

	void Peer::close()
	{
		if (m_closed) {
			return;
		}
		if(m_pTransport.get()){
			m_pTransport->close();
		}
	}

	bool Peer::connected() {
		return m_connected;
	}

	json Peer::data() {
		return m_data;
	}

	void Peer::setData(json datax) {
		//TODO
	}

	std::future<json> Peer::request(string method, json data) {
		auto request = Message::createRequest(method, data);
		auto id = request["id"].get<int>();
		std::cout << "[Peer] send a new request id=" << request["id"].get<int>() << " method=" << method << endl;
		auto promise = std::make_shared<std::promise<json>>();
		//std::promise<json> promise;
		//m_promises[id] = std::make_shared<std::promise<json>>(std::move(promise));
		auto future = promise->get_future();
		m_promises[id] = promise;
		int timeout_ms = 15000 * (15 + (0.1 * m_promises.size()));
		//设置计划任务，用于处理超时
		m_pTransport->scheduleTask(timeout_ms, [this, id]() {
			auto it = m_promises.find(id);
			if (it != m_promises.end()) {
				std::cout << "[Peer] request timeout id=" << id << std::endl;
				it->second->set_exception(std::make_exception_ptr(std::runtime_error("request timeout")));
				m_promises.erase(it);
			}
		});
		m_pTransport->send(request);
		return future;
	}


	void Peer::notify(string method, json data) {
		auto request = Message::createNotification(method, data);
		this->m_pTransport->send(request);
	}

	void Peer::onOpen()
	{
		std::cout << "[Peer] onOpen!" << std::endl;
		m_connected = true;
		if (onPeerOpen)
		{
			onPeerOpen();
		}
	}

	void Peer::onClosed()
	{
		if (this->m_closed)
			return;
		m_connected = false;
		if (onPeerClosed)
		{
			onPeerClosed();
		}
	}

	void Peer::onDisconnected()
	{
		m_connected = false;
		if (onPeerDisconnected)
		{
			onPeerDisconnected();
		}
	}

	void Peer::onFailed()
	{
		if (this->m_closed)
			return;
		m_connected = false;
		if (onPeerFailed)
		{
			onPeerFailed();
		}
	}

	void Peer::onMessage(json message) {
       std::cout<<"[Peer] onMessage:"<<message.dump()<<std::endl;
	   if(message["request"].is_boolean()){
		   std::cout<<"[Peer] onMessage request:"<<message.dump()<<std::endl;
		   handleRequest(message);
	   }
	   else if(message["response"].is_boolean()){
		   std::cout<<"[Peer] onMessage response:"<<message.dump()<<std::endl;
		   handleResponse(message);
	   }
	   else if(message["notification"].is_boolean()){
		   std::cout<<"[Peer] onMessage notification:"<<message.dump()<<std::endl;
		   handleNotification(message);
	   }
	   else{
		   std::cout<<"[Peer] onMessage invalid:"<<message.dump()<<std::endl;
	   }
	}

	void Peer::handleRequest(json request)
	{
		// 定义 accept 和 reject 回调
		AcceptCallback accept = [this, request](const std::string &data)
		{
			auto response = Message::createSuccessResponse(request, data);
			this->m_pTransport->send(response);
		};

		RejectCallback reject = [this, request](int errorCode, const std::string &errorReason)
		{
			auto response = Message::createErrorResponse(request, errorCode, errorReason);
			this->m_pTransport->send(response);
		};

		// 判断 emitRequest 是否已被设置，如果已设置，则使用设置的回调，否则不进行任何操作
		if (onServerRequest)
		{
			onServerRequest(request, accept, reject);
		}
	}

	void Peer::handleResponse(json response)
	{
		auto id = response["id"].get<int>();
		auto it = m_promises.find(id);
		auto isOk = response.contains("ok") && response["ok"].get<bool>();
		if (it != m_promises.end())
		{
			isOk ? it->second->set_value(response["data"]) : it->second->set_exception(std::make_exception_ptr(std::runtime_error("response error")));
			// 取消计划任务
			m_pTransport->cancelScheduledTask();
			m_promises.erase(it);
		}
	}

	void Peer::handleNotification(json notification) {
		// 判断一下接收线程是在哪个线程，如果还是在lws线程，需要给到队列里，然后work线程一起处理
		//获取当前所在线程并打印
		std::cout << "handleNotification thread id=" << std::this_thread::get_id() << std::endl;
		if (onServerNotification)
		{
			onServerNotification(notification);
		}
	}
}
