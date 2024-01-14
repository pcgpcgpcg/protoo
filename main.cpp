#include <iostream>
#include "Message.h"
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

    return 0;
}


