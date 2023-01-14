#ifndef __LWS_CLIENT_H__
#define __LWS_CLIENT_H__

//#ifdef __cplusplus
//extern "C" {
//#endif
#include "libwebsockets.h"
#include "uv.h"
//#ifdef __cplusplus
//}
//#endif

#define MAX_PAYLOAD_SIZE 1024

class LWSClient
{
private:
    //char *server_address;
    //int port;
    lws_context_creation_info m_ctxInfo = {0};
    lws_context *m_context = {NULL};
    lws_client_connect_info m_connInfo={0};
    lws *m_wsi = {NULL};
    
public:
    LWSClient(char* inputUrl);
    ~LWSClient();
    LWSClient(const LWSClient &obj);
    void Init(uv_loop_t* loop);
    int SetSSL(const char* ca_filepath,
               const char* server_cert_filepath,
               const char*server_private_key_filepath,
               bool is_support_ssl);
    int Create();
    int Connect(int is_ssl_support);
    int Run(int wait_time);
    void Destroy();
};


#endif/* __LWS_CLIENT_H__ */
