#include <iostream>
#include "../include/student.h"
#include "../include/network/LWSClient.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "libwebsockets.h"
#include "uv.h"
#ifdef __cplusplus
}
#endif


static volatile int exit_sig = 0;
static struct lws_context *context;
static struct lws *client_wsi;
static int interrupted, port = 443, ssl_connection = LCCSCF_USE_SSL;
static const char *server_address = "libwebsockets.org", *pro = "lws-mirror-protocol";
static lws_sorted_usec_list_t sul;

static const lws_retry_bo_t retry = {
    .secs_since_valid_ping = 3,
    .secs_since_valid_hangup = 10,
};

static void
connect_cb(lws_sorted_usec_list_t *_sul)
{
    struct lws_client_connect_info i;

    lwsl_notice("%s: connecting\n", __func__);

    memset(&i, 0, sizeof(i));

    i.context = context;
    i.port = port;
    i.address = server_address;
    i.path = "/";
    i.host = i.address;
    i.origin = i.address;
    i.ssl_connection = ssl_connection;
    i.protocol = pro;
    i.alpn = "h2;http/1.1";
    i.local_protocol_name = "lws-ping-test";
    i.pwsi = &client_wsi;
    i.retry_and_idle_policy = &retry;

    if (!lws_client_connect_via_info(&i))
        lws_sul_schedule(context, 0, _sul, connect_cb, 5 * LWS_USEC_PER_SEC);
}

static int
callback_minimal_pingtest(struct lws *wsi, enum lws_callback_reasons reason,
             void *user, void *in, size_t len)
{
    switch (reason) {
    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        lwsl_err("CLIENT_CONNECTION_ERROR: %s\n",
             in ? (char *)in : "(null)");
        lws_sul_schedule(context, 0, &sul, connect_cb, 5 * LWS_USEC_PER_SEC);
        break;

    case LWS_CALLBACK_CLIENT_ESTABLISHED:
        lwsl_user("%s: established\n", __func__);
        break;

    default:
        break;
    }

    return lws_callback_http_dummy(wsi, reason, user, in, len);
}

static const struct lws_protocols protocols[] = {
    {
        "lws-ping-test",
        callback_minimal_pingtest,
        0, 0, 0, NULL, 0
    },
    LWS_PROTOCOL_LIST_TERM
};

void sigint_handler(int sig)
{
    interrupted = 1;
}

int main(int argc, char *argv[]){
    signal(SIGINT, sigint_handler);
    std::cout<<"hello world" << std::endl;
    Student s("joe");
    s.display();
    
    LWSClient client("ws://152.136.16.141:13000/");
    client.Init(NULL);
    //client.SetSSL(NULL,NULL,NULL);
    client.Create();
    //client.Connect(0);
    int n = 0;
    while(n >= 0 && !interrupted)
        n = client.Run(&interrupted);

    client.Destroy();

    return 0;
}


//raw test
static void test_lws_client(){
    uv_loop_t* loop;
    uv_loop_init(loop);
    
    lws_context_creation_info info;
    info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    info.port = CONTEXT_PORT_NO_LISTEN; /* we do not run any server */
    info.iface = NULL;
    info.gid = -1;
    info.uid = -1;
    info.protocols = protocols;
    info.fd_limit_per_thread = 1 + 1 + 1;
    info.foreign_loops = (void**)(&loop);
    lws_context *context = lws_create_context(&info);
    if (!context) {
        lwsl_err("lws init failed\n");
        return 1;
    }
    
    char addr_port[256] = { 0 };
    sprintf(addr_port, "%s:%u", server_address, port&65535);
    lws_client_connect_info conn_info = { 0 };
    conn_info.context = context;
    conn_info.address = server_address;
    conn_info.port = port;
    conn_info.ssl_connection = 1;
    conn_info.path = "./";
    conn_info.host =  addr_port;
    conn_info.origin = addr_port;
    conn_info.protocol = protocols[0].name;
    
    lws *wsi = lws_client_connect_via_info(&conn_info);
    while(!exit_sig){
        //run poll, max can wait 1000ms
        lws_service(context, 1000);
        //当连接可以接受新数据时,触发一次writeable事件回调
        //当连接正在后台发送数据时，它不能接受新的输入写入请求，所有writeable事件回调不会执行
        lws_callback_on_writable(wsi);
    }
    //destroy context
    lws_context_destroy(context);
}
