#include "../../include/network/LWSClient.h"

static int lws_client_callback( struct lws *wsi,enum lws_callback_reasons reason,void *user,void *in,size_t len);
/**
 * 会话上下文对象，结构根据需要自定义
 */
struct session_data {
    int msg_count;
    unsigned char buf[LWS_PRE + MAX_PAYLOAD_SIZE];
    int len;
};

/**
 * 支持的WebSocket子协议数组
 * 子协议即JavaScript客户端WebSocket(url, protocols)第2参数数组的元素
 * 你需要为每种协议提供回调函数
 */
struct lws_protocols protocols[] = {
    {
        "ws", //Sec-Websockets-Protocol
        lws_client_callback, //协议回调
        sizeof( struct session_data ), //自定义数据空间大小：每个ws连接均会分配一个自定义数据空间
        MAX_PAYLOAD_SIZE,
    },
    {
        NULL, NULL,   0 // 最后一个元素固定为此格式
    }
};

static struct ConnectionInfo {
    lws_sorted_usec_list_t    sul;         //schedule connection retry
    struct lws_client_connect_info *conn_info; //connection informations
    struct lws        *wsi;         //related wsi if any
    uint16_t        retry_count; //count of consequetive retries
    int interrupted;
} g_connInfo;

static const uint32_t backoff_ms[] = { 1000, 2000, 3000, 4000, 5000 };
static const lws_retry_bo_t retry = {
    .retry_ms_table            = backoff_ms,
    .retry_ms_table_count        = LWS_ARRAY_SIZE(backoff_ms),
    .conceal_count            = LWS_ARRAY_SIZE(backoff_ms),

    .secs_since_valid_ping        = 400,  /* force PINGs after secs idle */
    .secs_since_valid_hangup    = 400, /* hangup after secs idle */

    .jitter_percent            = 2,
};
//sets a validity regime of testing validity with PING every 3s and failing if it didn't get the PONG back within 10s.
//static const lws_retry_bo_t retry = {
//    .secs_since_valid_ping = 3, //
//    .secs_since_valid_hangup = 10,
//};

//断线重连实现
static void connect_client(lws_sorted_usec_list_t *sul)
{
    struct ConnectionInfo *cinfo = lws_container_of(sul, struct ConnectionInfo, sul);
    if(!cinfo){
        return;
    }

    cinfo->conn_info->pwsi = &(cinfo->wsi);
    cinfo->conn_info->retry_and_idle_policy = &retry;
    cinfo->conn_info->userdata = cinfo;
    
    if (!lws_client_connect_via_info(cinfo->conn_info)){
        //Failed... schedule a retry... we can't use the _retry_wsi() convenience wrapper api here because no valid wsi at this point.
        if (lws_retry_sul_schedule(cinfo->conn_info->context, 0, sul, &retry,
                                   connect_client, &cinfo->retry_count)){
            lwsl_err("%s: connection attempts exhausted\n", __func__);
            cinfo->interrupted = 1;
            return -1;
        }
    }
    
    return 1;
    
//        i.context = context;
//        i.port = 443;
//        i.address = "fstream.binance.com";
//        i.path = "/";
//        i.host = i.address;
//        i.origin = i.address;
//        i.ssl_connection = LCCSCF_USE_SSL | LCCSCF_PRIORITIZE_READS;
//        i.protocol = NULL;
//        i.local_protocol_name = "lws-minimal-client";
//        i.pwsi = &mco->wsi;
//        i.retry_and_idle_policy = &retry;
//        i.userdata = mco;
 
    // 下面的调用触发LWS_CALLBACK_PROTOCOL_INIT事件
    // 创建一个客户端连接
//    m_wsi = lws_client_connect_via_info( &m_connInfo );
//    if(!m_wsi){
//        /*
//         * Failed... schedule a retry... we can't use the _retry_wsi()
//         * convenience wrapper api here because no valid wsi at this
//         * point.
//         */
//        uint16_t retrycount = 10;
//        Connect1 conn = &LWSClient::Connect;
//        //传递lambda函数是否可以
//        if (lws_retry_sul_schedule(m_context, 0, sul, &retry,
//                                   this->*conn, &retrycount)) {
//            lwsl_err("%s: connection attempts exhausted\n", __func__);
//            m_interrupted = true;
//        }
//        return -1;
//    }
//
//    return 1;
}

