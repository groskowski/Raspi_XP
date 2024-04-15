# Define the compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -g

# Linker flags
LDFLAGS = -lpigpio -pthread -lrt

# Define the output program name
TARGET = TestReel

# Define source files
SRC = main.cpp Reel.cpp

# Define object files
OBJ = main.o Reel.o

# Rule for making the final target
$(TARGET): $(OBJ)
	$(CXX) -o $(TARGET) $(OBJ) $(CXXFLAGS) $(LDFLAGS)

# Rule for making the object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<
	
Reel.o: Reel.cpp
	$(CC) $(CFLAGS) -c $(INCLUDE) $<

# Clean the build directory
clean:
	rm -f $(TARGET) $(OBJ)
