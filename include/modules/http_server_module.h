/**
 * @file http_server_module.h
 * @brief HTTP server module for external communication
 * @author SwarmApp Development Team
 * @version 1.0.0
 */

#ifndef HTTP_SERVER_MODULE_H
#define HTTP_SERVER_MODULE_H

#include "../core/module.h"
#include <string>
#include <map>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>

namespace swarm {

/**
 * @brief HTTP request structure
 * 
 * Contains all information about an incoming HTTP request
 */
struct HttpRequest {
    std::string method;                                    ///< HTTP method (GET, POST, etc.)
    std::string path;                                      ///< Request path
    std::string version;                                   ///< HTTP version
    std::map<std::string, std::string> headers;            ///< Request headers
    std::string body;                                      ///< Request body
};

/**
 * @brief HTTP response structure
 * 
 * Contains all information about an HTTP response
 */
struct HttpResponse {
    int statusCode;                                        ///< HTTP status code
    std::string statusText;                                ///< HTTP status text
    std::map<std::string, std::string> headers;            ///< Response headers
    std::string body;                                      ///< Response body
};

/**
 * @brief HTTP handler function type
 * 
 * Functions of this type handle HTTP requests and return responses
 */
using HttpHandler = std::function<HttpResponse(const HttpRequest&)>;

/**
 * @brief HTTP Server Module
 * 
 * The HttpServerModule provides HTTP server functionality for external communication.
 * It allows other modules and external clients to interact with the SwarmApp system
 * through RESTful HTTP APIs.
 * 
 * Features:
 * - Configurable routes and handlers
 * - Thread-safe request handling
 * - Request statistics and monitoring
 * - CORS support
 * - Health check endpoints
 * 
 * @note This module runs in its own thread and is thread-safe
 * @see Module
 * @see HttpRequest
 * @see HttpResponse
 */
class HttpServerModule : public Module {
public:
    /**
     * @brief Constructor
     * 
     * Initializes the HTTP server module with default configuration
     */
    HttpServerModule();
    
    /**
     * @brief Destructor
     * 
     * Stops the server and cleans up resources
     */
    ~HttpServerModule() override;
    
    /**
     * @name Module Interface Implementation
     * @{
     */
    
    /**
     * @brief Initialize the HTTP server
     * 
     * Sets up the server socket and initializes configuration.
     * 
     * @return true if initialization was successful, false otherwise
     */
    bool initialize() override;
    
    /**
     * @brief Start the HTTP server
     * 
     * Starts the server thread and begins accepting connections.
     */
    void start() override;
    
    /**
     * @brief Stop the HTTP server
     * 
     * Stops accepting new connections and shuts down the server thread.
     */
    void stop() override;
    
    /**
     * @brief Shutdown the HTTP server
     * 
     * Performs complete cleanup of server resources.
     */
    void shutdown() override;
    
    /**
     * @brief Get the module name
     * 
     * @return The module name: "http-server"
     */
    std::string getName() const override { return "http-server"; }
    
    /**
     * @brief Get the module version
     * 
     * @return The module version: "1.0.0"
     */
    std::string getVersion() const override { return "1.0.0"; }
    
    /**
     * @brief Get module dependencies
     * 
     * @return Empty vector (no dependencies)
     */
    std::vector<std::string> getDependencies() const override { return {}; }
    
    /**
     * @brief Check if the server is running
     * 
     * @return true if the server is running, false otherwise
     */
    bool isRunning() const override { return running_; }
    
    /**
     * @brief Get the server status
     * 
     * @return A human-readable status string
     */
    std::string getStatus() const override;
    
    /**
     * @brief Configure the HTTP server
     * 
     * @param config Configuration parameters (port, host, etc.)
     * @return true if configuration was successful, false otherwise
     */
    bool configure(const std::map<std::string, std::string>& config) override;
    
    /**
     * @brief Handle incoming messages
     * 
     * Processes messages from the message bus.
     * 
     * @param topic The message topic
     * @param message The message payload
     */
    void onMessage(const std::string& topic, const std::string& message) override;
    
    /** @} */
    
    /**
     * @name HTTP Server Methods
     * @{
     */
    
    /**
     * @brief Add a route handler
     * 
     * Registers a handler function for a specific HTTP method and path.
     * 
     * @param method The HTTP method (GET, POST, PUT, DELETE, etc.)
     * @param path The URL path pattern
     * @param handler The function to handle requests for this route
     */
    void addRoute(const std::string& method, const std::string& path, HttpHandler handler);
    
    /**
     * @brief Remove a route handler
     * 
     * Removes a previously registered route handler.
     * 
     * @param method The HTTP method
     * @param path The URL path pattern
     */
    void removeRoute(const std::string& method, const std::string& path);
    
    /** @} */
    
    /**
     * @name Statistics Methods
     * @{
     */
    
    /**
     * @brief Get the total number of requests processed
     * 
     * @return The total request count since server start
     */
    size_t getRequestCount() const;
    
    /**
     * @brief Get the number of active connections
     * 
     * @return The current number of active client connections
     */
    size_t getActiveConnections() const;
    
    /** @} */
    
    /**
     * @name Health Check Methods
     * @{
     */
    
    /**
     * @brief Check if the server is healthy
     * 
     * @return true if the server is healthy and responding, false otherwise
     */
    bool isHealthy() const;
    
    /** @} */

private:
    /**
     * @brief Main server loop
     * 
     * Runs in a separate thread and accepts incoming connections
     */
    void serverLoop();
    
    /**
     * @brief Handle a client connection
     * 
     * @param clientSocket The client socket file descriptor
     */
    void handleConnection(int clientSocket);
    
    /**
     * @brief Process an HTTP request
     * 
     * @param request The parsed HTTP request
     * @return The HTTP response to send
     */
    HttpResponse processRequest(const HttpRequest& request);
    
    /**
     * @brief Create an HTTP response string
     * 
     * @param response The response object
     * @return The formatted HTTP response string
     */
    std::string createHttpResponse(const HttpResponse& response);
    
    /**
     * @brief Parse a raw HTTP request
     * 
     * @param rawRequest The raw HTTP request string
     * @return The parsed HTTP request object
     */
    HttpRequest parseHttpRequest(const std::string& rawRequest);
    
    int serverSocket_;                                     ///< Server socket file descriptor
    int port_;                                            ///< Server port number
    std::string host_;                                    ///< Server host address
    std::thread serverThread_;                            ///< Server thread
    std::atomic<bool> shouldStop_;                        ///< Flag to stop the server
    std::atomic<size_t> requestCount_;                    ///< Total request count
    std::atomic<size_t> activeConnections_;               ///< Active connection count
    
    std::map<std::string, std::map<std::string, HttpHandler>> routes_; ///< Route handlers
    std::mutex routesMutex_;                              ///< Mutex for route access
    
    // Configuration
    int maxConnections_;                                  ///< Maximum concurrent connections
    int requestTimeout_;                                  ///< Request timeout in seconds
    bool enableCors_;                                     ///< Enable CORS support
};

} // namespace swarm

#endif // HTTP_SERVER_MODULE_H
