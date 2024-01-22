#include "HTTPTransport.h"
#include <regex>

static int interrupted = 0;

HTTPTransport::HTTPTransport()
{

    m_protocols[0] = {
        "http",
        [](struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) -> int
        {
            // auto pSelf = (WebSocketTransport*)lws_context_user(lws_get_context(wsi));
            // char buf[LWS_PRE + 1024], *start = &buf[LWS_PRE], *p = start,
            //                           *end = &buf[sizeof(buf) - 1];
            // int n;
                                        // 没有 payload，只发送 HTTP 头部
            unsigned char buffer[LWS_PRE + 512], *p = &buffer[LWS_PRE];
            int n;
            switch (reason)
            {
            case LWS_CALLBACK_CLIENT_ESTABLISHED:
            {
                std::cout << "[HTTPTransport] http connected" << std::endl;
                lws_callback_on_writable(wsi);
                break;
            }
            case LWS_CALLBACK_CLIENT_HTTP_BIND_PROTOCOL:
            {
                std::cout<<"LWS_CALLBACK_CLIENT_HTTP_BIND_PROTOCOL"<<std::endl;
                break;
            }
            case LWS_CALLBACK_CLIENT_WRITEABLE:
            {
                std::cout<<"LWS_CALLBACK_CLIENT_WRITEABLE"<<std::endl;
                // ClientData *clientData = (ClientData *)lws_wsi_user(wsi);
                // if (clientData == nullptr)
                // {
                //     // 错误处理: 无法获取客户端数据
                //     break;
                // }
                // unsigned char *buf = new unsigned char[LWS_PRE + 512];
                // unsigned char *p = &buf[LWS_PRE];
                // int n;

                // if (!clientData->postData.empty())
                // {
                //     n = sprintf((char *)p, "POST %s HTTP/1.1\r\n"
                //                            "Host: %s\r\n"
                //                            "Content-Type: application/x-www-form-urlencoded\r\n"
                //                            "Content-Length: %lu\r\n\r\n"
                //                            "%s",
                //                 clientData->path.c_str(),
                //                 clientData->host.c_str(),
                //                 clientData->postData.length(),
                //                 clientData->postData.c_str());
                // }
                // else
                // {
                //     n = sprintf((char *)p, "POST %s HTTP/1.1\r\n"
                //                            "Host: %s\r\n"
                //                            "Content-Length: 0\r\n\r\n", // 无内容的POST请求
                //                 clientData->path.c_str(),
                //                 clientData->host.c_str());
                // }

                // // Write http request with data to server.
                // lws_write(wsi, p, n, LWS_WRITE_HTTP);

                // delete[] buf;
                break;
            }
            case LWS_CALLBACK_CLIENT_RECEIVE:
            {
                std::cout << "Received data: " << std::string((const char *)in, len) << std::endl;
                break;
            }
            case LWS_CALLBACK_CLOSED:
            {
                std::cout << "LWS_CALLBACK_CLOSED" << std::endl;
                interrupted = 1;
                // 确保删除之前分配的用户数据
                ClientData *clientData = (ClientData *)lws_wsi_user(wsi);
                if (clientData)
                {
                    delete clientData;
                    lws_set_wsi_user(wsi, NULL); // 清除用户数据指针
                }
                break;
            }
            case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            {
                interrupted = 1;
                std::cout << "[HTTPTransport] http connection error" << std::endl;
                lws_cancel_service(lws_get_context(wsi));
                break;
            }

            case LWS_CALLBACK_CLOSED_CLIENT_HTTP:
            {
                std::cout << "LWS_CALLBACK_CLOSED_CLIENT_HTTP" << std::endl;
                break;
            }

            case LWS_CALLBACK_ESTABLISHED_CLIENT_HTTP:
            {
                int status = (int)lws_http_client_http_response(wsi);
                std::cout << "connected with server response:" << status << std::endl;
                break;
            }

            case LWS_CALLBACK_RECEIVE_CLIENT_HTTP_READ:
            {
                std::cout<<"LWS_CALLBACK_RECEIVE_CLIENT_HTTP_READ"<<std::endl;
                return 0; /* don't passthru */
            }

            case LWS_CALLBACK_RECEIVE_CLIENT_HTTP:
            {
                std::cout<<"LWS_CALLBACK_RECEIVE_CLIENT_HTTP"<<std::endl;
                n = sizeof(buffer) - LWS_PRE;
                if (lws_http_client_read(wsi, (char**)&p, &n) < 0)
                	return -1;
                std::cout<<"Received data: " << std::string((const char *)p, n) << std::endl;
                return 0; /* don't passthru */
            }

            case LWS_CALLBACK_COMPLETED_CLIENT_HTTP:
            {
                std::cout<<"LWS_CALLBACK_COMPLETED_CLIENT_HTTP"<<std::endl;
                break;
            }

                /* ...callbacks related to generating the POST... */
            case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
            {
                std::cout<<"LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER"<<std::endl;
                // Tell lws we are going to send the body next...
                if (!lws_http_is_redirected_to_get(wsi))
                {
                    lwsl_user("%s: doing POST flow\n", __func__);
                    lws_client_http_body_pending(wsi, 1);
                    lws_callback_on_writable(wsi);
                }
                else
                    lwsl_user("%s: doing GET flow\n", __func__);
                break;
            }

            case LWS_CALLBACK_CLIENT_HTTP_WRITEABLE:
            {
              std::cout<<"LWS_CALLBACK_CLIENT_HTTP_WRITEABLE"<<std::endl;
            // 添加 POST 请求的 HTTP 头部
            if (lws_add_http_header_status(wsi, HTTP_STATUS_OK, &p, buffer + sizeof(buffer)) ||
                lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_CONTENT_LENGTH,
                                             (unsigned char *)"0", 1, &p, buffer + sizeof(buffer)) ||
                lws_finalize_http_header(wsi, &p, buffer + sizeof(buffer))) {
                    return -1; // 头部添加失败，关闭连接
            }

            // 写入 HTTP 头部
            n = lws_write(wsi, buffer + LWS_PRE, p - (buffer + LWS_PRE), LWS_WRITE_HTTP_HEADERS);
            if (n < 0) {
                lwsl_err("ERROR %d writing HTTP headers to socket\n", n);
                return -1; // 写入失败，关闭连接
            }

            // 如果这是最后的写入操作，通知 lws
            lws_client_http_body_pending(wsi, 0);
            // 请求再次写入，通常用于发送请求体，此处不需要
            // lws_callback_on_writable(wsi);

            break;
            }
            }
            return lws_callback_http_dummy(wsi, reason, user, in, len);
        },
        0,
        LWS_MAX_SMP};
    m_protocols[1] = {NULL, NULL, 0, 0}; // terminator

    runHTTP();
}

