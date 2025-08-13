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

// HTTP request structure
struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
};

// HTTP response structure
struct HttpResponse {
    int statusCode;
    std::string statusText;
    std::map<std::string, std::string> headers;
    std::string body;
};

// HTTP handler function type
using HttpHandler = std::function<HttpResponse(const HttpRequest&)>;

// HTTP Server Module
class HttpServerModule : public Module {
public:
    HttpServerModule();
    ~HttpServerModule() override;
    
    // Module interface implementation
    bool initialize() override;
    void start() override;
    void stop() override;
    void shutdown() override;
    
    std::string getName() const override { return "http-server"; }
    std::string getVersion() const override { return "1.0.0"; }
    std::vector<std::string> getDependencies() const override { return {}; }
    
    bool isRunning() const override { return running_; }
    std::string getStatus() const override;
    
    bool configure(const std::map<std::string, std::string>& config) override;
    void onMessage(const std::string& topic, const std::string& message) override;
    
    // HTTP server specific methods
    void addRoute(const std::string& method, const std::string& path, HttpHandler handler);
    void removeRoute(const std::string& method, const std::string& path);
    
    // Server statistics
    size_t getRequestCount() const;
    size_t getActiveConnections() const;
    
    // Health check
    bool isHealthy() const;

private:
    void serverLoop();
    void handleConnection(int clientSocket);
    HttpResponse processRequest(const HttpRequest& request);
    std::string createHttpResponse(const HttpResponse& response);
    HttpRequest parseHttpRequest(const std::string& rawRequest);
    
    int serverSocket_;
    int port_;
    std::string host_;
    std::thread serverThread_;
    std::atomic<bool> shouldStop_;
    std::atomic<size_t> requestCount_;
    std::atomic<size_t> activeConnections_;
    
    std::map<std::string, std::map<std::string, HttpHandler>> routes_;
    std::mutex routesMutex_;
    
    // Configuration
    int maxConnections_;
    int requestTimeout_;
    bool enableCors_;
};

} // namespace swarm

#endif // HTTP_SERVER_MODULE_H
