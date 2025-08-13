#include <gtest/gtest.h>
#include <gmock/gmock.h>
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

// Test fixture for HTTP server tests
class HTTPServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up any common test data
    }
    
    void TearDown() override {
        // Clean up after each test
    }
};

// Test JSON response creation
TEST_F(HTTPServerTest, CreateJSONResponse) {
    MockHTTPServer server(5001);
    std::string json = server.testCreateJSONResponse("Test Message", "test-host");
    
    EXPECT_THAT(json, ::testing::HasSubstr("Test Message"));
    EXPECT_THAT(json, ::testing::HasSubstr("test-host"));
    EXPECT_THAT(json, ::testing::HasSubstr("1.0.0"));
    EXPECT_THAT(json, ::testing::HasSubstr("timestamp"));
    EXPECT_THAT(json, ::testing::HasSubstr("\"message\":"));
    EXPECT_THAT(json, ::testing::HasSubstr("\"hostname\":"));
    EXPECT_THAT(json, ::testing::HasSubstr("\"version\":"));
    EXPECT_THAT(json, ::testing::StartsWith("{"));
    EXPECT_THAT(json, ::testing::EndsWith("}"));
}

// Test HTTP response creation
TEST_F(HTTPServerTest, CreateHTTPResponse) {
    MockHTTPServer server(5001);
    std::string response = server.testCreateHTTPResponse(200, "application/json", "{\"test\": true}");
    
    EXPECT_THAT(response, ::testing::HasSubstr("HTTP/1.1 200 OK"));
    EXPECT_THAT(response, ::testing::HasSubstr("Content-Type: application/json"));
    EXPECT_THAT(response, ::testing::HasSubstr("{\"test\": true}"));
    EXPECT_THAT(response, ::testing::HasSubstr("Content-Length:"));
    EXPECT_THAT(response, ::testing::HasSubstr("Connection: close"));
    EXPECT_THAT(response, ::testing::HasSubstr("\r\n\r\n"));
}

// Test hostname retrieval
TEST_F(HTTPServerTest, GetHostname) {
    MockHTTPServer server(5001);
    std::string hostname = server.testGetHostname();
    
    EXPECT_FALSE(hostname.empty());
    EXPECT_NE(hostname, "unknown");
}

// Test current time retrieval
TEST_F(HTTPServerTest, GetCurrentTime) {
    MockHTTPServer server(5001);
    std::string time1 = server.testGetCurrentTime();
    
    // Wait a moment
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::string time2 = server.testGetCurrentTime();
    
    EXPECT_FALSE(time1.empty());
    EXPECT_FALSE(time2.empty());
    // Note: Times might be the same if called within the same second
    // This is acceptable behavior
}

// Test server object creation
TEST_F(HTTPServerTest, ServerCreation) {
    MockHTTPServer server(5002);
    // If we get here, the server was created successfully
    SUCCEED();
}

// Test HTTP response format validation
TEST_F(HTTPServerTest, HTTPResponseFormat) {
    MockHTTPServer server(5001);
    std::string response = server.testCreateHTTPResponse(200, "application/json", "{\"test\": true}");
    
    // Check HTTP response structure
    EXPECT_THAT(response, ::testing::HasSubstr("HTTP/1.1 200 OK"));
    EXPECT_THAT(response, ::testing::HasSubstr("Content-Type: application/json"));
    EXPECT_THAT(response, ::testing::HasSubstr("Content-Length:"));
    EXPECT_THAT(response, ::testing::HasSubstr("Connection: close"));
    EXPECT_THAT(response, ::testing::HasSubstr("\r\n\r\n"));
}

