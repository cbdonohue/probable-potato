#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include <ctime>

class SimpleHTTPServer {
private:
    int server_fd;
    int port;
    
    std::string getCurrentTime() {
        time_t now = time(0);
        char* dt = ctime(&now);
        std::string timeStr(dt);
        timeStr.pop_back(); // Remove newline
        return timeStr;
    }
    
    std::string getHostname() {
        char hostname[256];
        if (gethostname(hostname, sizeof(hostname)) == 0) {
            return std::string(hostname);
        }
        return "unknown";
    }
    
    std::string createJSONResponse(const std::string& message, const std::string& hostname) {
        std::ostringstream json;
        json << "{\n";
        json << "  \"message\": \"" << message << "\",\n";
        json << "  \"hostname\": \"" << hostname << "\",\n";
        json << "  \"version\": \"1.0.0\",\n";
        json << "  \"timestamp\": \"" << getCurrentTime() << "\"\n";
        json << "}";
        return json.str();
    }
    
    std::string createHTTPResponse(int statusCode, const std::string& contentType, const std::string& body) {
        std::ostringstream response;
        response << "HTTP/1.1 " << statusCode << " OK\r\n";
        response << "Content-Type: " << contentType << "\r\n";
        response << "Content-Length: " << body.length() << "\r\n";
        response << "Access-Control-Allow-Origin: *\r\n";
        response << "Connection: close\r\n";
        response << "\r\n";
        response << body;
        return response.str();
    }
    
    void handleRequest(int client_socket) {
        char buffer[1024] = {0};
        int valread = read(client_socket, buffer, 1023);
        
        if (valread > 0) {
            std::string request(buffer);
            std::string response;
            
            // Parse the first line to get the method and path
            std::istringstream requestStream(request);
            std::string method, path, protocol;
            requestStream >> method >> path >> protocol;
            
            if (method == "GET") {
                if (path == "/" || path == "/index.html") {
                    // Main endpoint
                    std::string jsonBody = createJSONResponse("Hello from Docker Swarm!", getHostname());
                    response = createHTTPResponse(200, "application/json", jsonBody);
                } else if (path == "/health") {
                    // Health endpoint
                    std::string jsonBody = "{\"status\": \"healthy\"}";
                    response = createHTTPResponse(200, "application/json", jsonBody);
                } else {
                    // 404 Not Found
                    std::string jsonBody = "{\"error\": \"Not Found\", \"path\": \"" + path + "\"}";
                    response = createHTTPResponse(404, "application/json", jsonBody);
                }
            } else {
                // Method not allowed
                std::string jsonBody = "{\"error\": \"Method Not Allowed\"}";
                response = createHTTPResponse(405, "application/json", jsonBody);
            }
            
            send(client_socket, response.c_str(), response.length(), 0);
        }
        
        close(client_socket);
    }

public:
    SimpleHTTPServer(int p) : port(p) {}
    
    bool start() {
        struct sockaddr_in address;
        int opt = 1;
        int addrlen = sizeof(address);
        
        // Create socket
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            std::cerr << "Socket creation failed" << std::endl;
            return false;
        }
        
        // Set socket options
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
            std::cerr << "Setsockopt failed" << std::endl;
            return false;
        }
        
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        
        // Bind socket
        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            std::cerr << "Bind failed" << std::endl;
            return false;
        }
        
        // Listen for connections
        if (listen(server_fd, 3) < 0) {
            std::cerr << "Listen failed" << std::endl;
            return false;
        }
        
        std::cout << "Server started on port " << port << std::endl;
        std::cout << "Available endpoints:" << std::endl;
        std::cout << "  GET / - Main endpoint" << std::endl;
        std::cout << "  GET /health - Health check" << std::endl;
        
        return true;
    }
    
    void run() {
        struct sockaddr_in address;
        int addrlen = sizeof(address);
        
        while (true) {
            int new_socket;
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                std::cerr << "Accept failed" << std::endl;
                continue;
            }
            
            handleRequest(new_socket);
        }
    }
    
    ~SimpleHTTPServer() {
        if (server_fd > 0) {
            close(server_fd);
        }
    }
};

int main() {
    SimpleHTTPServer server(5000);
    
    if (!server.start()) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }
    
    server.run();
    
    return 0;
}
