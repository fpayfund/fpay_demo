include ../Makefile.inc

SRC_COMM=main.cpp
OBJ_COMM = $(SRC_COMM:.cpp=.o)
PROGRAM = daemon_server
CXXFLAGS += -lpthread -std=c++11
.PHONY:$(PROGRAM)

.SUFFIXES: .o .cpp
.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(PROGRAM): $(OBJ_COMM) $(COMM_LIB) 
	$(CXX) -o $@ $(OBJ_COMM) $(COMM_LIB) $(LDFLAGS)
clean:
	rm $(OBJ_COMM) $(PROGRAM)