/**
 * 某个协议下的连接发生事件时，执行的回调函数
 *
 * wsi：指向WebSocket实例的指针
 * reason：导致回调的事件
 * user 库为每个WebSocket会话分配的内存空间
 * in 某些事件使用此参数，作为传入数据的指针
 * len 某些事件使用此参数，说明传入数据的长度
 */
static int lws_client_callback( struct lws *wsi,
                enum lws_callback_reasons reason, 
                void *user, 
                void *in, 
                size_t len )
{
    struct session_data *data = (struct session_data *) user;

    struct ConnectionInfo *cinfo = (struct ConnectionInfo *)user;
    switch ( reason ) {
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            lwsl_err("CLIENT_CONNECTION_ERROR: %s\n",
                         in ? (char *)in : "(null)");
            goto do_retry;
                    break;

        case LWS_CALLBACK_CLIENT_ESTABLISHED:   // 连接到服务器后的回调
            lwsl_notice( "Connected to server ok!\n" );
            cinfo->wsi = wsi;
            lws_callback_on_writable(wsi);
            break;
 
        case LWS_CALLBACK_CLIENT_RECEIVE:       // 接收到服务器数据后的回调，数据为in，其长度为len
            lwsl_notice( "Rx: %s\n", (char *) in );
            break;

        case LWS_CALLBACK_CLIENT_WRITEABLE:     // 当此客户端可以发送数据时的回调
//            if ( data->msg_count < 3 ) {
//                // 前面LWS_PRE个字节必须留给LWS
//                memset( data->buf, 0, sizeof( data->buf ));
//                char *msg = (char *) &data->buf[ LWS_PRE ];
//                data->len = sprintf( msg, "你好 %d", ++data->msg_count );
//                lwsl_notice( "Tx: %s\n", msg );
//                // 通过WebSocket发送文本消息
//                lws_write( wsi, &data->buf[ LWS_PRE ], data->len, LWS_WRITE_TEXT );
//            }
            break;
        case LWS_CALLBACK_CLIENT_CLOSED:
            goto do_retry;
        case LWS_CALLBACK_PROTOCOL_DESTROY:
            lws_sul_cancel(&cinfo->sul);
            break;

        default:
            break;
    }
    
    return lws_callback_http_dummy(wsi, reason, user, in, len);
    
do_retry: //retry the connection to keep it nailed up,For this example, we try to conceal any problem for one set of backoff retries and then exit the app.If you set retry.conceal_count to be LWS_RETRY_CONCEAL_ALWAYS,it will never give up and keep retrying at the last backoff delay plus the random jitter amount.
    if (lws_retry_sul_schedule_retry_wsi(wsi, &cinfo->sul, connect_client,
                         &cinfo->retry_count)) {
        lwsl_err("%s: connection attempts exhausted\n", __func__);
        cinfo->interrupted = 1;
    }

    return 0;
}

/*
构造函数
*/
LWSClient::LWSClient(char* inputUrl){
    // Parse the input url (e.g. wss://echo.websocket.org:1234/test)
    //   the protocol (wss)
    //   the address (echo.websocket.org)
    //   the port (1234)
    //   the path (/test)
    const char *urlProtocol = NULL, *urlTempPath = NULL;
    char urlPath[300]; // The final path string
    //   https://gist.github.com/iUltimateLP/17604e35f0d7a859c7a263075581f99a
    memset(&m_connInfo, 0, sizeof(m_connInfo));
    int port = 0;
    const char* address = NULL;
    //char tmp_url[512] = {0};
    char *tmp_url = new char[1024];
    memset(tmp_url, 0, 1024);
    strncpy(tmp_url,inputUrl,1024);
    if (lws_parse_uri((char*)tmp_url, &urlProtocol, &address, &port, &urlTempPath))
    {
        printf("Couldn't parse URL\n");
    }
    m_connInfo.address = address;
    m_connInfo.port = port;
    m_connInfo.path = "/";
    m_connInfo.host = m_connInfo.address; //just set to the server address
    m_connInfo.origin = m_connInfo.address; //just set to the server address
    m_connInfo.protocol = protocols[0].name;
    m_ctxInfo.ssl_ca_filepath = NULL;
    m_ctxInfo.ssl_cert_filepath = NULL;
    m_ctxInfo.ssl_private_key_filepath = NULL;
    if(!strcmp(urlProtocol,"wss")){
        m_connInfo.ssl_connection = 1;
    }else{
        m_connInfo.ssl_connection = 0;
    }
    // Fix up the urlPath by adding a / at the beginning, copy the temp path, and add a \0 at the end
    urlPath[0] = '/';
    strncpy(urlPath + 1, urlTempPath, sizeof(urlPath) - 2);
    urlPath[sizeof(urlPath) - 1] = '\0';
    m_connInfo.path = urlPath;
    delete[] tmp_url;
}

