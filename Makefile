CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra -march=native -flto
TARGET = homework

SRCS = src/main.cpp src/GameState.cpp src/Board.cpp src/Move.cpp \
       src/MoveGenerator.cpp src/Evaluator.cpp src/Search.cpp src/Utils.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
