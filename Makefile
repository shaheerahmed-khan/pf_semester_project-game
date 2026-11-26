CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra
LDFLAGS ?= -lsfml-graphics -lsfml-window -lsfml-system

ifeq ($(OS),Windows_NT)
EXE := .exe
else
EXE :=
endif

TARGET := vice-project-doom$(EXE)
SRC := src/main.cpp

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
