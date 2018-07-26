include ../Makefile.inc

NODE_SERVER_SRC = Config.cpp Helper.cpp FPService.cpp FPReqHandler.cpp FPNodeServer.cpp 
NODE_SERVER_OBJ = $(NODE_SERVER_SRC:.cpp=.o)
NODE_SERVER = node_server
CXXFLAGS += -lpthread -std=c++11
.PHONY: all $(NODE_SERVER)

.SUFFIXES: .o .cpp
.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all:$(NODE_SERVER)

$(NODE_SERVER): $(NODE_SERVER_OBJ) $(COMM_LIB) 
	$(CXX) -o $@ $(NODE_SERVER_OBJ) $(COMM_LIB) $(LDFLAGS)

clean:
	rm *.o 
	rm $(NODE_SERVER)
