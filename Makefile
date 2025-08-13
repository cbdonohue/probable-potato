CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
TARGET = swarm-app
SOURCE = main.cpp

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

.PHONY: all clean install
