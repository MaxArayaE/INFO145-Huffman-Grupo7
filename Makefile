CXX      := g++
CXXFLAGS := -O3 -std=c++17 -Wall -Wextra -Wpedantic -march=native
TARGET   := main
SRC      := main.cpp
HEADERS  := $(wildcard include/*.hpp)

.PHONY: all clean valgrind

all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
	rm -f bench/*.csv

valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./$(TARGET) -i data/small.csv