HTTPTransport::~HTTPTransport(){
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

void HTTPTransport::sendPostRequest(std::string url, std::string path, std::string body)
{
    interrupted = 0;
    std::regex ex("(http|https)://([^/ :]+):?([^/ ]*)(/?[^ #]*)");
    std::cmatch what;
    std::string protocol = "http";
    std::string domain = "";
    int port = 80;
    if (regex_match(url.c_str(), what, ex))
    {
        protocol = std::string(what[1].first, what[1].second);
        domain = std::string(what[2].first, what[2].second);
        std::string portStr = std::string(what[3].first, what[3].second);
        if (portStr == "") {
            if (protocol == "http") {
                port = 80;
            }
            else {
                port = 443;
            }
        }else{
            port = atoi(portStr.c_str());
        }
    }
    // 创建并发送POST请求
    // 填充struct lws_client_connect_info结构体，定义请求细节
    // 填充连接信息结构体
    struct lws_client_connect_info ccinfo = {0};
    ccinfo.context = m_context;
    ccinfo.address = domain.c_str();/* 服务器地址 */;
    ccinfo.port = port/* 服务器端口 */;
    ccinfo.path = path.c_str()/* 请求的URI */;
    ccinfo.host = ccinfo.address;
    ccinfo.origin = ccinfo.address;
    ccinfo.protocol = m_protocols[0].name;
    ccinfo.method = "POST";
    ccinfo.ssl_connection = 0;//SSL is enabled by default but you can change this to 0 and it will disable SSL

    // 定义并分配自定义数据结构实例并填充数据
    //ClientData *clientData = new ClientData();

    // 请确保在连接结束时或适当时机释放分配的内存
    //clientData->path = path;
    //clientData->host = domain;
    // 设置userdata字段指向我们的自定义数据
    //ccinfo.userdata = clientData;
    ccinfo.userdata = NULL; /* 可以设置为指向请求正文数据的指针 */
    int n = 1;


    // 发起连接
    m_wsi = lws_client_connect_via_info(&ccinfo);
    if (!m_wsi)
    {
        // 错误处理
    }
    while (n>=0 && !interrupted)
		n = lws_service(m_context, 0);
}

void HTTPTransport::sendGetRequest(const std::string &url)
{
    
}

    //TODO should handle exception on connnect
void HTTPTransport::runHTTP()
{
    lws_context_creation_info info;
    memset(&info, 0, sizeof(info));
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = m_protocols;
    info.gid = -1;
    info.uid = -1;
    info.connect_timeout_secs = 10;
    info.fd_limit_per_thread = 1 + 1 + 1;
    if (m_context == nullptr)
    {
        m_context = lws_create_context(&info);
        if (!m_context)
        {
            std::cout << "lws init failed" << std::endl;
            return;
        }
    }
    // 开启一个新的线程，用于websocket连接
    // m_pWsThread = new std::thread([&](){ 
    //     //打印一下当前线程id
    //     std::cout << "[HTTPTransport] http thread id=" << std::this_thread::get_id() << std::endl;
        
    //     while(!m_stopped){
    //         //此处需研究实现重连机制
    //         lws_service(m_context, 0);
    //     }
    //     lws_context_destroy(m_context);
    //     std::cout<<"http thread exit"<<std::endl; });
}
