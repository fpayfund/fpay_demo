include ../Makefile.inc
.PHONY: all clean

SRC_COMM =  \
			TcpServerMaster.cpp \
			MasterConn.cpp \
		   	Worker.cpp

# -------------------------------------------------------------------------
OBJ_COMM  = $(SRC_COMM:.cpp=.o)
OBJ_COMM_R  = $(SRC_COMM:.cpp=.ro)
INSTALL_PATH=../lib/
STATIC_LIB=dispatcher.a

.SUFFIXES: .o .cpp
.cpp.o:
	$(CXX) $(CXXFLAG) ${INCLUDE} -c -o $@ $<


.SUFFIXES: .ro .cpp
.cpp.ro:
	$(CXX32) $(CXXFLAG_R) ${INCLUDE} -c -o $@ $<

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

