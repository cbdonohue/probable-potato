# Message Bus Architecture Diagram (ZeroMQ Integration)

## Overview
The Message Bus in SwarmApp now uses ZeroMQ for high-performance, distributed messaging while maintaining the same publish/subscribe API. This provides better scalability, reliability, and performance compared to the previous custom implementation.

## Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              MESSAGE BUS (ZeroMQ)                           │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐         │
│  │   PUBLISHERS    │    │   SUBSCRIBERS   │    │   ZERO MQ CORE   │         │
│  │                 │    │                 │    │                 │         │
│  │ ┌─────────────┐ │    │ ┌─────────────┐ │    │ ┌─────────────┐ │         │
│  │ │HTTP Server  │ │    │ │Health       │ │    │ │Publisher    │ │         │
│  │ │Module       │ │    │ │Monitor      │ │    │ │Socket       │ │         │
│  │ │             │ │    │ │Module       │ │    │ │(ZMQ_PUB)    │ │         │
│  │ │publish()    │ │    │ │             │ │    │ │tcp://5555   │ │         │
│  │ │publishAsync()│ │    │ │subscribe()  │ │    │ │             │ │         │
│  │ └─────────────┘ │    │ │onMessage()  │ │    │ └─────────────┘ │         │
│  └─────────────────┘    │ └─────────────┘ │    │                 │         │
│                         └─────────────────┘    │ ┌─────────────┐ │         │
│                                                 │ │Subscriber  │ │         │
│  ┌─────────────────┐    ┌─────────────────┐    │ │Socket      │ │         │
│  │   OTHER MODULES │    │   OTHER MODULES │    │ │(ZMQ_SUB)   │ │         │
│  │                 │    │                 │    │ │tcp://5556  │ │         │
│  │ ┌─────────────┐ │    │ ┌─────────────┐ │    │ │             │ │         │
│  │ │Module A     │ │    │ │Module B     │ │    │ └─────────────┘ │         │
│  │ │             │ │    │ │             │ │    │                 │         │
│  │ │publish()    │ │    │ │subscribe()  │ │    │ ┌─────────────┐ │         │
│  │ └─────────────┘ │    │ │onMessage()  │ │    │ │Local        │ │         │
│  └─────────────────┘    │ └─────────────┘ │    │ │Subscribers  │ │         │
│                         └─────────────────┘    │ │Registry     │ │         │
│                                                 │ │             │ │         │
│                                                 │ │topic1 →     │ │         │
│                                                 │ │[handler1,   │ │         │
│                                                 │ │ handler2]   │ │         │
│                                                 │ └─────────────┘ │         │
│                                                 └─────────────────┘         │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Data Flow Diagram

```
┌─────────────┐    1. subscribe()    ┌─────────────┐
│   Module    │ ────────────────────→ │ Message Bus │
│ (Subscriber)│                      │             │
└─────────────┘                      │             │
                                     │             │
┌─────────────┐    2. publish()      │             │
│   Module    │ ────────────────────→ │             │
│ (Publisher) │                      │             │
└─────────────┘                      │             │
                                     │             │    3. ZeroMQ
                                     │             │ ──────────────────┐
                                     │             │                   │
                                     │             │                   ▼
                                     │             │            ┌─────────────┐
                                     │             │            │ ZeroMQ      │
                                     │             │            │ Publisher   │
                                     │             │            │ Socket      │
                                     │             │            └─────────────┘
                                     │             │                   │
                                     │             │                   ▼
                                     │             │            ┌─────────────┐
                                     │             │            │ ZeroMQ      │
                                     │             │            │ Subscriber  │
                                     │             │            │ Socket      │
                                     │             │            └─────────────┘
                                     │             │                   │
                                     │             │                   ▼
                                     │             │    4. Worker Thread│
                                     │             │ ←─────────────────┘
                                     │             │
                                     │             │    5. handler()   │
                                     │             │ ←─────────────────┘
                                     │             │
                                     │             │    6. onMessage() │
                                     │             │ ←─────────────────┘
                                     └─────────────┘
```

## Key Components

