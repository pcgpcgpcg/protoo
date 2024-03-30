#include "WebSocketTransport.h"
#include "Message.h"
#include <regex>
#include <fmt/core.h>
#include <fmt/color.h>
//#include <cpprest/asyncrt_utils.h>
//using namespace std;
//using namespace nlohmann;
//constructor
#define TRANSPORT_LOG_ENABLE 1

    static const uint32_t backoff_ms[] = { 1000, 2000, 3000, 4000, 4000 };

    static const lws_retry_bo_t retry = {
        .retry_ms_table = backoff_ms,
        .retry_ms_table_count = LWS_ARRAY_SIZE(backoff_ms),
         //If you set retry.conceal_count to be LWS_RETRY_CONCEAL_ALWAYS, it will never give up and keep retrying at the last backoff
        .conceal_count = LWS_RETRY_CONCEAL_ALWAYS , //endless try

        .secs_since_valid_ping = 3,  /* force PINGs after secs idle */
        .secs_since_valid_hangup = 10, /* hangup after secs idle */

        .jitter_percent = 20,
    };

 //   static const uint32_t _rbo_bo_0[] = {
 //1000,  2000,  3000,  5000,  10000,
 //   };
 //   static const lws_retry_bo_t _rbo_0 = {
 //       .retry_ms_table = _rbo_bo_0,
 //       .retry_ms_table_count = 5,
 //       .conceal_count = 5,
 //       .secs_since_valid_ping = 30,
 //       .secs_since_valid_hangup = 35,
 //       .jitter_percent = 20,
 //   };

    //static const uint32_t backoff_ms[] = { 1000, 2000, 3000, 4000, 5000 };

    //static const lws_retry_bo_t retry = {
    //    .retry_ms_table = backoff_ms,
    //    .retry_ms_table_count = LWS_ARRAY_SIZE(backoff_ms),
    //    .conceal_count = LWS_ARRAY_SIZE(backoff_ms),

    //    .secs_since_valid_ping = 3,  /* force PINGs after secs idle */
    //    .secs_since_valid_hangup = 10, /* hangup after secs idle */

    //    .jitter_percent = 20,
    //};

    static void connectClient(lws_sorted_usec_list_t* sul) {
        struct ConnectionInfo* pInfo = lws_container_of(sul, struct ConnectionInfo, sul);
        WebSocketTransport* pWSTransport = reinterpret_cast<WebSocketTransport*>(pInfo->user); // 示例用法
        if (!pWSTransport) {
            return;
        }
		if (pInfo->retry_count > 0) {
			fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold,
				"连接异常，正在尝试重连, 第{}!\n次", pInfo->retry_count);
		}
        lws_client_connect_info ccinfo = { 0 };
        memset(&ccinfo, 0, sizeof(ccinfo));
        ccinfo.context = pWSTransport->m_context;
        ccinfo.ssl_connection = 0;//0 will disable ssl and enable ssl set the follow: LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED | LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK;
        ccinfo.host = pWSTransport->m_url.c_str();
        ccinfo.address = pWSTransport->m_url.c_str();
        ccinfo.port = pWSTransport->m_port;
        ccinfo.path = pWSTransport->m_path.c_str();
        ccinfo.origin = pWSTransport->m_url.c_str();
        ccinfo.protocol = pWSTransport->m_protocols[0].name;
        ccinfo.pwsi = &pInfo->wsi;

        ccinfo.retry_and_idle_policy = &retry;

        ccinfo.userdata = pInfo;
        pWSTransport->m_connectionInfo.wsi = lws_client_connect_via_info(&ccinfo);
        if (!pWSTransport->m_connectionInfo.wsi) {
            /*
             * Failed... schedule a retry... we can't use the _retry_wsi()
             * convenience wrapper api here because no valid wsi at this
             * point.
             */
            if (lws_retry_sul_schedule(pWSTransport->m_context, 0, sul, &retry,
                connectClient, &pInfo->retry_count)) {
                lwsl_err("%s: connection attempts exhausted\n", __func__);
                pWSTransport->m_stopped = true;
            }
        }
    }

    static int
        callback_minimal(struct lws* wsi, enum lws_callback_reasons reason,
            void* user, void* in, size_t len)
    {
        struct ConnectionInfo* pInfo = (struct ConnectionInfo*)user;
        if (!pInfo) {
            return 0;
        }
        WebSocketTransport* pSelf = reinterpret_cast<WebSocketTransport*>(pInfo->user); // 示例用法
        if (!pSelf) {
            return 0;
        }

        switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            std::cout << "websocket connected" << std::endl;
            if (pInfo->retry_count > 0) {
				fmt::print(fg(fmt::color::green) | fmt::emphasis::bold,
                    					"第{}次自动重连成功!", pInfo->retry_count);
                pSelf->m_listener->onReConnected();
            }
            else {
                fmt::print(fg(fmt::color::green) | fmt::emphasis::bold, "websocket连接成功!");
                pSelf->m_listener->onConnected();
            }
           
            break;

        case LWS_CALLBACK_CLIENT_RECEIVE:
            if (len > 0)
            {
                pSelf->m_receivedMsg.append((const char*)in, len);
                if (lws_is_final_fragment(wsi))
                {
                    std::cout << "received message:" << pSelf->m_receivedMsg << std::endl;
                    auto jmsg = protoo::Message::parse(pSelf->m_receivedMsg);
                    pSelf->m_listener->onMessage(jmsg);
                    pSelf->m_receivedMsg.clear();
                }
            }
            break;
        case LWS_CALLBACK_CLIENT_WRITEABLE:
            if (!pSelf->m_msgQueue.empty())
            {
                auto msg = pSelf->m_msgQueue.front();
                std::cout << "send message:" << msg << std::endl;
                pSelf->m_msgQueue.pop();
                auto* p = (unsigned char*)malloc(LWS_SEND_BUFFER_PRE_PADDING + msg.length() + LWS_SEND_BUFFER_POST_PADDING);
                memcpy(p + LWS_SEND_BUFFER_PRE_PADDING, msg.c_str(), msg.length());
                lws_write(wsi, p + LWS_SEND_BUFFER_PRE_PADDING, msg.length(), LWS_WRITE_TEXT);
                free(p);
            }
            else
            {
                pSelf->m_noMsg = true;
                // cout << "Send keep-alive message" << endl;
                // uint8_t ping[LWS_PRE + 125];
                // lws_write(wsi, ping + LWS_PRE, 0, LWS_WRITE_PING);
            }
            break;
        case LWS_CALLBACK_CLIENT_CLOSED:
            std::cout << "websocket closed" << std::endl;
            //pSelf->m_wsClient = nullptr;
            pSelf->m_listener->onClosed();
            goto do_retry;
            break;
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            std::cout << "websocket connection error" << std::endl;
            pSelf->m_listener->onFailed();
            goto do_retry;
            break;
        case LWS_CALLBACK_WSI_DESTROY:
            std::cout << "websocket connection destroy" << std::endl;
            // cout << "[ws Connection destruction]" << endl;
             //pSelf->m_wsClient = nullptr;
            break;

        default:
            break;
        }

        return lws_callback_http_dummy(wsi, reason, user, in, len);

    do_retry:
        /*
         * retry the connection to keep it nailed up
         *
         * For this example, we try to conceal any problem for one set of
         * backoff retries and then exit the app.
         *
         * If you set retry.conceal_count to be LWS_RETRY_CONCEAL_ALWAYS,
         * it will never give up and keep retrying at the last backoff
         * delay plus the random jitter amount.
         */
        fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold,
            "连接异常，正在尝试重连, 第{}!\n次", pInfo->retry_count);
        if (lws_retry_sul_schedule_retry_wsi(wsi, &pInfo->sul, connectClient,
            &pInfo->retry_count)) {
            lwsl_err("%s: connection attempts exhausted\n", __func__);
            pSelf->m_stopped = true;
        }
        return 0;
    }

