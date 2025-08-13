CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2 -pthread -I./include
TARGET = swarm-app
TEST_TARGET = test-swarm-app
SOURCE = src/main.cpp
TEST_SOURCE = tests/test_main.cpp

# Google Test flags
GTEST_FLAGS = -lgtest -lgtest_main -lgmock -lpthread

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

test: $(TEST_TARGET)

$(TEST_TARGET): $(TEST_SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $(TEST_SOURCE) $(GTEST_FLAGS)

clean:
	rm -f $(TARGET) $(TEST_TARGET)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Run tests
run-tests: test
	./$(TEST_TARGET)

# Build and test
check: test run-tests

.PHONY: all test clean install run-tests check