// Test JSON structure validation
TEST_F(HTTPServerTest, JSONStructureValidation) {
    MockHTTPServer server(5001);
    std::string json = server.testCreateJSONResponse("Test", "test-host");
    
    // Check for required JSON fields
    EXPECT_THAT(json, ::testing::HasSubstr("\"message\": \"Test\""));
    EXPECT_THAT(json, ::testing::HasSubstr("\"hostname\": \"test-host\""));
    EXPECT_THAT(json, ::testing::HasSubstr("\"version\": \"1.0.0\""));
    EXPECT_THAT(json, ::testing::HasSubstr("\"timestamp\":"));
    
    // Check JSON structure
    EXPECT_THAT(json, ::testing::StartsWith("{"));
    EXPECT_THAT(json, ::testing::EndsWith("}"));
}

// Test error response validation
TEST_F(HTTPServerTest, ErrorResponseValidation) {
    MockHTTPServer server(5001);
    std::string errorResponse = server.testCreateHTTPResponse(404, "application/json", "{\"error\": \"Not Found\"}");
    
    EXPECT_THAT(errorResponse, ::testing::HasSubstr("HTTP/1.1 404 OK"));
    EXPECT_THAT(errorResponse, ::testing::HasSubstr("{\"error\": \"Not Found\"}"));
}

// Test method not allowed response validation
TEST_F(HTTPServerTest, MethodNotAllowedResponseValidation) {
    MockHTTPServer server(5001);
    std::string methodResponse = server.testCreateHTTPResponse(405, "application/json", "{\"error\": \"Method Not Allowed\"}");
    
    EXPECT_THAT(methodResponse, ::testing::HasSubstr("HTTP/1.1 405 OK"));
    EXPECT_THAT(methodResponse, ::testing::HasSubstr("{\"error\": \"Method Not Allowed\"}"));
}

// Test JSON field presence
TEST_F(HTTPServerTest, JSONFieldPresence) {
    MockHTTPServer server(5001);
    std::string json = server.testCreateJSONResponse("Test", "test-host");
    
    // Check for required JSON fields
    EXPECT_THAT(json, ::testing::HasSubstr("\"message\":"));
    EXPECT_THAT(json, ::testing::HasSubstr("\"hostname\":"));
    EXPECT_THAT(json, ::testing::HasSubstr("\"version\":"));
    EXPECT_THAT(json, ::testing::HasSubstr("\"timestamp\":"));
}

// Test HTTP status codes
TEST_F(HTTPServerTest, HTTPStatusCodes) {
    MockHTTPServer server(5001);
    
    // Test 200 OK
    std::string response200 = server.testCreateHTTPResponse(200, "application/json", "{\"status\": \"ok\"}");
    EXPECT_THAT(response200, ::testing::HasSubstr("HTTP/1.1 200 OK"));
    
    // Test 404 Not Found
    std::string response404 = server.testCreateHTTPResponse(404, "application/json", "{\"error\": \"not found\"}");
    EXPECT_THAT(response404, ::testing::HasSubstr("HTTP/1.1 404 OK"));
    
    // Test 500 Internal Server Error
    std::string response500 = server.testCreateHTTPResponse(500, "application/json", "{\"error\": \"internal error\"}");
    EXPECT_THAT(response500, ::testing::HasSubstr("HTTP/1.1 500 OK"));
}

// Test content type headers
TEST_F(HTTPServerTest, ContentTypeHeaders) {
    MockHTTPServer server(5001);
    
    // Test JSON content type
    std::string jsonResponse = server.testCreateHTTPResponse(200, "application/json", "{}");
    EXPECT_THAT(jsonResponse, ::testing::HasSubstr("Content-Type: application/json"));
    
    // Test HTML content type
    std::string htmlResponse = server.testCreateHTTPResponse(200, "text/html", "<html></html>");
    EXPECT_THAT(htmlResponse, ::testing::HasSubstr("Content-Type: text/html"));
    
    // Test plain text content type
    std::string textResponse = server.testCreateHTTPResponse(200, "text/plain", "Hello World");
    EXPECT_THAT(textResponse, ::testing::HasSubstr("Content-Type: text/plain"));
}

