#include "../../../include/modules/http_server_module.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include <ctime>

namespace swarm {

HttpServerModule::HttpServerModule() 
    : serverSocket_(-1), port_(5000), host_("0.0.0.0"), shouldStop_(false),
      requestCount_(0), activeConnections_(0), maxConnections_(100),
      requestTimeout_(30), enableCors_(true) {
}

HttpServerModule::~HttpServerModule() {
    shutdown();
}

bool HttpServerModule::initialize() {
    // Add default routes
    addRoute("GET", "/", [this](const HttpRequest& req) {
        return HttpResponse{200, "OK", {{"Content-Type", "application/json"}}, 
            "{\"message\": \"Hello from SwarmApp HTTP Server!\", \"module\": \"http-server\"}"};
    });
    
    addRoute("GET", "/health", [this](const HttpRequest& req) {
        return HttpResponse{200, "OK", {{"Content-Type", "application/json"}}, 
            "{\"status\": \"healthy\", \"module\": \"http-server\"}"};
    });
    
    addRoute("GET", "/status", [this](const HttpRequest& req) {
        std::ostringstream json;
        json << "{\"module\": \"http-server\", \"running\": " << (running_ ? "true" : "false")
             << ", \"requests\": " << requestCount_.load()
             << ", \"connections\": " << activeConnections_.load()
             << ", \"port\": " << port_ << "}";
        return HttpResponse{200, "OK", {{"Content-Type", "application/json"}}, json.str()};
    });
    
    return true;
}

void HttpServerModule::start() {
    if (running_) return;
    
    // Create socket
    serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket_ < 0) {
        throw std::runtime_error("Failed to create socket");
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        close(serverSocket_);
        throw std::runtime_error("Failed to set socket options");
    }
    
    // Bind socket
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);
    
    if (bind(serverSocket_, (struct sockaddr *)&address, sizeof(address)) < 0) {
        close(serverSocket_);
        throw std::runtime_error("Failed to bind socket");
    }
    
    // Listen for connections
    if (listen(serverSocket_, 3) < 0) {
        close(serverSocket_);
        throw std::runtime_error("Failed to listen on socket");
    }
    
    running_ = true;
    shouldStop_ = false;
    serverThread_ = std::thread(&HttpServerModule::serverLoop, this);
    
    std::cout << "HTTP Server started on port " << port_ << std::endl;
}

void HttpServerModule::stop() {
    if (!running_) return;
    
    shouldStop_ = true;
    running_ = false;
    
    // Close socket to wake up accept
    if (serverSocket_ >= 0) {
        close(serverSocket_);
        serverSocket_ = -1;
    }
    
    if (serverThread_.joinable()) {
        serverThread_.join();
    }
    
    std::cout << "HTTP Server stopped" << std::endl;
}

void HttpServerModule::shutdown() {
    stop();
}

std::string HttpServerModule::getStatus() const {
    std::ostringstream status;
    status << "HTTP Server (port: " << port_ << ", running: " << (running_ ? "yes" : "no")
           << ", requests: " << requestCount_.load() << ", connections: " << activeConnections_.load() << ")";
    return status.str();
}

bool HttpServerModule::configure(const std::map<std::string, std::string>& config) {
    auto it = config.find("port");
    if (it != config.end()) {
        port_ = std::stoi(it->second);
    }
    
    it = config.find("host");
    if (it != config.end()) {
        host_ = it->second;
    }
    
    it = config.find("max_connections");
    if (it != config.end()) {
        maxConnections_ = std::stoi(it->second);
    }
    
    it = config.find("request_timeout");
    if (it != config.end()) {
        requestTimeout_ = std::stoi(it->second);
    }
    
    it = config.find("enable_cors");
    if (it != config.end()) {
        enableCors_ = (it->second == "true" || it->second == "1");
    }
    
    return true;
}

void HttpServerModule::onMessage(const std::string& topic, const std::string& message) {
    // Handle messages from other modules
    if (topic == "http.request") {
        // Could handle dynamic route updates here
    }
}

void HttpServerModule::addRoute(const std::string& method, const std::string& path, HttpHandler handler) {
    std::lock_guard<std::mutex> lock(routesMutex_);
    routes_[method][path] = handler;
}

