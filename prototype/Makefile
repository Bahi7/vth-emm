# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -std=c++17

# Include directories
INCLUDES = -IC:\Users\Enuma\Downloads\cereal-master\cereal-master\include

# Linker flags for Windows Sockets
LDFLAGS = -lws2_32

# Executables
TARGETS = server client

# Default rule
all: $(TARGETS)

# Object files
OBJS = LDX.o PathOram.o

Lat.o: Lat.cpp Lat.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c Lat.cpp

LDX.o: LDX.cpp LDX.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c LDX.cpp

PathOram.o: PathOram.cpp PathOram.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c PathOram.cpp

server: server.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o server server.cpp $(OBJS) $(LDFLAGS)

client: client.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o client client.cpp $(OBJS) $(LDFLAGS)

clean:
	del *.o server.exe client.exe

.PHONY: all clean

