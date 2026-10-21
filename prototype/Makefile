# MinGW build of the 2023 prototype (Windows sockets + cereal).
# cereal is header-only: point CEREAL_INC at its include directory.
CXX = g++
CXXFLAGS = -Wall -std=c++17
CEREAL_INC ?= ../third_party/cereal/include
INCLUDES = -I$(CEREAL_INC)
LDFLAGS = -lws2_32

TARGETS = server client

all: $(TARGETS)

# LDX.cpp, Lat.cpp and PathORAM.cpp are included directly by the two programs,
# so each program is a single translation unit.
server: server.cpp LDX.cpp PathORAM.cpp utils.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o server server.cpp $(LDFLAGS)

client: client.cpp Lat.cpp LDX.cpp utils.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o client client.cpp $(LDFLAGS)

clean:
	del /Q server.exe client.exe 2>NUL

.PHONY: all clean
