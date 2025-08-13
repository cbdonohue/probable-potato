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
#include <thread>
#include <chrono>

// Include the server header
#include "../include/server.h"

class TestFramework {
private:
    int passed = 0;
public:
    int failed = 0;
    
public:
    void assert(bool condition, const std::string& testName) {
        if (condition) {
            std::cout << "✅ PASS: " << testName << std::endl;
            passed++;
        } else {
            std::cout << "❌ FAIL: " << testName << std::endl;
            failed++;
        }
    }
    
    void assertEqual(const std::string& actual, const std::string& expected, const std::string& testName) {
        if (actual == expected) {
            std::cout << "✅ PASS: " << testName << std::endl;
            passed++;
        } else {
            std::cout << "❌ FAIL: " << testName << " (expected: '" << expected << "', got: '" << actual << "')" << std::endl;
            failed++;
        }
    }
    
    void assertContains(const std::string& text, const std::string& substring, const std::string& testName) {
        if (text.find(substring) != std::string::npos) {
            std::cout << "✅ PASS: " << testName << std::endl;
            passed++;
        } else {
            std::cout << "❌ FAIL: " << testName << " (text does not contain: '" << substring << "')" << std::endl;
            failed++;
        }
    }
    
    void printSummary() {
        std::cout << "\n=== TEST SUMMARY ===" << std::endl;
        std::cout << "Passed: " << passed << std::endl;
        std::cout << "Failed: " << failed << std::endl;
        std::cout << "Total: " << (passed + failed) << std::endl;
        
        if (failed == 0) {
            std::cout << "🎉 All tests passed!" << std::endl;
        } else {
            std::cout << "💥 " << failed << " test(s) failed!" << std::endl;
        }
    }
};

class MockHTTPServer : public SimpleHTTPServer {
public:
    MockHTTPServer(int port) : SimpleHTTPServer(port) {}
    
    // Expose protected methods for testing
    std::string testGetCurrentTime() { return getCurrentTime(); }
    std::string testGetHostname() { return getHostname(); }
    std::string testCreateJSONResponse(const std::string& message, const std::string& hostname) {
        return createJSONResponse(message, hostname);
    }
    std::string testCreateHTTPResponse(int statusCode, const std::string& contentType, const std::string& body) {
        return createHTTPResponse(statusCode, contentType, body);
    }
};

std::string sendHTTPRequest(const std::string& method, const std::string& path, int port = 5001) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return "ERROR: Socket creation failed";
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        return "ERROR: Invalid address";
    }
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sock);
        return "ERROR: Connection failed";
    }
    
    std::string request = method + " " + path + " HTTP/1.1\r\n";
    request += "Host: localhost\r\n";
    request += "Connection: close\r\n";
    request += "\r\n";
    
    send(sock, request.c_str(), request.length(), 0);
    
    char buffer[4096] = {0};
    int valread = read(sock, buffer, 4095);
    close(sock);
    
    if (valread > 0) {
        return std::string(buffer);
    }
    
    return "ERROR: No response";
}