WebSocketTransport::WebSocketTransport(string url, TransportListener* listener) {
    memset(&m_sul, 0, sizeof(m_sul));
    memset(&m_connectionInfo.sul, 0, sizeof(m_connectionInfo.sul));

   std::regex ex("(ws|wss)://([^/ :]+):?([^/ ]*)(/?[^ #]*)");
    std::cmatch what;
    if (regex_match(url.c_str(), what, ex))
    {
        std::string protocol = std::string(what[1].first, what[1].second);
        std::string domain = std::string(what[2].first, what[2].second);
        std::string port = std::string(what[3].first, what[3].second);
        std::string path = std::string(what[4].first, what[4].second);
        //string query    = string(what[5].first, what[5].second);
        m_prefix = protocol;
        m_path = path;
        m_url = domain;
        if (port == "") {
            if (protocol == "ws") {
                m_port = 80;
            }
            else {
                m_port = 443;
            }
        }
        else {
            m_port = atoi(port.c_str());
        }
    }
    m_closed = false;
    //m_options = options;
    m_listener = listener;
	m_closed = false;
    //set protocols
    m_protocols[0] = {
        "protoo",
       callback_minimal,
        0,
        1024,
        0,
        NULL,
        0};
         m_protocols[1] = LWS_PROTOCOL_LIST_TERM;
    //lauch websocket connection
	runWebSocket();
}

