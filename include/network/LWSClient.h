#ifndef __LWS_CLIENT_H__
#define __LWS_CLIENT_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "libwebsockets.h"
#include "uv.h"
#ifdef __cplusplus
}
#endif

class LWSClient
{
    private:

        char *server_address;

        int port;

    public:

        LWSClient(char *lws_server_address,int port);

        ~LWSClient();

        LWSClient(const lws_client &obj);

        void Init();

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
