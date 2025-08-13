#include "../include/core/module_manager.h"
#include <iostream>
#include <signal.h>
#include <thread>
#include <chrono>

using namespace swarm;

ModuleManager* g_moduleManager = nullptr;

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ", shutting down Core Service..." << std::endl;
    if (g_moduleManager) {
        g_moduleManager->shutdownAllModules();
    }
    exit(0);
}

int main() {
    std::cout << "🚀 Starting SwarmApp Core Service" << std::endl;

    // Set up signal handling
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    try {
        // Create module manager (this starts the message bus)
        ModuleManager moduleManager;
        g_moduleManager = &moduleManager;

        std::cout << "✅ Core Service initialized successfully" << std::endl;
        std::cout << "📡 Message Bus is running" << std::endl;
        std::cout << "🔧 Press Ctrl+C to stop" << std::endl;

        // Keep the core service running
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            
            // Print status every 10 seconds
            std::cout << "\n📈 Core Service Status:" << std::endl;
            std::cout << "   Message Bus: " << (moduleManager.getMessageBus()->isRunning() ? "✅ Running" : "❌ Stopped") << std::endl;
            std::cout << "   Messages Processed: " << moduleManager.getMessageBus()->getMessageCount() << std::endl;
            
            auto loadedModules = moduleManager.getLoadedModules();
            std::cout << "   Loaded Modules: " << loadedModules.size() << std::endl;
            for (const auto& moduleName : loadedModules) {
                std::cout << "     - " << moduleName << ": " << (moduleManager.isModuleRunning(moduleName) ? "Running" : "Stopped") << std::endl;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "💥 Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
