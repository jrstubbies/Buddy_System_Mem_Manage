CC := g++
TARGET := main

# Detect the operating system
ifeq ($(OS),Windows_NT)
	# Windows
	CFLAGS := -O2 -std=c++14 -Wall -c
    LFLAGS := -lkernel32 -lpsapi
#     LFLAGS :=
    EXTENSION := .exe
	CLEANUP := del
	CLEANUP_OBJS := del *.o
# 	CLEANUP_OBJS := del *.exe
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		# macOS
		EXTENSION := .out
		CFLAGS := -O2 -std=c++14 -Wall -c
		LFLAGS := 
		CLEANUP := rm -f
		CLEANUP_OBJS := rm -f *.o
	else ifeq ($(UNAME_S),Linux)
		# Linux
		EXTENSION := .out
		CFLAGS := -O2 -std=c++14 -Wall -c   
		LFLAGS := 
		CLEANUP := rm -f
		CLEANUP_OBJS := rm -f *.o
	endif
endif

# Find all source files (.cpp) and header files (.h)
SRCS := $(wildcard *.cpp) $(wildcard */*.cpp)
HDRS := $(wildcard *.h) $(wildcard */*.h)

# Create object file names based on source file names
OBJS := $(SRCS:.cpp=.o)

# Output executable
# EXECUTABLE := main.exe

# Rule to build the executable
$(TARGET)$(EXTENSION): $(OBJS)
	$(CC) -O2 -Wl,-s -o $@ $(OBJS) $(LFLAGS)

# Rule to build object files
%.o: %.cpp $(HDRS)
	$(CC) -O2 -std=c++11 -c $< -o $@

clean:
	$(CLEANUP) $(TARGET)$(EXTENSION)
	$(CLEANUP_OBJS)
