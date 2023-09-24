CXXFLAGS = -std=c++23 -Wall -Wextra -O3

SRC = ./src/main.cpp
HEADERS = $(wildcard ./src/*.hpp)
OBJ = $(SRC:.cpp=.o)

TARGET = s++

all: $(TARGET)

$(OBJ): $(HEADERS)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET)

clean:
	rm -f $(TARGET) $(OBJ)