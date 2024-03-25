//the qrtm sdk header file
#ifndef QRTM_H
#define QRTM_H
#include <string>
#include <optional>
#include <future>
#include <memory>
#include "Peer.h"
struct RTMConfig {
public:
    std::optional<std::string> serverUrl;
    std::optional<std::string> token;
    std::optional<std::string> encryptionMode;
    std::optional<std::string> cipherKey;
    std::optional<std::vector<uint8_t>> salt;
    std::optional<bool> useStringUserId;
    std::optional<int> presenceTimeout;
    std::optional<bool> logUpload;
    std::optional<std::string> logLevel;
    std::optional<bool> cloudProxy;

    // Default constructor with default arguments
    RTMConfig(
        std::optional<std::string> serverUrl = std::nullopt,
        std::optional<std::string> token = std::nullopt,
        std::optional<std::string> encryptionMode = std::nullopt,
        std::optional<std::string> cipherKey = std::nullopt,
        std::optional<std::vector<uint8_t>> salt = std::nullopt,
        std::optional<bool> useStringUserId = std::nullopt,
        std::optional<int> presenceTimeout = std::nullopt,
        std::optional<bool> logUpload = std::nullopt,
        std::optional<std::string> logLevel = std::nullopt,
        std::optional<bool> cloudProxy = std::nullopt
    ) : serverUrl(serverUrl),
        token(token),
        encryptionMode(encryptionMode),
        cipherKey(cipherKey),
        salt(salt),
        useStringUserId(useStringUserId),
        presenceTimeout(presenceTimeout),
        logUpload(logUpload),
        logLevel(logLevel),
        cloudProxy(cloudProxy) {}
};

struct LoginResponse {
    int64_t timeToken; // Operation success timestamp

    LoginResponse(int64_t timeToken)
        : timeToken(timeToken) {}
};

struct LogoutResponse {
    int64_t timeToken; // Operation success timestamp

    LogoutResponse(int64_t timeToken)
        : timeToken(timeToken) {}
};

struct ErrorInfo {
    bool error;
    std::string operation;
    int errorCode;
    std::string reason;

    ErrorInfo(bool error, const std::string& operation, int errorCode, const std::string& reason)
        : error(error), operation(operation), errorCode(errorCode), reason(reason) {}
};

struct SubscribeResponse {
    int64_t timeToken; // Operation success timestamp
    std::string channelName; // Channel name for this operation

    SubscribeResponse(int64_t timeToken, const std::string& channelName)
        : timeToken(timeToken), channelName(channelName) {}
};

struct UnsubscribeResponse {
    int64_t timeToken; // Operation success timestamp
    std::string channelName; // Channel name for this operation

    UnsubscribeResponse(int64_t timeToken, const std::string& channelName)
        : timeToken(timeToken), channelName(channelName) {}
};

struct PublishResponse {
    int64_t timeToken; // (Reserved property) Operation success timestamp
    std::string channelName; // Channel name

    PublishResponse(int64_t timeToken, const std::string& channelName)
        : timeToken(timeToken), channelName(channelName) {}
};

struct QRTMEventPresence {
    std::string action;
    std::string channelType = "MESSAGE";
    std::string channelName;
    std::string publisher;

    QRTMEventPresence(const std::string& action, const std::string& channelName, const std::string& publisher)
        : action(action), channelName(channelName), publisher(publisher) {}
};

struct QRTMEvent {
    std::string eventType; // can be "presence" or "message" now
    std::string channelType = "MESSAGE";
    std::string channelName;
    std::string publisher;
    std::optional<std::string> action;
    std::optional<std::string> message;

    QRTMEvent(const std::string& eventType, const std::string& channelName, const std::string& publisher,
              const std::optional<std::string>& action = std::nullopt, 
              const std::optional<std::string>& message = std::nullopt)
        : eventType(eventType), channelName(channelName), publisher(publisher), action(action), message(message) {}
};

enum class QRTMChannelType {
    Message,
    Stream
};

enum class QRTMPresenceEventType {
    RemoteJoinChannel,
    RemoteLeaveChannel,
    RemoteConnectionTimeout,
    ErrorOutOfService
};

struct QRtmPresenceEvent {
    QRTMPresenceEventType type;
    QRTMChannelType channelType = QRTMChannelType::Message;
    std::string channelName;
    std::string publisher;

    QRtmPresenceEvent(QRTMPresenceEventType type, const std::string& channelName, const std::string& publisher)
        : type(type), channelName(channelName), publisher(publisher) {}
};

enum QRTMError {
    loginError,
    logoutError,
    connectFailed,
    subScribeFailed,
    publishFailed
};

//以上为类型定义
//以下为接口定义
class RTM;

class RTMListener {
public:
    virtual void onReceiveMessage(RTM*, const std::string&, const std::string&, const std::string&) = 0;
    virtual void onReceivePresence(RTM*, const std::string&, const QRtmPresenceEvent&) = 0;
    virtual void onConnected(RTM*) = 0;
    virtual void onDisconnected(RTM*) = 0;
    virtual void onReconnecting(RTM*) = 0;
    virtual void onReconnected(RTM*) = 0;
    virtual void onClosed(RTM*) = 0;
};

class RTM {
    public:
        RTM(std::string appId, std::string userId, const RTMConfig& config, RTMListener* listener);
        ~RTM();
        // 为了防止潜在的问题，如资源泄漏或竞争条件,禁止类的复制和赋值
        RTM(const RTM&) = delete;
        RTM& operator=(const RTM&) = delete;
    public:
        void InvokeTask(std::function<void()> task);
        void connect(std::string serverUrl);
        std::future<LoginResponse> login();
        std::future<LogoutResponse> logout();
        std::future<SubscribeResponse> subscribe(const std::string& channelName);
        std::future<UnsubscribeResponse> unsubscribe(const std::string& channelName);
        void publish(const std::string& channelName, const std::string& message);
private:
    std::future<LoginResponse> loginInternal();
    std::future<LogoutResponse> logoutInternal();
    std::future<SubscribeResponse> subscribeInternal(const std::string& channelName);
    std::future<UnsubscribeResponse> unsubscribeInternal(const std::string& channelName);
    void publishInternal(const std::string& channelName, const std::string& message);
    void WorkerFunction();
    bool IsRunningOnWorkerThread() const;
    private:
    std::unique_ptr<protoo::Peer> m_pPeer;
        std::string m_appId;
        std::string m_userId;
        RTMConfig m_config;
        RTMListener* m_pListener;
        std::string m_channelName{""}; //保存之前订阅的channel,便于重连后自动订阅
private:
    std::thread m_WorkerThread;
    std::thread::id m_WorkerThreadId{std::thread::id()};
    std::atomic<bool> m_bRunning{false};
    // 线程安全的任务队列
    std::queue<std::function<void()>> tasks;
    std::mutex tasks_mutex;
};

#endif // QRTM_H