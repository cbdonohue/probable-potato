#include "modules/api_module.h"
#include <iostream>
#include <signal.h>
#include <memory>

std::unique_ptr<swarm::ApiModule> g_apiModule;
bool g_running = true;

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ". Shutting down..." << std::endl;
    g_running = false;
    
    if (g_apiModule) {
        g_apiModule->stop();
    }
}

int main(int argc, char* argv[]) {
    // Set up signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    std::cout << "Starting SwarmApp API Server..." << std::endl;
    
    try {
        // Create API module
        g_apiModule = std::make_unique<swarm::ApiModule>();
        
        // Configure the API module
        std::map<std::string, std::string> config = {
            {"host", "127.0.0.1"},
            {"port", "8080"},
            {"max_connections", "100"},
            {"enable_cors", "true"}
        };
        
        // Parse command line arguments
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "--host" && i + 1 < argc) {
                config["host"] = argv[++i];
            } else if (arg == "--port" && i + 1 < argc) {
                config["port"] = argv[++i];
            } else if (arg == "--max-connections" && i + 1 < argc) {
                config["max_connections"] = argv[++i];
            } else if (arg == "--no-cors") {
                config["enable_cors"] = "false";
            } else if (arg == "--help" || arg == "-h") {
                std::cout << "Usage: " << argv[0] << " [OPTIONS]" << std::endl;
                std::cout << "Options:" << std::endl;
                std::cout << "  --host HOST           Server host (default: 127.0.0.1)" << std::endl;
                std::cout << "  --port PORT           Server port (default: 8080)" << std::endl;
                std::cout << "  --max-connections N   Maximum connections (default: 100)" << std::endl;
                std::cout << "  --no-cors             Disable CORS" << std::endl;
                std::cout << "  --help, -h            Show this help message" << std::endl;
                return 0;
            }
        }
        
        // Configure and initialize the module
        if (!g_apiModule->configure(config)) {
            std::cerr << "Failed to configure API module" << std::endl;
            return 1;
        }
        
        if (!g_apiModule->initialize()) {
            std::cerr << "Failed to initialize API module" << std::endl;
            return 1;
        }
        
        std::cout << "API Server configured and initialized" << std::endl;
        std::cout << "Server will start on " << config["host"] << ":" << config["port"] << std::endl;
        std::cout << "Available endpoints:" << std::endl;
        std::cout << "  GET /              - API information" << std::endl;
        std::cout << "  GET /health        - Health check" << std::endl;
        std::cout << "  GET /status        - Server status" << std::endl;
        std::cout << "  GET /api/info      - API information" << std::endl;
        std::cout << "Press Ctrl+C to stop the server" << std::endl;
        
        // Start the API server
        g_apiModule->start();
        
        // Keep the server running
        while (g_running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        std::cout << "API Server stopped" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