/*
析构函数
*/
LWSClient::~LWSClient()
{
    if(m_context){
        lws_context_destroy(m_context);
        m_context = NULL;
    }
    lwsl_notice("析构完成\n");
}

/*
拷贝构造
*/
LWSClient::LWSClient(const LWSClient &obj)
{
    
}

/*
init lwsclient
*/
void LWSClient::Init(uv_loop_t* loop)
{
    //uv_loop_t* loop;
    //uv_loop_init(loop);
    m_ctxInfo.port = CONTEXT_PORT_NO_LISTEN;
    m_ctxInfo.iface = NULL;//which ethernet such as eth1、eth2,set to NULL can listen all eth
    m_ctxInfo.protocols = protocols;
    m_ctxInfo.gid = -1;
    m_ctxInfo.uid = -1;
    //m_ctxInfo.options |= LWS_SERVER_OPTION_LIBUV;//maybe just for vhost
    if (loop)
    {
        m_ctxInfo.foreign_loops = (void **)(&loop);
    }
}

/*
设置ssl
*/
//int LWSClient::SetSSL(const char* ca_filepath,
//                            const char* server_cert_filepath,
//                            const char*server_private_key_filepath)
//{
//    if(!m_isSupportSSL)
//    {
//        m_ctxInfo.ssl_ca_filepath = NULL;
//        m_ctxInfo.ssl_cert_filepath = NULL;
//        m_ctxInfo.ssl_private_key_filepath = NULL;
//    }
//    else
//    {
//        m_ctxInfo.ssl_ca_filepath = ca_filepath;
//        m_ctxInfo.ssl_cert_filepath = server_cert_filepath;
//        m_ctxInfo.ssl_private_key_filepath = server_private_key_filepath;
//        m_ctxInfo.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
//    //m_ctxInfo.options |= LWS_SERVER_OPTION_REQUIRE_VALID_OPENSSL_CLIENT_CERT;
//    }
//    return this->m_isSupportSSL;
//}


//创建客户端
int LWSClient::Create()
{
    // 创建一个WebSocket处理器
    m_context = lws_create_context( &m_ctxInfo );
    if(!m_context)
        return -1;
    //add the context
    m_connInfo.context = m_context;
    g_connInfo.conn_info = &m_connInfo;
    //schedule the first client connection attempt to happen immediately
    lws_sul_schedule(m_context, 0, &g_connInfo.sul, connect_client, 1);
    return 0;
}

//运行客户端
int LWSClient::Run(int* interrupted)
{
    *interrupted = g_connInfo.interrupted;
    //可能需要起一个线程来循环这个情况,需要跟loop关联起来
    int n = lws_service( m_context, 0 );
    /**
     * 下面的调用的意义是：当连接可以接受新数据时，触发一次WRITEABLE事件回调
     * 当连接正在后台发送数据时，它不能接受新的数据写入请求，所有WRITEABLE事件回调不会执行
     */
    //lws_callback_on_writable( m_wsi );
    return n;
}


//销毁
void LWSClient::Destroy()
{
    lws_context_destroy(m_context);
}

void LWSClient::Send(std::string message){
    //https://github.com/maurodelazeri/RaccoonWSClient/blob/master/WsRaccoonClient.cc
    //https://github.com/David-Alderson-Bose/websocket-to-me/blob/master/src/sockie.cpp
}
