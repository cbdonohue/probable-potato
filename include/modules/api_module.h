#pragma once

#include "core/module.h"
#include <oatpp/network/Server.hpp>
#include <oatpp/network/tcp/server/ConnectionProvider.hpp>
#include <oatpp/web/server/HttpConnectionHandler.hpp>
#include <oatpp/web/server/HttpRouter.hpp>
#include <oatpp/Environment.hpp>
#include <memory>
#include <string>
#include <map>
#include <atomic>

namespace swarm {

// Simple HTTP Request Handler
class SimpleHttpHandler : public oatpp::web::server::HttpRequestHandler {
public:
    std::shared_ptr<oatpp::web::protocol::http::outgoing::Response> handle(
        const std::shared_ptr<oatpp::web::protocol::http::incoming::Request>& request) override;
};

// API Module class
class ApiModule : public Module {
public:
    ApiModule();
    ~ApiModule() override;
    
    // Module interface implementation
    bool initialize() override;
    void start() override;
    void stop() override;
    void shutdown() override;
    std::string getName() const override;
    std::string getVersion() const override;
    std::vector<std::string> getDependencies() const override;
    bool isRunning() const override;
    std::string getStatus() const override;
    bool configure(const std::map<std::string, std::string>& config) override;
    void onMessage(const std::string& topic, const std::string& message) override;
    
    // API-specific methods
    void setCorsEnabled(bool enabled);
    void setMaxConnections(int maxConnections);
    
private:
    // Oat++ components
    std::shared_ptr<oatpp::network::Server> m_server;
    std::shared_ptr<oatpp::web::server::HttpRouter> m_router;
    std::shared_ptr<oatpp::web::server::HttpConnectionHandler> m_connectionHandler;
    std::shared_ptr<oatpp::network::tcp::server::ConnectionProvider> m_connectionProvider;
    std::shared_ptr<SimpleHttpHandler> m_httpHandler;
    
    // Configuration
    std::string m_host;
    int m_port;
    int m_maxConnections;
    bool m_corsEnabled;
    std::atomic<bool> m_running;
    std::atomic<int> m_requestCount;
    std::atomic<int> m_activeConnections;
    
    // Internal methods
    void setupRouter();
    void logRequest(const std::string& method, const std::string& path);
};

} // namespace swarm