### 1. ZeroMQ Core Components
- **Context**: `std::unique_ptr<zmq::context_t> context_` - ZeroMQ context for socket management
- **Publisher Socket**: `std::unique_ptr<zmq::socket_t> publisher_socket_` - ZMQ_PUB socket for sending messages
- **Subscriber Socket**: `std::unique_ptr<zmq::socket_t> subscriber_socket_` - ZMQ_SUB socket for receiving messages
- **Endpoints**: TCP endpoints for inter-process communication (tcp://127.0.0.1:5555/5556)

### 2. Local Message Handling (Backward Compatibility)
- **Subscribers Registry**: `std::map<std::string, std::vector<MessageHandler>> subscribers_`
- **Message Queue**: `std::vector<Message> messageQueue_` (for async messages)
- **Threading**: Worker thread for processing ZeroMQ messages and async queue

### 3. Message Structure
```cpp
struct Message {
    std::string topic;                                    // Message topic/category
    std::string payload;                                  // Message content
    std::chrono::system_clock::time_point timestamp;     // Message timestamp
};
```

### 4. ZeroMQ Message Format
```
"topic message_payload"
```
- Topic and payload are space-separated
- Sent as single ZeroMQ message frame

## Message Flow Types

### Synchronous Publishing (`publish()`)
```
Publisher → ZeroMQ Publisher Socket → ZeroMQ Subscriber Socket → Worker Thread → Handler Execution → Subscriber
```

**Steps:**
1. Publisher calls `publish(topic, message)`
2. Message Bus sends via ZeroMQ publisher socket
3. ZeroMQ subscriber socket receives message
4. Worker thread processes received message
5. Handlers execute in worker thread context
6. Increments message counter

### Asynchronous Publishing (`publishAsync()`)
```
Publisher → Local Message Queue → Worker Thread → ZeroMQ Publisher → ZeroMQ Subscriber → Handler Execution → Subscriber
```

**Steps:**
1. Publisher calls `publishAsync(topic, message)`
2. Message Bus adds message to local queue
3. Worker thread processes queued messages
4. Each message is published via ZeroMQ
5. ZeroMQ subscriber receives and processes
6. Handlers execute in worker thread context

## Threading Model

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Main Thread   │    │  Publisher      │    │  Worker Thread  │
│                 │    │  Threads        │    │                 │
│                 │    │                 │    │                 │
│ subscribe()     │    │ publishAsync()  │    │ processMessages()│
│ publish()       │    │                 │    │ ZeroMQ Polling  │
│ start()         │    │                 │    │ Message Routing │
│ stop()          │    │                 │    │ Handler Exec    │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         │                       │                       │
         ▼                       ▼                       ▼
┌─────────────────────────────────────────────────────────────────┐
│                    SHARED RESOURCES                             │
│                                                                 │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐            │
│  │ subscribers_│  │ messageQueue_│  │ ZeroMQ      │            │
│  │ (read/write)│  │ (write)     │  │ Sockets     │            │
│  │             │  │             │  │ Context     │            │
│  └─────────────┘  └─────────────┘  └─────────────┘            │
└─────────────────────────────────────────────────────────────────┘
```

## ZeroMQ Configuration

### Socket Endpoints
- **Publisher**: `tcp://127.0.0.1:5555` - For sending messages
- **Subscriber**: `tcp://127.0.0.1:5556` - For receiving messages

### Socket Types
- **ZMQ_PUB**: Publisher socket for broadcasting messages
- **ZMQ_SUB**: Subscriber socket for receiving messages with topic filtering

### Polling Configuration
- **Poll Timeout**: 100ms for responsive message processing
- **Poll Events**: ZMQ_POLLIN for incoming messages

## Error Handling

```
┌─────────────┐
│ ZeroMQ      │
│ Operation   │
└─────────────┘
       │
       ▼
┌─────────────┐
│ Try-Catch   │
│ Block       │
└─────────────┘
       │
       ▼
┌─────────────┐
│ Log Error   │
│ Continue    │
└─────────────┘
```

- ZeroMQ operations wrapped in try-catch blocks
- Errors logged to stderr
- Message processing continues despite individual errors
- Graceful degradation on ZeroMQ failures

## Performance Benefits

### Compared to Custom Implementation
- **Higher Throughput**: ZeroMQ optimized for high-performance messaging
- **Lower Latency**: Efficient message routing and delivery
- **Better Scalability**: Supports distributed messaging across processes
- **Memory Efficiency**: Zero-copy message passing where possible
- **Network Transparency**: Can easily extend to network-based messaging

### ZeroMQ Features Utilized
- **Asynchronous I/O**: Non-blocking socket operations
- **Message Batching**: Efficient message grouping
- **Topic Filtering**: Built-in subscriber topic matching
- **Connection Management**: Automatic reconnection and recovery

## Usage Examples

### Subscribing to Messages (Unchanged API)
```cpp
// Module subscribes to health check messages
messageBus->subscribe("health.check", [this](const std::string& topic, const std::string& message) {
    this->onMessage(topic, message);
});
```

### Publishing Messages (Unchanged API)
```cpp
// Synchronous publishing (now via ZeroMQ)
messageBus->publish("health.status_change", "{\"module\": \"http-server\", \"healthy\": true}");

// Asynchronous publishing (queued then ZeroMQ)
messageBus->publishAsync("health.check", "manual_check");
```

### Module Integration (Unchanged)
```cpp
// Module receives message bus reference from ModuleManager
void Module::setMessageBus(MessageBus* bus) { 
    messageBus_ = bus; 
}

// Module implements message handling
void HealthMonitorModule::onMessage(const std::string& topic, const std::string& message) {
    if (topic == "health.check") {
        performHealthCheck(message);
    }
}
```

## Installation and Setup

### Dependencies
```bash
# Ubuntu/Debian
sudo apt-get install libzmq3-dev pkg-config

# CentOS/RHEL/Fedora
sudo yum install zeromq-devel pkgconfig

# Arch Linux
sudo pacman -S zeromq pkg-config

# Or use the provided script
./scripts/install_zeromq.sh
```

### Building with ZeroMQ
```bash
mkdir -p build && cd build
cmake ..
make
```

### Verification
```bash
# Check ZeroMQ version
pkg-config --modversion libzmq

# Build and test
make && ./swarm-app
```

## Migration Benefits

### Backward Compatibility
- **Same API**: All existing code continues to work unchanged
- **Same Patterns**: Publish/subscribe pattern maintained
- **Same Lifecycle**: start()/stop() methods work identically

### Enhanced Capabilities
- **Distributed Messaging**: Can communicate across processes
- **Network Messaging**: Can extend to network-based communication
- **Better Performance**: Optimized message routing and delivery
- **Reliability**: ZeroMQ's battle-tested messaging infrastructure

### Future Extensibility
- **Multi-Process**: Can easily support multiple SwarmApp instances
- **Network Clustering**: Can extend to distributed deployments
- **Message Persistence**: Can add ZeroMQ persistence plugins
- **Load Balancing**: Can implement ZeroMQ load balancing patterns

The ZeroMQ integration provides a robust, high-performance messaging foundation while maintaining complete backward compatibility with your existing codebase.
