CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -MMD -MP

# Source files
SRCS = main.cpp lexer.cpp parser.cpp interpreter.cpp
# Object files
OBJS = $(SRCS:.cpp=.o)
# Target executable
TARGET = kumu

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)

clean:
	rm -f $(OBJS) $(OBJS:.o=.d) $(TARGET)
