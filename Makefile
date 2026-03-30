CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I. -I./includes -I/usr/include/lua5.3
LDFLAGS = -L./lib -lraylib -llua5.3 -lGL -lm -lpthread -ldl -lrt -lX11

TARGET = browser
SRC = $(wildcard *.cpp) \
      $(wildcard ui/*.cpp) \
      $(wildcard parser/*.cpp) \
      $(wildcard network/*.cpp)

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
