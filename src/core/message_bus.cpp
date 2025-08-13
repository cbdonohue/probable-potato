#include "../../include/core/message_bus.h"
#include <iostream>
#include <algorithm>

namespace swarm {

MessageBus::MessageBus() : running_(false), messageCount_(0) {
}

MessageBus::~MessageBus() {
    stop();
}

void MessageBus::subscribe(const std::string& topic, MessageHandler handler) {
    std::lock_guard<std::mutex> lock(subscribersMutex_);
    subscribers_[topic].push_back(handler);
}

void MessageBus::unsubscribe(const std::string& topic, MessageHandler handler) {
    std::lock_guard<std::mutex> lock(subscribersMutex_);
    auto it = subscribers_.find(topic);
    if (it != subscribers_.end()) {
        auto& handlers = it->second;
        // Note: This is a simplified implementation
        // In a real implementation, you'd need a way to identify specific handlers
        handlers.clear(); // Remove all handlers for this topic
    }
}

void MessageBus::publish(const std::string& topic, const std::string& message) {
    std::lock_guard<std::mutex> lock(subscribersMutex_);
    auto it = subscribers_.find(topic);
    if (it != subscribers_.end()) {
        for (const auto& handler : it->second) {
            try {
                handler(topic, message);
            } catch (const std::exception& e) {
                std::cerr << "Error in message handler: " << e.what() << std::endl;
            }
        }
    }
    messageCount_++;
}

void MessageBus::publishAsync(const std::string& topic, const std::string& message) {
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        messageQueue_.push_back({topic, message, std::chrono::system_clock::now()});
    }
    queueCondition_.notify_one();
}

void MessageBus::start() {
    if (!running_) {
        running_ = true;
        workerThread_ = std::thread(&MessageBus::processMessages, this);
    }
}

void MessageBus::stop() {
    if (running_) {
        running_ = false;
        queueCondition_.notify_all();
        if (workerThread_.joinable()) {
            workerThread_.join();
        }
    }
}

bool MessageBus::isRunning() const {
    return running_;
}

size_t MessageBus::getMessageCount() const {
    return messageCount_;
}

size_t MessageBus::getSubscriberCount(const std::string& topic) const {
    std::lock_guard<std::mutex> lock(subscribersMutex_);
    auto it = subscribers_.find(topic);
    return (it != subscribers_.end()) ? it->second.size() : 0;
}

void MessageBus::processMessages() {
    while (running_) {
        std::vector<Message> messages;
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCondition_.wait(lock, [this] { return !messageQueue_.empty() || !running_; });
            
            if (!running_) break;
            
            messages.swap(messageQueue_);
        }
        
        for (const auto& msg : messages) {
            publish(msg.topic, msg.payload);
        }
    }
}

} // namespace swarm
