CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra
LIBS := -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
EXE := $(if $(filter Windows_NT,$(OS)),.exe,)
TARGET := vice-project-doom$(EXE)

.PHONY: all clean

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o $@ $(LIBS)

clean:
	rm -f $(TARGET)
