/**
 * @file message_bus.h
 * @brief Message bus implementation for inter-module communication using ZeroMQ
 * @author SwarmApp Development Team
 * @version 1.0.0
 */

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

/**
 * @brief Message bus for inter-module communication using ZeroMQ
 * 
 * The MessageBus class provides a thread-safe, asynchronous messaging system
 * for communication between modules in the SwarmApp framework. It uses ZeroMQ
 * for high-performance message passing and supports both synchronous and
 * asynchronous message publishing.
 * 
 * Features:
 * - Topic-based message routing
 * - Thread-safe operations
 * - Asynchronous message processing
 * - ZeroMQ integration for scalability
 * - Message statistics and monitoring
 * 
 * @note This class is thread-safe and can be used from multiple threads
 * @see Module
 * @see ModuleManager
 */
class MessageBus {
public:
    /**
     * @brief Type definition for message handler functions
     * 
     * Message handlers receive the topic and message payload as parameters
     */
    using MessageHandler = std::function<void(const std::string&, const std::string&)>;
    
    /**
     * @brief Constructor
     * 
     * Initializes the message bus with ZeroMQ context and sockets
     */
    MessageBus();
    
    /**
     * @brief Destructor
     * 
     * Cleans up ZeroMQ resources and stops the message processing thread
     */
    ~MessageBus();
    
    /**
     * @name Message Handling Methods
     * @{
     */
    
    /**
     * @brief Subscribe to a topic
     * 
     * Registers a message handler for a specific topic. When messages are
     * published to this topic, the handler will be called.
     * 
     * @param topic The topic to subscribe to
     * @param handler The function to call when messages are received
     * @note Multiple handlers can be registered for the same topic
     */
    void subscribe(const std::string& topic, MessageHandler handler);
    
    /**
     * @brief Unsubscribe from a topic
     * 
     * Removes a message handler from a specific topic.
     * 
     * @param topic The topic to unsubscribe from
     * @param handler The handler to remove
     * @note Only the exact handler instance will be removed
     */
    void unsubscribe(const std::string& topic, MessageHandler handler);
    
    /**
     * @brief Publish a message synchronously
     * 
     * Publishes a message to a topic and waits for all handlers to process it.
     * 
     * @param topic The topic to publish to
     * @param message The message payload
     */
    void publish(const std::string& topic, const std::string& message);
    
    /**
     * @brief Publish a message asynchronously
     * 
     * Queues a message for asynchronous processing by the message bus thread.
     * 
     * @param topic The topic to publish to
     * @param message The message payload
     */
    void publishAsync(const std::string& topic, const std::string& message);
    
    /** @} */
    
    /**
     * @name Bus Management Methods
     * @{
     */
    
    /**
     * @brief Start the message bus
     * 
     * Initializes ZeroMQ sockets and starts the message processing thread.
     */
    void start();
    
    /**
     * @brief Stop the message bus
     * 
     * Stops the message processing thread and cleans up resources.
     */
    void stop();
    
    /**
     * @brief Check if the message bus is running
     * 
     * @return true if the message bus is running, false otherwise
     */
    bool isRunning() const;
    
    /** @} */
    
    /**
     * @name Statistics Methods
     * @{
     */
    
    /**
     * @brief Get the total number of messages processed
     * 
     * @return The total message count since the bus was started
     */
    size_t getMessageCount() const;
    
    /**
     * @brief Get the number of subscribers for a topic
     * 
     * @param topic The topic to check
     * @return The number of registered handlers for the topic
     */
    size_t getSubscriberCount(const std::string& topic) const;
    
    /** @} */

private:
    /**
     * @brief Internal message structure
     * 
     * Contains all information about a message including topic, payload, and timestamp
     */
    struct Message {
        std::string topic;                                    ///< The message topic
        std::string payload;                                  ///< The message payload
        std::chrono::system_clock::time_point timestamp;     ///< Message timestamp
    };
    
    /**
     * @brief Process messages from the queue
     * 
     * This method runs in a separate thread and processes queued messages
     */
    void processMessages();
    
    /**
     * @brief Initialize ZeroMQ context and sockets
     * 
     * Sets up the ZeroMQ publisher and subscriber sockets
     */
    void setupZeroMQ();
    
    /**
     * @brief Clean up ZeroMQ resources
     * 
     * Closes sockets and cleans up the ZeroMQ context
     */
    void cleanupZeroMQ();
    
    // ZeroMQ components
    std::unique_ptr<zmq::context_t> context_;        ///< ZeroMQ context
    std::unique_ptr<zmq::socket_t> publisher_socket_; ///< Publisher socket for sending messages
    std::unique_ptr<zmq::socket_t> subscriber_socket_; ///< Subscriber socket for receiving messages
    
    // Internal message handling
    std::map<std::string, std::vector<MessageHandler>> subscribers_; ///< Topic to handlers mapping
    std::vector<Message> messageQueue_;                              ///< Queue for async messages
    mutable std::mutex subscribersMutex_;                            ///< Mutex for subscribers map
    std::mutex queueMutex_;                                          ///< Mutex for message queue
    std::condition_variable queueCondition_;                         ///< Condition variable for queue
    std::thread workerThread_;                                       ///< Thread for processing messages
    std::atomic<bool> running_;                                      ///< Flag indicating if bus is running
    std::atomic<size_t> messageCount_;                               ///< Total message count
    
    // ZeroMQ configuration
    static constexpr const char* PUBLISHER_ENDPOINT = "tcp://127.0.0.1:5555";  ///< Publisher endpoint
    static constexpr const char* SUBSCRIBER_ENDPOINT = "tcp://127.0.0.1:5556"; ///< Subscriber endpoint
    static constexpr int MAX_PORT_RETRIES = 5;                                 ///< Max port retry attempts
    static constexpr int PORT_INCREMENT = 10;                                  ///< Port increment for retries
};

} // namespace swarm

#endif // MESSAGE_BUS_H
