CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I./includes
LDFLAGS = -L./lib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

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
