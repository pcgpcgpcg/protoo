#include "LWSClient.h"

static struct lws_context_creation_info ctx_info = { 0 };
static struct lws_context *context = NULL;
static struct lws_client_connect_info conn_info = { 0 };
static struct lws *wsi = NULL;

/**
 * 会话上下文对象，结构根据需要自定义
 */
struct session_data {
    int msg_count;
    unsigned char buf[LWS_PRE + MAX_PAYLOAD_SIZE];
    int len;
};

/**
 * 某个协议下的连接发生事件时，执行的回调函数
 *
 * wsi：指向WebSocket实例的指针
 * reason：导致回调的事件
 * user 库为每个WebSocket会话分配的内存空间
 * in 某些事件使用此参数，作为传入数据的指针
 * len 某些事件使用此参数，说明传入数据的长度
 */
int lws_client_callback( struct lws *wsi, 
                enum lws_callback_reasons reason, 
                void *user, 
                void *in, 
                size_t len )
{
    struct session_data *data = (struct session_data *) user;

    switch ( reason ) {

        case LWS_CALLBACK_CLIENT_ESTABLISHED:   // 连接到服务器后的回调
            lwsl_notice( "Connected to server ok!\n" );
            break;
 
        case LWS_CALLBACK_CLIENT_RECEIVE:       // 接收到服务器数据后的回调，数据为in，其长度为len
            lwsl_notice( "Rx: %s\n", (char *) in );
            break;

        case LWS_CALLBACK_CLIENT_WRITEABLE:     // 当此客户端可以发送数据时的回调
            if ( data->msg_count < 3 ) {
                // 前面LWS_PRE个字节必须留给LWS
                memset( data->buf, 0, sizeof( data->buf ));
                char *msg = (char *) &data->buf[ LWS_PRE ];
                data->len = sprintf( msg, "你好 %d", ++data->msg_count );
                lwsl_notice( "Tx: %s\n", msg );
                // 通过WebSocket发送文本消息
                lws_write( wsi, &data->buf[ LWS_PRE ], data->len, LWS_WRITE_TEXT );
            }
            break;

        default:
            break;
//            lwsl_notice("not support\n");
    }

    return 0;
}

/**
 * 支持的WebSocket子协议数组
 * 子协议即JavaScript客户端WebSocket(url, protocols)第2参数数组的元素
 * 你需要为每种协议提供回调函数
 */
struct lws_protocols protocols[] = {
    {
        //协议名称，协议回调，接收缓冲区大小
        "ws", lws_client_callback, sizeof( struct session_data ), MAX_PAYLOAD_SIZE,
    },
    {
        NULL, NULL,   0 // 最后一个元素固定为此格式
    }
};

/*
构造函数
*/
LWSClient::LWSClient(char *_address,int _port)
{
    server_address = _address;
    port = _port;
}

/*
析构函数
*/
LWSClient::~LWSClient()
{
    lwsl_notice("析构完成\n");
}

/*
拷贝构造
*/
LWSClient::LWSClient(const lws_client &obj)
{
    
}

/*
init lwsclient
*/
void LWSClient::init(v_loop_t* loop)
{
    //uv_loop_t* loop;
    //uv_loop_init(loop);
    ctx_info.port = CONTEXT_PORT_NO_LISTEN;
    ctx_info.iface = NULL;
    ctx_info.protocols = protocols;
    ctx_info.gid = -1;
    ctx_info.uid = -1;
    if (loop)
    {
        ctx_info.foreign_loops = (void **)(&loop);
    }
}

/*
设置ssl
*/
int LWSClient::set_ssl(const char* ca_filepath, 
                            const char* server_cert_filepath,
                            const char*server_private_key_filepath,
                            bool is_support_ssl)
{
    if(!is_support_ssl)
    {
        ctx_info.ssl_ca_filepath = NULL;
        ctx_info.ssl_cert_filepath = NULL;
        ctx_info.ssl_private_key_filepath = NULL;
    }
    else
    {
        ctx_info.ssl_ca_filepath = ca_filepath;
        ctx_info.ssl_cert_filepath = server_cert_filepath;
        ctx_info.ssl_private_key_filepath = server_private_key_filepath;
        ctx_info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    //ctx_info.options |= LWS_SERVER_OPTION_REQUIRE_VALID_OPENSSL_CLIENT_CERT;
    }

    return is_support_ssl;
}

/*
创建客户端
*/
int LWSClient::create()
{
    // 创建一个WebSocket处理器
    context = lws_create_context( &ctx_info );
    if(!context)
        return -1;
    return 0;
}

/*
连接客户端
*/
int LWSClient::connect(int is_ssl_support)
{
    char addr_port[256] = { 0 };
    sprintf(addr_port, "%s:%u", server_address, port & 65535 );
 
    // 客户端连接参数
    conn_info = { 0 };
    conn_info.context = context;
    conn_info.address = server_address;
    conn_info.port = port;

    if(!is_ssl_support)
        conn_info.ssl_connection = 0;
    else
        conn_info.ssl_connection = 1;
    conn_info.path = "./";
    conn_info.host = addr_port;
    conn_info.origin = addr_port;
    conn_info.protocol = protocols[ 0 ].name;
 
    // 下面的调用触发LWS_CALLBACK_PROTOCOL_INIT事件
    // 创建一个客户端连接
    wsi = lws_client_connect_via_info( &conn_info );
    if(!wsi)
        return -1;
    return 1;
}

/*
运行客户端
*/
int LWSClient::run(int wait_time)
{
    lws_service( context, wait_time );
    /**
     * 下面的调用的意义是：当连接可以接受新数据时，触发一次WRITEABLE事件回调
     * 当连接正在后台发送数据时，它不能接受新的数据写入请求，所有WRITEABLE事件回调不会执行
     */
    lws_callback_on_writable( wsi );
}

/*
销毁
*/
void LWSClient::destroy()
{
    lws_context_destroy(context);
}