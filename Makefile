CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2 -pthread
TARGET = swarm-app
TEST_TARGET = test-swarm-app
SOURCE = main.cpp
TEST_SOURCE = test_main.cpp

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

test: $(TEST_TARGET)

$(TEST_TARGET): $(TEST_SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $(TEST_SOURCE)

clean:
	rm -f $(TARGET) $(TEST_TARGET)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

.PHONY: all test clean install
