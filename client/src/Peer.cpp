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

	//void Peer::requestTest(string method, json data) {
	//    auto request = Message::createRequest(method, data);
	//    std::cout << "[Peer] send a new request id=:" << request["id"].get<int>() << " method=" << method << endl;
	//    //会进入到uWS的线程发送
	//    std::promise<json> promise;
	//    this->m_pTransport->send(request);//just like await    
	//}

	std::future<json> Peer::request(string method, json data) {
		auto request = Message::createRequest(method, data);
		auto id = request["id"].get<int>();
		std::cout << "[Peer] send a new request id=:" << request["id"].get<int>() << " method=" << method << endl;
		auto promise = std::make_shared<std::promise<json>>();
		auto future = promise->get_future();
		//存储promise 并计划一个超时任务
		m_promises[id] = promise;
		

	}

// 	   std::future<std::string> request(const std::string& method, const std::string& data = "") {
//         Message request = Message::createRequest(method, data);
//         // this->_logger.debug...

//         // 发送消息
//         transport->send(request.toString()); // 假设 Message 类有 toString 方法来序列化数据

//         // 创建 promise 和 future
//         auto promise = std::make_shared<std::promise<std::string>>();
//         auto future = promise->get_future();

//         // 存储 promise，并计划一个超时任务
//         promises[request.id] = promise;

//         // 超时配置
//         const int timeout_ms = ...;

//         // 使用 lws_sorted_usec_list_t 结构和 transport 的 scheduleTimeout 方法来计划超时任务
//         lws_sorted_usec_list_t sul;
//         transport->scheduleTimeout(&sul, [this, request_id=request.id]() {
//             auto it = promises.find(request_id);
//             if (it != promises.end()) {
//                 it->second->set_exception(std::make_exception_ptr(std::runtime_error("request timeout")));
//                 promises.erase(it);
//             }
//         }, timeout_ms);

//         return future;
//     }
// };


	void Peer::notify(string method, json data) {
		auto request = Message::createNotification(method, data);
		//this->m_pTransport->send(request);
	}


	void Peer::onOpen() {
		std::cout<<"[Peer] onOpen!"<<std::endl;
		m_pTransport->send(json::parse("{\"type\":\"login\",\"data\":{\"username\":\"test\",\"password\":\"test\"}}"));
	}

	void Peer::onClosed() {

	}

	void Peer::onMessage(json message) {
       std::cout<<"[Peer] onMessage:"<<message.dump()<<std::endl;
	}

	void Peer::onDisconnected() {

	}

	void Peer::onFailed() {
// 		if (this->m_closed)
// 			return;
// #if PROTOO_LOG_ENABLE
// 		std::cout << "[Peer] emit failed\n" << endl;
// #endif
// 		this->m_connected = false;
// 		this->m_emitter.emit("failed", nullptr);
	}

	void Peer::handleRequest(json request) {
// 		std::shared_ptr<PROTOO_MSG> pmsg(new PROTOO_MSG);
// 		//message
// 		pmsg->message = request;
// #if PROTOO_LOG_ENABLE
// 		std::cout << "[Peer] handleRequest from server request=" << request.dump(4) << endl;
// #endif
// 		//accept
// 		pmsg->accept = [&, request](json data) {
// 			std::cout << "[Peer] handleRequest accept" << request.dump(4) << endl;
// 			auto response = Message::createSuccessResponse(request, data);
// 			this->m_pTransport->send(response);
// 		};
// 		//reject
// 		pmsg->reject = [&, request](int errorCode, std::string errorReason) {
// 			auto response = Message::createErrorResponse(request, errorCode, errorReason);
// 			this->m_pTransport->send(response);

// 		};
// 		m_emitter.emit("request", pmsg);
	}

	void Peer::handleResponse(json response) {

	}

	void Peer::handleNotification(json notification) {
		// std::shared_ptr<PROTOO_MSG> pmsg(new PROTOO_MSG);
		// pmsg->message = notification;
		// this->m_emitter.emit("notification", pmsg);
	}
}
