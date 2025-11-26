CXX=g++
CXXFLAGS=-Wall -O2 -std=c++17

all: server client

server: server.o sha256.o
	$(CXX) $(CXXFLAGS) -o server server.o sha256.o

server.o: server.cpp sha256.hpp
	$(CXX) $(CXXFLAGS) -c server.cpp

sha256.o: sha256.cpp sha256.hpp
	$(CXX) $(CXXFLAGS) -c sha256.cpp

client: client.o sha256.o
	$(CXX) $(CXXFLAGS) -o client client.o sha256.o

client.o: client.cpp sha256.hpp
	$(CXX) $(CXXFLAGS) -c client.cpp

clean:
	rm -f *.o server client