WebSocketTransport::~WebSocketTransport()
{
    m_stopped = true;
    if (m_pWsThread && m_pWsThread->joinable())
    {
        m_pWsThread->join();
    }
    if (m_pWsThread)
    {
        delete m_pWsThread;
        m_pWsThread = nullptr;
    }
    if(m_context){
        lws_context_destroy(m_context);
        m_context = nullptr;
    }
}

bool WebSocketTransport::closed() {
	return m_closed;
}

void WebSocketTransport::close() {
	if (m_closed) {
		return;
	}
	m_closed = true;
	m_listener->onClosed();
}

void WebSocketTransport::send(json message)
{
    this->InvokeTask([this, message](){
         auto msg = message.dump();
	     m_msgQueue.push(msg);
         m_noMsg = false;
         if (m_connectionInfo.wsi)
         {
             lws_callback_on_writable(m_connectionInfo.wsi);
         } 
     });
}

void WebSocketTransport::InvokeTask(std::function<void()> task)
{
    {
        std::lock_guard<std::mutex> lock(tasks_mutex);
        tasks.push(task);
    }
    lws_cancel_service(m_context); // 假设`context`是线程A中的lws_context
}

#ifdef _WIN32

#elif defined __linux__
uint32_t GetCurrentThreadId() {
	return 0;
}

#elif defined __APPLE__
uint32_t GetCurrentThreadId() {
	return 0;
}
#elif defined __unix__
uint32_t GetCurrentThreadId() {
	return 0;
}
#else

#endif

//TODO should handle exception on connnect
void WebSocketTransport::runWebSocket() {
	//开启一个新的线程，用于websocket连接
	m_pWsThread = new std::thread([&]() {
		//打印一下当前线程id
		//std::cout << "[WebSocketTransport] websocket thread id=" << std::this_thread::get_id() << std::endl;
		lws_context_creation_info info;
		memset(&info, 0, sizeof(info));
		info.port = CONTEXT_PORT_NO_LISTEN;
		info.iface = NULL;
		info.protocols = m_protocols;
		info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
		info.fd_limit_per_thread = 1 + 1 + 1;
		info.ssl_cert_filepath = NULL;
		info.ssl_private_key_filepath = NULL;
		info.gid = -1;
		info.uid = -1;
		info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
		//info.ws_ping_pong_interval = 10;
		info.ka_time = 10;
		info.ka_probes = 10;
		info.ka_interval = 10;
		if (m_context == nullptr) {
			m_context = lws_create_context(&info);
			if (!m_context) {
				std::cout << "lws init failed" << std::endl;
				return;
			}
		}
        m_connectionInfo.user = this;
		/* schedule the first client connection attempt to happen immediately */
		lws_sul_schedule(m_context, 0, &m_connectionInfo.sul, connectClient, 1);

		while (!m_stopped && m_nServiceRet >= 0)
		{
			// 此处还要看一下发送队列，如果发送队列里有消息，就要调用lws_callback_on_writable
			//if (!m_msgQueue.empty())
			//{
			//    if (m_wsClient)
			//    {
			//        // 打印一下当前线程信息
			//        std::cout << "[WebSocketTransport] send thread id=" << std::this_thread::get_id() << std::endl;
			//        lws_callback_on_writable(m_wsClient);
			//    }
			//}

			// 此处需研究实现重连机制
			m_nServiceRet = lws_service(m_context, 0);
			// 处理所有调度的任务
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
		}
		lws_context_destroy(m_context);
		std::cout << "websocket thread exit" << std::endl;
		});
}

void WebSocketTransport::scheduleTask(int afterMs, std::function<void()> task)
{
    InvokeTask([this, afterMs, task]()
               {
            if (!m_context) {
                std::cerr << "Cannot schedule task: LWS context not created." << std::endl;
                return;
            }

            // Time conversion to microseconds
            lws_usec_t delay = afterMs * LWS_US_PER_MS;

            // Save task to be called by the static callback
            m_task = task;
            // Setup the callback function to be called after the delay
            lws_sul_schedule(m_context, 0, &m_sul, [](lws_sorted_usec_list_t* sul) {
                // Retrieve the WebSocketTransport instance from the sul
                WebSocketTransport* self = lws_container_of(sul, WebSocketTransport, m_sul);
                // Execute the saved task
                if (self->m_task) {
                    self->m_task();
                }
            }, delay); 
        });
}

void WebSocketTransport::cancelScheduledTask()
{
    InvokeTask([this]()
               {
            if (!m_context) {
                std::cerr << "Cannot cancel scheduled task: LWS context not created." << std::endl;
                return;
            }

            // 使用 lws_sul_cancel 来取消定时任务
            lws_sul_cancel(&m_sul); });
}

void WebSocketTransport::handleMessages(std::string message) {
	auto jmsg = protoo::Message::parse(message);
	m_listener->onMessage(jmsg);
}