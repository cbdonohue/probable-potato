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

namespace swarm {

// Message bus for inter-module communication
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
    
    std::map<std::string, std::vector<MessageHandler>> subscribers_;
    std::vector<Message> messageQueue_;
    mutable std::mutex subscribersMutex_;
    std::mutex queueMutex_;
    std::condition_variable queueCondition_;
    std::thread workerThread_;
    bool running_;
    size_t messageCount_;
};

} // namespace swarm

#endif // MESSAGE_BUS_H
