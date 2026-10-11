CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -MMD -MP

# Source files
SRCS = main.cpp lexer.cpp parser.cpp interpreter.cpp
# Object files
OBJS = $(SRCS:.cpp=.o)
# Target executable
TARGET = kumu

# Windows cross-build (MinGW-w64, posix threads for std::this_thread).
# Statically linked so kumu.exe runs on a stock Windows machine with no
# MinGW runtime DLLs alongside it.
WIN_CXX = x86_64-w64-mingw32-g++-posix
WIN_TARGET = kumu.exe
WIN_LDFLAGS = -static -static-libgcc -static-libstdc++ -s

all: $(TARGET)

windows: $(WIN_TARGET)

$(WIN_TARGET): $(SRCS) $(wildcard *.hpp)
	$(WIN_CXX) -std=c++17 -Wall -Wextra -O2 -o $(WIN_TARGET) $(SRCS) $(WIN_LDFLAGS)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)

.PHONY: all windows clean

clean:
	rm -f $(OBJS) $(OBJS:.o=.d) $(TARGET) $(WIN_TARGET)
