CXX=g++
CXXFLAGS=-std=c++14 -ggdb -Wall -Wextra -pedantic -Werror -Wnon-virtual-dtor -Iinclude

SERVER_OBJS=MetadataStore.o
CLIENT_OBJS=SurfStoreProxy.o
CLIENT_LIBS=`xmlrpc-c-config c++ libwww-client --libs`
CLIENT_FLAGS=`xmlrpc-c-config c++ libwww-client --cflags`
SERVER_LIBS=`xmlrpc-c-config c++2 abyss-server server-util --libs`
SERVER_FLAGS=`xmlrpc-c-config c++2 abyss-server server-util --cflags`

default: client server

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

server: server.o $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) $(CLIENT_FLAGS) -o server server.o $(SERVER_OBJS) $(SERVER_LIBS)

client: client.o $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) $(SERVER_FLAGS) -o client client.o $(CLIENT_OBJS) $(CLIENT_LIBS)

clean:
	rm -f client server *.o