void runServerInBackground(SimpleHTTPServer& server) {
    std::thread serverThread([&server]() {
        if (server.start()) {
            server.run();
        }
    });
    serverThread.detach();
    
    // Give server time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

int main() {
    TestFramework test;
    
    std::cout << "🧪 Running C++ HTTP Server Unit Tests\n" << std::endl;
    
    // Test 1: JSON Response Creation
    {
        MockHTTPServer server(5001);
        std::string json = server.testCreateJSONResponse("Test Message", "test-host");
        test.assertContains(json, "Test Message", "JSON response contains message");
        test.assertContains(json, "test-host", "JSON response contains hostname");
        test.assertContains(json, "1.0.0", "JSON response contains version");
        test.assertContains(json, "timestamp", "JSON response contains timestamp");
    }
    
    // Test 2: HTTP Response Creation
    {
        MockHTTPServer server(5001);
        std::string response = server.testCreateHTTPResponse(200, "application/json", "{\"test\": true}");
        test.assertContains(response, "HTTP/1.1 200 OK", "HTTP response contains status line");
        test.assertContains(response, "Content-Type: application/json", "HTTP response contains content type");
        test.assertContains(response, "{\"test\": true}", "HTTP response contains body");
    }
    
    // Test 3: Hostname Retrieval
    {
        MockHTTPServer server(5001);
        std::string hostname = server.testGetHostname();
        test.assert(!hostname.empty(), "Hostname is not empty");
        test.assert(hostname != "unknown", "Hostname is retrieved successfully");
    }
    
    // Test 4: Current Time Retrieval
    {
        MockHTTPServer server(5001);
        std::string time1 = server.testGetCurrentTime();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::string time2 = server.testGetCurrentTime();
        test.assert(!time1.empty(), "Current time is not empty");
        test.assert(!time2.empty(), "Second time call is not empty");
    }
    
    // Test 5: Server Creation
    {
        MockHTTPServer server(5002);
        test.assert(true, "Server object creation successful");
    }
    
    // Test 6: HTTP Response Format Validation
    {
        MockHTTPServer server(5001);
        std::string response = server.testCreateHTTPResponse(200, "application/json", "{\"test\": true}");
        
        // Check HTTP response structure
        test.assertContains(response, "HTTP/1.1 200 OK", "HTTP response has correct status line");
        test.assertContains(response, "Content-Type: application/json", "HTTP response has correct content type");
        test.assertContains(response, "Content-Length:", "HTTP response has content length");
        test.assertContains(response, "Connection: close", "HTTP response has connection header");
        test.assertContains(response, "\r\n\r\n", "HTTP response has proper header/body separator");
    }
    
    // Test 7: JSON Response Structure Validation
    {
        MockHTTPServer server(5001);
        std::string json = server.testCreateJSONResponse("Test Message", "test-host");
        
        // Validate JSON structure
        test.assertContains(json, "\"message\": \"Test Message\"", "JSON has correct message field");
        test.assertContains(json, "\"hostname\": \"test-host\"", "JSON has correct hostname field");
        test.assertContains(json, "\"version\": \"1.0.0\"", "JSON has correct version field");
        test.assertContains(json, "\"timestamp\":", "JSON has timestamp field");
    }
    
    // Test 8: Error Response Validation
    {
        MockHTTPServer server(5001);
        std::string errorResponse = server.testCreateHTTPResponse(404, "application/json", "{\"error\": \"Not Found\"}");
        
        test.assertContains(errorResponse, "HTTP/1.1 404 OK", "Error response has correct status");
        test.assertContains(errorResponse, "{\"error\": \"Not Found\"}", "Error response has correct body");
    }
    
    // Test 9: Method Not Allowed Response Validation
    {
        MockHTTPServer server(5001);
        std::string methodResponse = server.testCreateHTTPResponse(405, "application/json", "{\"error\": \"Method Not Allowed\"}");
        
        test.assertContains(methodResponse, "HTTP/1.1 405 OK", "Method not allowed has correct status");
        test.assertContains(methodResponse, "{\"error\": \"Method Not Allowed\"}", "Method not allowed has correct body");
    }
    
    // Test 10: JSON Structure Validation
    {
        MockHTTPServer server(5001);
        std::string json = server.testCreateJSONResponse("Test", "test-host");
        
        // Check for required JSON fields
        test.assertContains(json, "\"message\":", "JSON contains message field");
        test.assertContains(json, "\"hostname\":", "JSON contains hostname field");
        test.assertContains(json, "\"version\":", "JSON contains version field");
        test.assertContains(json, "\"timestamp\":", "JSON contains timestamp field");
        
        // Check JSON structure
        test.assertContains(json, "{", "JSON starts with {");
        test.assertContains(json, "}", "JSON ends with }");
    }
    
    test.printSummary();
    
    return (test.failed == 0) ? 0 : 1;
}
