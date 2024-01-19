#ifndef PEER_H
#define PEER_H

#include "json.hpp"
#include "WebSocketTransport.h"
#include <stdexcept>
#include "utils.h"

using namespace std;
using namespace nlohmann;

//定义回调类型
using AcceptCallback = std::function<void(const std::string &)>;							 // 本地接受服务器的request
using RejectCallback = std::function<void(int, const std::string &)>;						 // 本地拒绝服务器的request
using onServerRequestCB = std::function<void(const json &, AcceptCallback, RejectCallback)>; // 本地接受服务器的request
using onServerNotificationCB = std::function<void(const json &)>;							 // 本地接受服务器的notification
using onPeerOpenCB = std::function<void()>;
using onPeerClosedCB = std::function<void()>;
using onPeerDisconnectedCB = std::function<void()>;
using onPeerFailedCB = std::function<void()>;

struct PROTOO_MSG {
	json message;
	function<void(json)> accept;
	function<void(int, string)> reject;//error code and error desc
	function<void(std::string)> Event;//event with json message as param
};

struct SENT_MSG {
	int id;
	string method;
	function<void(json)> resolve;
	function<void(string)> reject;
	function<void()> close;
};

struct SentRequest {
	std::string id;
	std::string method;
	shared_ptr<promise<json>> response;
};

namespace protoo {
	class PeerError : public std::runtime_error
	{
	public:
		explicit PeerError(const char* description);
	};

	/* Inline methods. */

	inline PeerError::PeerError(const char* description)
		: std::runtime_error(description)
	{
	}

	class Peer : public WebSocketTransport::TransportListener {
	public:
		Peer(string url);
		~Peer();
		/* Virtual methods inherited from WebsocketTransport::TransportListener. */
	public:
		void onOpen() override;
		void onClosed() override;
		void onMessage(json message) override;
		void onDisconnected() override;
		void onFailed() override;

	public:
		bool closed();
		bool connected();
		json data();
		void setData(json data);
		void close();
		//void requestTest(string method, json data);
		std::future<json> request(string method, json data = json({}));
		void notify(string method, json data = json({}));

	private:
		//void handleTransport();
		void handleRequest(json request);
		void handleResponse(json response);
		void handleNotification(json notification);

	// public:
	// 	event::emitter_t<std::shared_ptr<PROTOO_MSG>> m_emitter;

	private:
		bool m_closed = false;
		unique_ptr<WebSocketTransport> m_pTransport;
		bool m_connected = false;
		json m_data = json({});
		map<int, shared_ptr<SENT_MSG>> m_sents;
		std::map<int, std::shared_ptr<std::promise<json>>> m_promises;
		public:
		onPeerOpenCB onPeerOpen;
		onPeerClosedCB onPeerClosed;
		onPeerDisconnectedCB onPeerDisconnected;
		onPeerFailedCB onPeerFailed;
		onServerRequestCB onServerRequest;
		onServerNotificationCB onServerNotification;

	};

}
#endif
