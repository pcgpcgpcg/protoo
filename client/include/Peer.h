#ifndef PEER_H
#define PEER_H

#include "json.hpp"
#include "WebSocketTransport.h"
#include <stdexcept>
#include "utils.h"

using namespace std;
using namespace nlohmann;

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

	public:
		event::emitter_t<std::shared_ptr<PROTOO_MSG>> m_emitter;

	private:
		bool m_closed = false;
		unique_ptr<WebSocketTransport> m_pTransport;
		bool m_connected = false;
		json m_data = json({});
		map<int, shared_ptr<SENT_MSG>> m_sents;
		PeerTimer mTimer;
		unique_ptr<std::promise<nlohmann::json>> m_pPromise;
		// Queue to maintain the receive tasks when there are no messages(yet).

	};

}
#endif
