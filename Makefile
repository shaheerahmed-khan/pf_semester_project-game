CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra
LDFLAGS ?= -lsfml-graphics -lsfml-window -lsfml-system

ifeq ($(OS),Windows_NT)
EXE := .exe
else
EXE :=
endif

TARGET := vice-project-doom$(EXE)
SRC := src/main.cpp src/enemies.cpp src/level.cpp src/player.cpp src/physics.cpp
OBJ := $(SRC:.cpp=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)
