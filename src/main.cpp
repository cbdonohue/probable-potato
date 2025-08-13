#include "../include/core/module_manager.h"
#include "../include/modules/http_server_module.h"
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
    std::cout << "🚀 Starting SwarmApp HTTP Server" << std::endl;
    
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
        
        std::cout << "📦 Registered modules: http-server" << std::endl;
        
        // Load and configure modules
        std::map<std::string, std::string> httpConfig = {
            {"port", "8080"},
            {"host", "0.0.0.0"},
            {"max_connections", "100"},
            {"enable_cors", "true"}
        };
        
        // Load modules
        if (!moduleManager.loadModule("http-server", httpConfig)) {
            std::cerr << "❌ Failed to load http-server module" << std::endl;
            return 1;
        }
        
        std::cout << "✅ Modules loaded successfully" << std::endl;
        
        // Start all modules
        moduleManager.startAllModules();
        
        std::cout << "🎯 HTTP Server is running..." << std::endl;
        std::cout << "📊 Available endpoints:" << std::endl;
        std::cout << "   GET http://localhost:8080/ - Main endpoint" << std::endl;
        std::cout << "   GET http://localhost:8080/health - Health check" << std::endl;
        std::cout << "   GET http://localhost:8080/status - Module status" << std::endl;
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
