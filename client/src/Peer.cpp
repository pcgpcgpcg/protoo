#include "Peer.h"
#include "Message.h"

using namespace std;
using namespace nlohmann;

namespace protoo {

	Peer::Peer(string url) {
		m_closed = false;
		m_connected = false;
		//自己new一个transport
		//m_pTransport.reset(new WebSocketTransport(url, json(), this));

	}
	Peer::~Peer() {

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
		//this->m_pTransport->close();
		//this->m_emitter.emit("close", nullptr);
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
// #ifdef _WIN32
// 		OutputDebugString("request");
// #endif
// 		auto request = Message::createRequest(method, data);
// #if PROTOO_LOG_ENABLE
// 		std::cout << "[Peer] send a new request id=:" << request["id"].get<int>() << " method=" << method << endl;
// #endif
// 		//会进入到uWS的线程发送
// 		this->m_pPromise.reset(new std::promise<json>);
// 		//std::promise<json> promise;
// 		this->m_pTransport->send(request);//just like await
// 		//pplx::task_completion_event<json> tce;
// 		//m_request_task_queue.push(tce);
// 		//Timer timer = Timer();
// 		auto timeout = 1500 * (15 + (0.1 * this->m_sents.size()));

// 		std::shared_ptr<SENT_MSG> sent(new SENT_MSG);
// 		sent->id = request["id"].get<int>();
// 		sent->method = request["method"].get<std::string>();
// 		//capture all reference and capture value of request
// 		sent->resolve = [&, request](json data2) {
// #if PROTOO_LOG_ENABLE
// 			std::cout << "[Peer] request resolved id=:" << request["id"].get<int>() << " data2=" << data2 << endl;
// #endif

// 			auto sent_element = this->m_sents.find(request["id"].get<int>());
// 			if (sent_element == m_sents.end()) {
// #if PROTOO_LOG_ENABLE
// 				std::cout << "[Peer] request id not found in map!\n" << std::endl;
// #endif

// 				return;
// 	}
// 			this->m_sents.erase(sent_element);
// 			this->mTimer.stop();
// 			this->m_pPromise->set_value(data2);
// };

// 		sent->reject = [&, request](string errorInfo) {
// #if PROTOO_LOG_ENABLE
// 			std::cout << "[Peer] request reject id=:" << request["id"].get<int>() << " errorInfo=" << errorInfo << endl;
// #endif

// 			auto sent_element = this->m_sents.find(request["id"].get<int>());
// 			if (sent_element == m_sents.end()) {
// 				std::cout << "[Peer] reject request id not found in map!\n" << std::endl;
// 				return;
// 			}
// 			this->m_sents.erase(sent_element);
// 			this->mTimer.stop();
// 			this->m_pPromise->set_exception(std::make_exception_ptr(PeerError(errorInfo.c_str())));
// 		};

// 		sent->close = [&]() {
// 			this->mTimer.stop();
// 		};
// 		//超时处理
// 		mTimer.setTimeout([&, request]() {
// #if PROTOO_LOG_ENABLE
// 			std::cout << "[Peer] request request timeout request id=" << request["id"].get<int>() << std::endl;
// #endif

// 			auto sent_element = this->m_sents.find(request["id"].get<int>());
// 			if (sent_element == m_sents.end()) {
// #if PROTOO_LOG_ENABLE
// 				std::cout << "[Peer] request id not found in map!\n" << std::endl;
// #endif

// 				return;
// 			}
// 			this->m_sents.erase(sent_element);
// 			this->m_pPromise->set_exception(std::make_exception_ptr(PeerError("peer request Time out error")));
// 			}, timeout);

// #if PROTOO_LOG_ENABLE
// 		std::cout << "[Peer] insert into m_sents request id=" << request["id"].get<int>() << std::endl;
// #endif // PROTOO_LOG_ENABLE

// 		this->m_sents[request["id"].get<int>()] = sent;
// #if PROTOO_LOG_ENABLE
// 		std::cout << "[Peer] after insert m_sents.size =" << m_sents.size() << std::endl;
// #endif 

// 		return this->m_pPromise->get_future();
	}

	void Peer::notify(string method, json data) {
		auto request = Message::createNotification(method, data);
		//this->m_pTransport->send(request);
	}


	void Peer::onOpen() {
// 		this->m_connected = true;
// #if PROTOO_LOG_ENABLE
// 		std::cout << "[Peer] emit open\n" << endl;
// #endif
// 		this->m_emitter.emit("open", nullptr);
	}

	void Peer::onClosed() {
// 		if (this->m_closed)
// 			return;
// 		this->m_closed = true;
// #if PROTOO_LOG_ENABLE
// 		std::cout << "[Peer] emit close!" << endl;
// #endif
// 		this->m_connected = false;
// 		this->m_emitter.emit("close", nullptr);
	}

	void Peer::onMessage(json message) {
// #if PROTOO_LOG_ENABLE
// 		std::cout << "[Peer] Peer::onMessage!\n" << message << std::endl;
// #endif
// 		if (message["request"].is_boolean()) {
// 			this->handleRequest(message);
// 		}
// 		else if (message["response"].is_boolean()) {
// 			this->handleResponse(message);
// 		}
// 		else if (message["notification"].is_boolean()) {
// 			this->handleNotification(message);
// 		}
	}

	void Peer::onDisconnected() {
// 		if (this->m_closed)
// 			return;
// #if PROTOO_LOG_ENABLE
// 		std::cout << "[Peer] emit disconnected\n" << endl;
// #endif
// 		this->m_connected = false;
// 		this->m_emitter.emit("disconnected", nullptr);
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
// #if PROTOO_LOG_ENABLE
// 		std::cout << "[Peer] handleResponse response=" << response << std::endl;
// #endif
// 		std::shared_ptr<PROTOO_MSG> pmsg(new PROTOO_MSG);
// 		pmsg->message = response;

// 		auto sent_element = this->m_sents.find(response["id"].get<int>());
// 		if (sent_element == m_sents.end()) {
// #if PROTOO_LOG_ENABLE
// 			std::cout << "[Peer] response id not found in map!\n" << std::endl;
// #endif
// 			return;
// 		}
// 		auto sent = sent_element->second;

// 		if (response.contains("ok"))
// 		{
// 			if (response["ok"].get<bool>())
// 				sent->resolve(response["data"]);
// 		}
// 		else
// 		{
// 			auto error = "error response!";
// 			sent->reject(error);
// 		}
	}

	void Peer::handleNotification(json notification) {
		// std::shared_ptr<PROTOO_MSG> pmsg(new PROTOO_MSG);
		// pmsg->message = notification;
		// this->m_emitter.emit("notification", pmsg);
	}
}
