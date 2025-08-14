#include "../include/core/module_manager.h"
#include "../include/modules/http_server_module.h"
#include "../include/modules/health_monitor_module.h"
#include "../include/modules/api_module.h"
#include <iostream>
#include <signal.h>
#include <unistd.h>

using namespace swarm;

ModuleManager* g_moduleManager = nullptr;

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ", shutting down..." << std::endl;
    if (g_moduleManager) {
        g_moduleManager->shutdownAllModules();
    }
    exit(0);
}

int main() {
    std::cout << "🚀 Starting SwarmApp with Multiple Modules" << std::endl;
    
    // Set up signal handling
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    try {
        // Create module manager
        ModuleManager moduleManager;
        g_moduleManager = &moduleManager;
        
        // Register modules
        moduleManager.registerModule("http-server", []() {
            return std::make_unique<HttpServerModule>();
        });
        
        moduleManager.registerModule("health-monitor", []() {
            return std::make_unique<HealthMonitorModule>();
        });
        
        moduleManager.registerModule("api", []() {
            return std::make_unique<ApiModule>();
        });
        
        std::cout << "📦 Registered modules: http-server, health-monitor, api" << std::endl;
        
        // Load and configure HTTP server module
        std::map<std::string, std::string> httpConfig = {
            {"port", "8082"},
            {"host", "0.0.0.0"},
            {"max_connections", "100"},
            {"enable_cors", "true"}
        };
        
        // Load and configure health monitor module
        std::map<std::string, std::string> healthConfig = {
            {"default_timeout_ms", "5000"},
            {"default_interval_ms", "10000"},
            {"max_failures", "3"},
            {"enable_notifications", "true"}
        };
        
        // Load and configure API module
        std::map<std::string, std::string> apiConfig = {
            {"port", "8083"},
            {"host", "0.0.0.0"},
            {"max_connections", "100"},
            {"enable_cors", "true"}
        };
        
        // Load modules
        if (!moduleManager.loadModule("http-server", httpConfig)) {
            std::cerr << "❌ Failed to load http-server module" << std::endl;
            return 1;
        }
        
        if (!moduleManager.loadModule("health-monitor", healthConfig)) {
            std::cerr << "❌ Failed to load health-monitor module" << std::endl;
            return 1;
        }
        
        if (!moduleManager.loadModule("api", apiConfig)) {
            std::cerr << "❌ Failed to load api module" << std::endl;
            return 1;
        }
        
        // Add health checks to monitor the HTTP server
        auto healthMonitor = moduleManager.getModule("health-monitor");
        if (auto* hm = dynamic_cast<HealthMonitorModule*>(healthMonitor)) {
            HealthCheckConfig httpCheck = {
                "http-server", "http", "http://localhost:8082/health", 5000, 10000, 3
            };
            hm->addHealthCheck(httpCheck);
            
            HealthCheckConfig mainCheck = {
                "main-endpoint", "http", "http://localhost:8082/", 5000, 15000, 3
            };
            hm->addHealthCheck(mainCheck);
            
            std::cout << "📋 Added health checks for HTTP server" << std::endl;
        }
        
        std::cout << "✅ Modules loaded successfully" << std::endl;
        
        // Start all modules
        moduleManager.startAllModules();
        
        std::cout << "🎯 Application is running..." << std::endl;
        std::cout << "📊 Available endpoints:" << std::endl;
        std::cout << "   HTTP Server (Legacy):" << std::endl;
        std::cout << "     GET http://localhost:8082/ - Main endpoint" << std::endl;
        std::cout << "     GET http://localhost:8082/health - Health check" << std::endl;
        std::cout << "     GET http://localhost:8082/status - Module status" << std::endl;
        std::cout << "   API Server (Oat++):" << std::endl;
        std::cout << "     GET http://localhost:8083/ - API information" << std::endl;
        std::cout << "     GET http://localhost:8083/health - Health check" << std::endl;
        std::cout << "     GET http://localhost:8083/status - Server status" << std::endl;
        std::cout << "     GET http://localhost:8083/api/info - API information" << std::endl;
        std::cout << "🔧 Press Ctrl+C to stop" << std::endl;
        
        // Keep the application running
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            
            // Print status every 10 seconds
            auto statuses = moduleManager.getModuleStatuses();
            std::cout << "\n📈 Module Status:" << std::endl;
            for (const auto& [name, status] : statuses) {
                std::cout << "   " << name << ": " << status << std::endl;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "💥 Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
