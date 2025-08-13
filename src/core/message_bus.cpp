#include "../../include/core/message_bus.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <zmq.hpp>

namespace swarm {

MessageBus::MessageBus() : running_(false), messageCount_(0) {
    setupZeroMQ();
}

MessageBus::~MessageBus() {
    stop();
    cleanupZeroMQ();
}

void MessageBus::setupZeroMQ() {
    try {
        // Create ZeroMQ context
        context_ = std::make_unique<zmq::context_t>(1);
        
        // Try to bind publisher socket with port retry logic
        publisher_socket_ = std::make_unique<zmq::socket_t>(*context_, ZMQ_PUB);
        int linger = 0;
        publisher_socket_->setsockopt(ZMQ_LINGER, &linger, sizeof(linger)); // Don't wait on close
        
        int pub_port = 5555;
        bool pub_bound = false;
        for (int i = 0; i < MAX_PORT_RETRIES && !pub_bound; i++) {
            try {
                std::string endpoint = "tcp://127.0.0.1:" + std::to_string(pub_port);
                publisher_socket_->bind(endpoint);
                pub_bound = true;
            } catch (const zmq::error_t& e) {
                if (i < MAX_PORT_RETRIES - 1) {
                    pub_port += PORT_INCREMENT;
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                } else {
                    throw;
                }
            }
        }
        
        // Try to bind subscriber socket with port retry logic
        subscriber_socket_ = std::make_unique<zmq::socket_t>(*context_, ZMQ_SUB);
        int sub_linger = 0;
        subscriber_socket_->setsockopt(ZMQ_LINGER, &sub_linger, sizeof(sub_linger)); // Don't wait on close
        
        int sub_port = 5556;
        bool sub_bound = false;
        for (int i = 0; i < MAX_PORT_RETRIES && !sub_bound; i++) {
            try {
                std::string endpoint = "tcp://127.0.0.1:" + std::to_string(sub_port);
                subscriber_socket_->bind(endpoint);
                sub_bound = true;
            } catch (const zmq::error_t& e) {
                if (i < MAX_PORT_RETRIES - 1) {
                    sub_port += PORT_INCREMENT;
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                } else {
                    throw;
                }
            }
        }
        
        // Allow time for sockets to bind
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
    } catch (const zmq::error_t& e) {
        std::cerr << "ZeroMQ setup error: " << e.what() << std::endl;
        throw;
    }
}

void MessageBus::cleanupZeroMQ() {
    try {
        if (publisher_socket_) {
            publisher_socket_->close();
            publisher_socket_.reset();
        }
        if (subscriber_socket_) {
            subscriber_socket_->close();
            subscriber_socket_.reset();
        }
        if (context_) {
            context_->close();
            context_.reset();
        }
        // Allow time for sockets to fully close
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    } catch (const zmq::error_t& e) {
        std::cerr << "ZeroMQ cleanup error: " << e.what() << std::endl;
    }
}

void MessageBus::subscribe(const std::string& topic, MessageHandler handler) {
    std::lock_guard<std::mutex> lock(subscribersMutex_);
    subscribers_[topic].push_back(handler);
    
    // Subscribe to topic in ZeroMQ
    try {
        subscriber_socket_->setsockopt(ZMQ_SUBSCRIBE, topic.c_str(), topic.length());
    } catch (const zmq::error_t& e) {
        std::cerr << "ZeroMQ subscribe error: " << e.what() << std::endl;
    }
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
    
    // Unsubscribe from topic in ZeroMQ
    try {
        subscriber_socket_->setsockopt(ZMQ_UNSUBSCRIBE, topic.c_str(), topic.length());
    } catch (const zmq::error_t& e) {
        std::cerr << "ZeroMQ unsubscribe error: " << e.what() << std::endl;
    }
}

void MessageBus::publish(const std::string& topic, const std::string& message) {
    try {
        // Send message via ZeroMQ publisher
        std::string zmq_message = topic + " " + message;
        zmq::message_t zmq_msg(zmq_message.data(), zmq_message.size());
        publisher_socket_->send(zmq_msg, zmq::send_flags::none);
        
        // Also handle locally for immediate subscribers
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
        
    } catch (const zmq::error_t& e) {
        std::cerr << "ZeroMQ publish error: " << e.what() << std::endl;
    }
}

void MessageBus::publishAsync(const std::string& topic, const std::string& message) {
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        messageQueue_.push_back({topic, message, std::chrono::system_clock::now()});
    }
    queueCondition_.notify_one();
}

void MessageBus::start() {
    if (!running_.exchange(true)) {
        workerThread_ = std::thread(&MessageBus::processMessages, this);
    }
}

void MessageBus::stop() {
    if (running_.exchange(false)) {
        queueCondition_.notify_all();
        if (workerThread_.joinable()) {
            workerThread_.join();
        }
    }
}

bool MessageBus::isRunning() const {
    return running_.load();
}

size_t MessageBus::getMessageCount() const {
    return messageCount_.load();
}

size_t MessageBus::getSubscriberCount(const std::string& topic) const {
    std::lock_guard<std::mutex> lock(subscribersMutex_);
    auto it = subscribers_.find(topic);
    return (it != subscribers_.end()) ? it->second.size() : 0;
}

void MessageBus::processMessages() {
    // Set up polling for ZeroMQ messages
    zmq::pollitem_t items[] = {
        { subscriber_socket_->handle(), 0, ZMQ_POLLIN, 0 }
    };
    
    while (running_.load()) {
        try {
            // Poll for ZeroMQ messages with timeout
            zmq::poll(items, 1, std::chrono::milliseconds(100));
            
            // Process ZeroMQ messages
            if (items[0].revents & ZMQ_POLLIN) {
                zmq::message_t zmq_msg;
                if (subscriber_socket_->recv(zmq_msg, zmq::recv_flags::none)) {
                    std::string received_msg(static_cast<char*>(zmq_msg.data()), zmq_msg.size());
                    
                    // Parse topic and message
                    size_t space_pos = received_msg.find(' ');
                    if (space_pos != std::string::npos) {
                        std::string topic = received_msg.substr(0, space_pos);
                        std::string message = received_msg.substr(space_pos + 1);
                        
                        // Handle message locally
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
                }
            }
            
            // Process queued async messages
            std::vector<Message> messages;
            {
                std::unique_lock<std::mutex> lock(queueMutex_);
                if (queueCondition_.wait_for(lock, std::chrono::milliseconds(10), 
                    [this] { return !messageQueue_.empty() || !running_.load(); })) {
                    
                    if (!running_.load()) break;
                    messages.swap(messageQueue_);
                }
            }
            
            for (const auto& msg : messages) {
                publish(msg.topic, msg.payload);
            }
            
        } catch (const zmq::error_t& e) {
            std::cerr << "ZeroMQ processing error: " << e.what() << std::endl;
            // Continue processing even if there's an error
        } catch (const std::exception& e) {
            std::cerr << "Message processing error: " << e.what() << std::endl;
        }
    }
}

} // namespace swarm
