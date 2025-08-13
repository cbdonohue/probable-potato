#include "../../../include/modules/http_server_module.h"
#include <iostream>
#include <signal.h>

using namespace swarm;

HttpServerModule* g_server = nullptr;

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ", shutting down HTTP server..." << std::endl;
    if (g_server) {
        g_server->stop();
    }
    exit(0);
}

int main() {
    std::cout << "🚀 Starting HTTP Server Module (Standalone)" << std::endl;

    // Set up signal handling
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    try {
        // Create and configure HTTP server module
        auto server = std::make_unique<HttpServerModule>();
        g_server = server.get();

        // Configure the server
        std::map<std::string, std::string> config = {
            {"port", "8080"},
            {"host", "0.0.0.0"},
            {"max_connections", "100"},
            {"enable_cors", "true"}
        };

        if (!server->configure(config)) {
            std::cerr << "❌ Failed to configure HTTP server module" << std::endl;
            return 1;
        }

        if (!server->initialize()) {
            std::cerr << "❌ Failed to initialize HTTP server module" << std::endl;
            return 1;
        }

        std::cout << "✅ HTTP Server module initialized successfully" << std::endl;

        // Start the server
        server->start();

        std::cout << "🎯 HTTP Server is running on port 8080" << std::endl;
        std::cout << "📊 Available endpoints:" << std::endl;
        std::cout << "   GET http://localhost:8080/ - Main endpoint" << std::endl;
        std::cout << "   GET http://localhost:8080/health - Health check" << std::endl;
        std::cout << "   GET http://localhost:8080/status - Server status" << std::endl;
        std::cout << "🔧 Press Ctrl+C to stop" << std::endl;

        // Keep the server running
        while (server->isRunning()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

    } catch (const std::exception& e) {
        std::cerr << "💥 Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