void HttpServerModule::removeRoute(const std::string& method, const std::string& path) {
    std::lock_guard<std::mutex> lock(routesMutex_);
    auto methodIt = routes_.find(method);
    if (methodIt != routes_.end()) {
        methodIt->second.erase(path);
    }
}

size_t HttpServerModule::getRequestCount() const {
    return requestCount_.load();
}

size_t HttpServerModule::getActiveConnections() const {
    return activeConnections_.load();
}

bool HttpServerModule::isHealthy() const {
    return running_ && serverSocket_ >= 0;
}

void HttpServerModule::serverLoop() {
    while (!shouldStop_) {
        struct sockaddr_in address;
        int addrlen = sizeof(address);
        
        int newSocket = accept(serverSocket_, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (newSocket < 0) {
            if (!shouldStop_) {
                std::cerr << "Accept failed" << std::endl;
            }
            continue;
        }
        
        activeConnections_++;
        
        // Handle connection in a separate thread or process
        std::thread([this, newSocket]() {
            handleConnection(newSocket);
            activeConnections_--;
        }).detach();
    }
}

void HttpServerModule::handleConnection(int clientSocket) {
    char buffer[4096] = {0};
    int valread = read(clientSocket, buffer, 4095);
    
    if (valread > 0) {
        std::string rawRequest(buffer);
        HttpRequest request = parseHttpRequest(rawRequest);
        HttpResponse response = processRequest(request);
        std::string httpResponse = createHttpResponse(response);
        
        send(clientSocket, httpResponse.c_str(), httpResponse.length(), 0);
        requestCount_++;
    }
    
    close(clientSocket);
}

HttpResponse HttpServerModule::processRequest(const HttpRequest& request) {
    std::lock_guard<std::mutex> lock(routesMutex_);
    
    auto methodIt = routes_.find(request.method);
    if (methodIt == routes_.end()) {
        return HttpResponse{405, "Method Not Allowed", {{"Content-Type", "application/json"}}, 
            "{\"error\": \"Method not allowed\"}"};
    }
    
    auto pathIt = methodIt->second.find(request.path);
    if (pathIt == methodIt->second.end()) {
        return HttpResponse{404, "Not Found", {{"Content-Type", "application/json"}}, 
            "{\"error\": \"Not found\"}"};
    }
    
    try {
        return pathIt->second(request);
    } catch (const std::exception& e) {
        return HttpResponse{500, "Internal Server Error", {{"Content-Type", "application/json"}}, 
            "{\"error\": \"Internal server error\"}"};
    }
}

std::string HttpServerModule::createHttpResponse(const HttpResponse& response) {
    std::ostringstream httpResponse;
    httpResponse << "HTTP/1.1 " << response.statusCode << " " << response.statusText << "\r\n";
    
    // Add headers
    for (const auto& [key, value] : response.headers) {
        httpResponse << key << ": " << value << "\r\n";
    }
    
    // Add CORS headers if enabled
    if (enableCors_) {
        httpResponse << "Access-Control-Allow-Origin: *\r\n";
    }
    
    httpResponse << "Content-Length: " << response.body.length() << "\r\n";
    httpResponse << "Connection: close\r\n";
    httpResponse << "\r\n";
    httpResponse << response.body;
    
    return httpResponse.str();
}

HttpRequest HttpServerModule::parseHttpRequest(const std::string& rawRequest) {
    HttpRequest request;
    std::istringstream stream(rawRequest);
    std::string line;
    
    // Parse first line (method, path, version)
    if (std::getline(stream, line)) {
        std::istringstream lineStream(line);
        lineStream >> request.method >> request.path >> request.version;
    }
    
    // Parse headers
    while (std::getline(stream, line) && line != "\r" && !line.empty()) {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            // Trim whitespace
            value.erase(0, value.find_first_not_of(" \t\r\n"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);
            request.headers[key] = value;
        }
    }
    
    // Parse body (simplified)
    std::string body;
    while (std::getline(stream, line)) {
        body += line + "\n";
    }
    request.body = body;
    
    return request;
}

} // namespace swarm
