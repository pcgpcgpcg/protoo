#include "WebSocketTransport.h"
#include "Message.h"
#include <regex>
//#include <cpprest/asyncrt_utils.h>
//using namespace std;
//using namespace nlohmann;
//constructor
#define TRANSPORT_LOG_ENABLE 1

std::condition_variable g_cvOnOpen;
    std::mutex g_mtx;

WebSocketTransport::WebSocketTransport(string url, TransportListener* listener) {
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
        [](struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) -> int
        {
            //auto pSelf = (WebSocketTransport*)lws_context_user(lws_get_context(wsi));
            auto *pSelf = (WebSocketTransport *)user;
            if(!pSelf){
                return 0;
            }
            if (!pSelf->m_listener)
            {
                return 0;
            }
            switch (reason)
            {
            case LWS_CALLBACK_CLIENT_ESTABLISHED:
                std::cout << "websocket connected" << std::endl;
                //pSelf->m_listener->onOpen();
                g_cvOnOpen.notify_one();
                break;
            case LWS_CALLBACK_CLIENT_RECEIVE:
                if (len > 0)
                {
                    pSelf->m_receivedMsg.append((const char *)in, len);
                    if (lws_is_final_fragment(wsi))
                    {
                        std::cout<<"received message:"<<pSelf->m_receivedMsg<<std::endl;
                        // auto jmsg = protoo::Message::parse(pSelf->m_receivedMsg);
                        // pSelf->m_listener->onMessage(jmsg);
                        // pSelf->m_receivedMsg.clear();
                    }
                }
                break;
            case LWS_CALLBACK_CLIENT_WRITEABLE:
                if (!pSelf->m_msgQueue.empty())
                {
                    auto msg = pSelf->m_msgQueue.front();
                    std::cout<<"send message:"<<msg<<std::endl;
                    pSelf->m_msgQueue.pop();
                    auto *p = (unsigned char *)malloc(LWS_SEND_BUFFER_PRE_PADDING + msg.length() + LWS_SEND_BUFFER_POST_PADDING);
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
            std::cout<<"websocket closed"<<std::endl;
                pSelf->m_wsClient = nullptr;
                pSelf->m_listener->onClosed();
                break;
            case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            std::cout<<"websocket connection error"<<std::endl;
                pSelf->m_listener->onFailed();
                break;
            case LWS_CALLBACK_WSI_DESTROY:
            std::cout<<"websocket connection destroy"<<std::endl;
                cout << "[ws Connection destruction]" << endl;
                pSelf->m_wsClient = nullptr;
                break;

            default:
                break;
            }
            return 0;
        },
        0,
        1024,
        0,
        this,
        10};
         m_protocols[1] = {nullptr, nullptr, 0, 0};
    //lauch websocket connection
	runWebSocket();
    //等待连接成功的消息
    std::unique_lock<std::mutex> lk(g_mtx);
    g_cvOnOpen.wait(lk);
    //接收到了信号，说明连接成功了 TODO:此处需要加个超时机制
    int a= 1;
    a++;
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
    auto msg = message.dump();
	m_msgQueue.push(msg);
    m_noMsg = false;
    if(m_wsClient){
        //打印一下当前线程信息
        std::cout << "[WebSocketTransport] send thread id=" << std::this_thread::get_id() << std::endl;
        lws_callback_on_writable(m_wsClient);
    }
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
        std::cout << "[WebSocketTransport] websocket thread id=" << std::this_thread::get_id() << std::endl;
        lws_context_creation_info info;
        memset(&info, 0, sizeof(info));
        info.port = CONTEXT_PORT_NO_LISTEN;
        info.iface = NULL;
        info.protocols = m_protocols;
        info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
        info.fd_limit_per_thread = 1024;
        info.ssl_cert_filepath = NULL;
        info.ssl_private_key_filepath = NULL;
        info.gid = -1;
        info.uid = -1;
        info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
        //info.ws_ping_pong_interval = 10;
        info.ka_time = 10;
        info.ka_probes = 10;
        info.ka_interval = 10;
        if(m_context == nullptr){
            m_context = lws_create_context(&info);
            if(!m_context){
                std::cout << "lws init failed" << std::endl;
                return;
            }
        }
        lws_client_connect_info ccinfo = {0};
        memset(&ccinfo, 0, sizeof(ccinfo));
        ccinfo.context = m_context;
        ccinfo.ssl_connection = 0;//0 will disable ssl and enable ssl set the follow: LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED | LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK;
        ccinfo.host = m_url.c_str();
        ccinfo.address = m_url.c_str();
        ccinfo.port = m_port;
        ccinfo.path = m_path.c_str();
        ccinfo.origin = m_url.c_str();
        ccinfo.protocol = m_protocols[0].name;

        ccinfo.userdata = this;
        m_wsClient = lws_client_connect_via_info(&ccinfo);
        while(!m_stopped && m_wsClient){
            //此处需研究实现重连机制
            lws_service(m_context, 50);
        }
        lws_context_destroy(m_context);
        m_wsClient = nullptr;
        std::cout<<"websocket thread exit"<<std::endl;
    });
}

    void WebSocketTransport::scheduleTask(int afterMs, std::function<void()> task) {
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
            // 执行任务
            // Retrieve the WebSocketTransport instance from the sul
            WebSocketTransport* self = lws_container_of(sul, WebSocketTransport, m_sul);
            // Execute the saved task
            if (self->m_task) {
                self->m_task();
            }
        }, delay);
    }

    // 新的方法：取消定时任务
    void WebSocketTransport::cancelScheduledTask() {
        if (!m_context) {
            std::cerr << "Cannot cancel scheduled task: LWS context not created." << std::endl;
            return;
        }

        // 使用 lws_sul_cancel 来取消定时任务
        lws_sul_cancel(&m_sul);
    }

void WebSocketTransport::handleMessages(std::string message) {
	auto jmsg = protoo::Message::parse(message);
	m_listener->onMessage(jmsg);
}