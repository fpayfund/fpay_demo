include ../Makefile.inc
.PHONY: all clean

SRC_COMM =  \
			RedisCache.cpp  \
			RedisClient.cpp

# -------------------------------------------------------------------------
OBJ_COMM  = $(SRC_COMM:.cpp=.o)
OBJ_COMM_R  = $(SRC_COMM:.cpp=.ro)
INSTALL_PATH=.
STATIC_LIB=libcache.a
CXXFLAGS += -Wall -ggdb -std=c++11

.SUFFIXES: .o .cpp
.cpp.o:
	$(CXX) $(CXXFLAGS) ${INCLUDE} -c -o $@ $<


all: $(STATIC_LIB)

$(STATIC_LIB): $(OBJ_COMM) | $(INSTALL_PATH)
	$(ARRU) $(INSTALL_PATH)/$(STATIC_LIB) $(OBJ_COMM)
	ranlib $(INSTALL_PATH)/$(STATIC_LIB)

$(INSTALL_PATH):
	mkdir -p $(INSTALL_PATH)
  
depend:
	mkdep $(CXXFLAG) $(SRC_COMM)

install:

clean: 
	rm -f *.o
	rm -f *.ro
	rm -f $(INSTALL_PATH)/$(STATIC_LIB)

