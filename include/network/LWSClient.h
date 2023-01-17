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
#include <iostream>
#include <string>
#include<queue>

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
    bool m_isSupportSSL = {false};
    bool m_interrupted = {false};
    std::queue<std::string> messageQueue; //messages to send
private:
    int pcg_parse_uri(char *p, const char **prot, const char **ads, int *port,
                                 const char **path);
    
public:
    LWSClient(char* inputUrl);
    ~LWSClient();
    LWSClient(const LWSClient &obj);
    void Init(uv_loop_t* loop);
    int SetSSL(const char* ca_filepath,
               const char* server_cert_filepath,
               const char*server_private_key_filepath);
    int Create();
    void Connect(lws_sorted_usec_list_t *sul);
    int Run(int wait_time);
    void Destroy();
    void Send(std::string message);
};


#endif/* __LWS_CLIENT_H__ */
