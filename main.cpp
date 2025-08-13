#include "server.h"

int main() {
    SimpleHTTPServer server(5000);
    
    if (!server.start()) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }
    
    server.run();
    
    return 0;
}
