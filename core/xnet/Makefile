include ../Makefile.inc

SRC_COMM = \
		   IOLoop.cpp \
		   Exception.cpp \
		   TimerList.cpp \
		   EventHandler.cpp \
		   SignalHandler.cpp \
		   Socket.cpp  \
		   TcpSock.cpp  \
		   TcpAcceptor.cpp  \
		   WhiteLst.cpp  \
		   env.cpp

# -------------------------------------------------------------------------
OBJ_COMM  = $(SRC_COMM:.cpp=.o)
OBJ_COMM_R  = $(SRC_COMM:.cpp=.ro)
INSTALL_PATH=../lib/
STATIC_LIB=xnet.a

.SUFFIXES: .o .cpp
.cpp.o:
	$(CXX) $(CXXFLAG) ${INCLUDE} -c -o $@ $<


.SUFFIXES: .ro .cpp
.cpp.ro:
	$(CXX32) $(CXXFLAG_R) ${INCLUDE} -c -o $@ $<

.PHONY: all
all: $(STATIC_LIB)

$(STATIC_LIB): $(OBJ_COMM) | $(INSTALL_PATH)
	$(ARRU) $(INSTALL_PATH)/$(STATIC_LIB) $(OBJ_COMM)
	ranlib $(INSTALL_PATH)/$(STATIC_LIB)

$(INSTALL_PATH):
	mkdir -p $(INSTALL_PATH)
  
test: $(STATIC_LIB) test.cpp
	$(CXX) $(CXXFLAG) -o test test.cpp $(STATIC_LIB)

depend:
	mkdep $(CXXFLAG) $(SRC_COMM)

install:

clean: 
	rm -f *.o
	rm -f *.ro
	rm -f $(INSTALL_PATH)/$(STATIC_LIB)

