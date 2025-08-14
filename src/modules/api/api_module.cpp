#include "modules/api_module.h"
#include <oatpp/network/Address.hpp>
#include <oatpp/web/protocol/http/outgoing/ResponseFactory.hpp>
#include <iostream>
#include <sstream>
#include <chrono>

namespace swarm {

// Implementation of SimpleHttpHandler
std::shared_ptr<oatpp::web::protocol::http::outgoing::Response> SimpleHttpHandler::handle(
    const std::shared_ptr<oatpp::web::protocol::http::incoming::Request>& request) {
    
    auto path = request->getStartingLine().path;
    auto method = request->getStartingLine().method;
    
    std::cout << "API Request: " << method.std_str() << " " << path.std_str() << std::endl;
    
    if (path == "/health" || path == "health") {
        auto response = oatpp::web::protocol::http::outgoing::ResponseFactory::createResponse(
            oatpp::web::protocol::http::Status::CODE_200, 
            "{\"status\":\"healthy\",\"timestamp\":\"2024-01-01T00:00:00Z\",\"version\":\"1.0.0\",\"hostname\":\"swarm-app\"}"
        );
        response->putHeader("Content-Type", "application/json");
        return response;
    }
    else if (path == "/status" || path == "status") {
        auto response = oatpp::web::protocol::http::outgoing::ResponseFactory::createResponse(
            oatpp::web::protocol::http::Status::CODE_200, 
            "{\"status\":\"running\",\"uptime\":\"0s\",\"requests_processed\":0,\"active_connections\":0,\"version\":\"1.0.0\"}"
        );
        response->putHeader("Content-Type", "application/json");
        return response;
    }
    else if (path == "/api/info" || path == "api/info") {
        auto response = oatpp::web::protocol::http::outgoing::ResponseFactory::createResponse(
            oatpp::web::protocol::http::Status::CODE_200, 
            "{\"name\":\"SwarmApp API\",\"version\":\"1.0.0\",\"description\":\"Distributed, modular C++ application framework API\",\"documentation_url\":\"/api/docs\"}"
        );
        response->putHeader("Content-Type", "application/json");
        return response;
    }
    else if (path == "/" || path == "" || path == "root") {
        auto response = oatpp::web::protocol::http::outgoing::ResponseFactory::createResponse(
            oatpp::web::protocol::http::Status::CODE_200, 
            "{\"name\":\"SwarmApp\",\"version\":\"1.0.0\",\"description\":\"Welcome to SwarmApp API\",\"documentation_url\":\"/api/info\"}"
        );
        response->putHeader("Content-Type", "application/json");
        return response;
    }
    else {
        // 404 handler
        auto response = oatpp::web::protocol::http::outgoing::ResponseFactory::createResponse(
            oatpp::web::protocol::http::Status::CODE_404, 
            "{\"code\":404,\"message\":\"Endpoint not found\",\"details\":\"The requested endpoint does not exist\"}"
        );
        response->putHeader("Content-Type", "application/json");
        return response;
    }
}

ApiModule::ApiModule()
    : m_host("127.0.0.1")
    , m_port(8080)
    , m_maxConnections(100)
    , m_corsEnabled(true)
    , m_running(false)
    , m_requestCount(0)
    , m_activeConnections(0) {
}

ApiModule::~ApiModule() {
    shutdown();
}

bool ApiModule::initialize() {
    try {
        // Initialize Oat++ environment
        oatpp::Environment::init();
        
        // Create router
        m_router = oatpp::web::server::HttpRouter::createShared();
        
        // Create HTTP handler
        m_httpHandler = std::make_shared<SimpleHttpHandler>();
        
        // Add handler to router for all paths
        m_router->route("GET", "/*", m_httpHandler);
        m_router->route("POST", "/*", m_httpHandler);
        m_router->route("PUT", "/*", m_httpHandler);
        m_router->route("DELETE", "/*", m_httpHandler);
        
        // Create connection handler
        m_connectionHandler = oatpp::web::server::HttpConnectionHandler::createShared(m_router);
        
        // Create connection provider
        m_connectionProvider = oatpp::network::tcp::server::ConnectionProvider::createShared(
            {m_host, static_cast<v_uint16>(m_port)}
        );
        
        // Create server
        m_server = oatpp::network::Server::createShared(m_connectionProvider, m_connectionHandler);
        
        std::cout << "API Module initialized on " << m_host << ":" << m_port << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize API Module: " << e.what() << std::endl;
        return false;
    }
}

void ApiModule::start() {
    if (!m_server) {
        std::cerr << "API Module not initialized" << std::endl;
        return;
    }
    
    try {
        m_running = true;
        std::cout << "Starting API Module server..." << std::endl;
        
        // Start server in a separate thread
        m_server->run();
        
        std::cout << "API Module server started successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to start API Module: " << e.what() << std::endl;
        m_running = false;
    }
}

void ApiModule::stop() {
    if (m_server && m_running) {
        std::cout << "Stopping API Module server..." << std::endl;
        m_server->stop();
        m_running = false;
        std::cout << "API Module server stopped" << std::endl;
    }
}

void ApiModule::shutdown() {
    stop();
    
    // Clean up Oat++ components
    m_server.reset();
    m_connectionHandler.reset();
    m_connectionProvider.reset();
    m_router.reset();
    m_httpHandler.reset();
    
    // Shutdown Oat++ environment
    oatpp::Environment::destroy();
    
    std::cout << "API Module shutdown complete" << std::endl;
}

std::string ApiModule::getName() const {
    return "api";
}

std::string ApiModule::getVersion() const {
    return "1.0.0";
}

std::vector<std::string> ApiModule::getDependencies() const {
    return {};
}

bool ApiModule::isRunning() const {
    return m_running.load();
}

std::string ApiModule::getStatus() const {
    std::ostringstream oss;
    oss << "API Module (host: " << m_host 
        << ", port: " << m_port 
        << ", running: " << (m_running.load() ? "yes" : "no")
        << ", requests: " << m_requestCount.load()
        << ", connections: " << m_activeConnections.load() << ")";
    return oss.str();
}

bool ApiModule::configure(const std::map<std::string, std::string>& config) {
    try {
        // Parse configuration
        auto hostIt = config.find("host");
        if (hostIt != config.end()) {
            m_host = hostIt->second;
        }
        
        auto portIt = config.find("port");
        if (portIt != config.end()) {
            m_port = std::stoi(portIt->second);
        }
        
        auto maxConnIt = config.find("max_connections");
        if (maxConnIt != config.end()) {
            m_maxConnections = std::stoi(maxConnIt->second);
        }
        
        auto corsIt = config.find("enable_cors");
        if (corsIt != config.end()) {
            m_corsEnabled = (corsIt->second == "true" || corsIt->second == "1");
        }
        
        std::cout << "API Module configured - Host: " << m_host 
                  << ", Port: " << m_port 
                  << ", Max Connections: " << m_maxConnections
                  << ", CORS: " << (m_corsEnabled ? "enabled" : "disabled") << std::endl;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to configure API Module: " << e.what() << std::endl;
        return false;
    }
}

void ApiModule::onMessage(const std::string& topic, const std::string& message) {
    // Handle incoming messages from other modules
    std::cout << "API Module received message on topic '" << topic << "': " << message << std::endl;
    
    // Increment request count for monitoring
    m_requestCount.fetch_add(1);
}

void ApiModule::setCorsEnabled(bool enabled) {
    m_corsEnabled = enabled;
}

void ApiModule::setMaxConnections(int maxConnections) {
    m_maxConnections = maxConnections;
}

void ApiModule::setupRouter() {
    if (!m_router) {
        m_router = oatpp::web::server::HttpRouter::createShared();
    }
}

void ApiModule::logRequest(const std::string& method, const std::string& path) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::cout << "[" << std::ctime(&time_t) << "] " 
              << method << " " << path 
              << " (requests: " << m_requestCount.load() << ")" << std::endl;
}

} // namespace swarm