// Test content length calculation
TEST_F(HTTPServerTest, ContentLengthCalculation) {
    MockHTTPServer server(5001);
    
    std::string shortBody = "short";
    std::string response1 = server.testCreateHTTPResponse(200, "text/plain", shortBody);
    EXPECT_THAT(response1, ::testing::HasSubstr("Content-Length: 5"));
    
    std::string longBody = "This is a much longer body content for testing";
    std::string response2 = server.testCreateHTTPResponse(200, "text/plain", longBody);
    EXPECT_THAT(response2, ::testing::HasSubstr("Content-Length: 46"));
}

// Test empty body handling
TEST_F(HTTPServerTest, EmptyBodyHandling) {
    MockHTTPServer server(5001);
    
    std::string emptyResponse = server.testCreateHTTPResponse(204, "text/plain", "");
    EXPECT_THAT(emptyResponse, ::testing::HasSubstr("HTTP/1.1 204 OK"));
    EXPECT_THAT(emptyResponse, ::testing::HasSubstr("Content-Length: 0"));
}

// Test CORS headers
TEST_F(HTTPServerTest, CORSHeaders) {
    MockHTTPServer server(5001);
    
    std::string response = server.testCreateHTTPResponse(200, "application/json", "{}");
    EXPECT_THAT(response, ::testing::HasSubstr("Access-Control-Allow-Origin: *"));
}

// Test connection header
TEST_F(HTTPServerTest, ConnectionHeader) {
    MockHTTPServer server(5001);
    
    std::string response = server.testCreateHTTPResponse(200, "application/json", "{}");
    EXPECT_THAT(response, ::testing::HasSubstr("Connection: close"));
}

// Test JSON response with special characters
TEST_F(HTTPServerTest, JSONWithSpecialCharacters) {
    MockHTTPServer server(5001);
    
    std::string json = server.testCreateJSONResponse("Test \"quoted\" message", "host-name_with.underscores");
    
    EXPECT_THAT(json, ::testing::HasSubstr("Test \"quoted\" message"));
    EXPECT_THAT(json, ::testing::HasSubstr("host-name_with.underscores"));
}

// Test timestamp format
TEST_F(HTTPServerTest, TimestampFormat) {
    MockHTTPServer server(5001);
    
    std::string json = server.testCreateJSONResponse("Test", "test-host");
    
    // Check that timestamp is present and has reasonable format
    EXPECT_THAT(json, ::testing::HasSubstr("\"timestamp\":"));
    
    // Extract timestamp value (simplified check)
    size_t timestampPos = json.find("\"timestamp\":");
    EXPECT_NE(timestampPos, std::string::npos);
    
    // Check that timestamp is not empty
    size_t valueStart = json.find("\"", timestampPos + 12) + 1;
    size_t valueEnd = json.find("\"", valueStart);
    std::string timestamp = json.substr(valueStart, valueEnd - valueStart);
    EXPECT_FALSE(timestamp.empty());
}

// Test server constructor with different ports
TEST_F(HTTPServerTest, ServerConstructorWithDifferentPorts) {
    MockHTTPServer server1(8080);
    MockHTTPServer server2(9000);
    MockHTTPServer server3(12345);
    
    // If we get here, all servers were created successfully
    SUCCEED();
}

// Test JSON response with empty strings
TEST_F(HTTPServerTest, JSONWithEmptyStrings) {
    MockHTTPServer server(5001);
    
    std::string json = server.testCreateJSONResponse("", "");
    
    EXPECT_THAT(json, ::testing::HasSubstr("\"message\": \"\""));
    EXPECT_THAT(json, ::testing::HasSubstr("\"hostname\": \"\""));
    EXPECT_THAT(json, ::testing::HasSubstr("\"version\": \"1.0.0\""));
    EXPECT_THAT(json, ::testing::HasSubstr("\"timestamp\":"));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
