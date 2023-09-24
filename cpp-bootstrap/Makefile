CXXFLAGS = -std=c++23 -Wall -Wextra -O3

SRC = ./main.cpp
HEADERS = $(wildcard ./*.hpp)
OBJ = $(SRC:.cpp=.o)

TARGET = s++

all: $(TARGET)

$(OBJ): $(HEADERS)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET)

clean:
	rm -f $(TARGET) $(OBJ)