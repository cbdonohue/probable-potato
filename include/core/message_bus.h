#ifndef MESSAGE_BUS_H
#define MESSAGE_BUS_H

#include <string>
#include <functional>
#include <map>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>

// ZeroMQ includes
#include <zmq.hpp>

namespace swarm {

// Message bus for inter-module communication using ZeroMQ
class MessageBus {
public:
    using MessageHandler = std::function<void(const std::string&, const std::string&)>;
    
    MessageBus();
    ~MessageBus();
    
    // Message handling
    void subscribe(const std::string& topic, MessageHandler handler);
    void unsubscribe(const std::string& topic, MessageHandler handler);
    void publish(const std::string& topic, const std::string& message);
    
    // Async message handling
    void publishAsync(const std::string& topic, const std::string& message);
    
    // Bus management
    void start();
    void stop();
    bool isRunning() const;
    
    // Statistics
    size_t getMessageCount() const;
    size_t getSubscriberCount(const std::string& topic) const;

private:
    struct Message {
        std::string topic;
        std::string payload;
        std::chrono::system_clock::time_point timestamp;
    };
    
    void processMessages();
    void setupZeroMQ();
    void cleanupZeroMQ();
    
    // ZeroMQ components
    std::unique_ptr<zmq::context_t> context_;
    std::unique_ptr<zmq::socket_t> publisher_socket_;
    std::unique_ptr<zmq::socket_t> subscriber_socket_;
    
    // Internal message handling
    std::map<std::string, std::vector<MessageHandler>> subscribers_;
    std::vector<Message> messageQueue_;
    mutable std::mutex subscribersMutex_;
    std::mutex queueMutex_;
    std::condition_variable queueCondition_;
    std::thread workerThread_;
    std::atomic<bool> running_;
    std::atomic<size_t> messageCount_;
    
    // ZeroMQ configuration
    static constexpr const char* PUBLISHER_ENDPOINT = "tcp://127.0.0.1:5555";
    static constexpr const char* SUBSCRIBER_ENDPOINT = "tcp://127.0.0.1:5556";
    static constexpr int MAX_PORT_RETRIES = 5;
    static constexpr int PORT_INCREMENT = 10;
};

} // namespace swarm

#endif // MESSAGE_BUS_H
