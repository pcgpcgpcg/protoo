#ifndef MESSAGE_H
#define MESSAGE_H
#include <string>
#include "json.hpp"

using namespace std;
using namespace nlohmann;
namespace protoo {
  class Message{
  public:
    static json parse(string raw);
    static json createRequest(string method,json data);
    static json createSuccessResponse(json request,json data);
    static json createErrorResponse(json request,int errorCode,string errorReason);
    static json createNotification(string method, json data);
  };

}

#endif
