include ../Makefile.inc

CXXFLAGS += -I../server -I. -lpthread -std=c++11
.PHONY: all clean
all:
	$(CXX) -o test $(COMM_LIB) $(LDFLAGS) $(CXXFLAGS) -DJSON_NEST test.cpp
	$(CXX) -o initSuperWallet initSuperWallet.cpp $(COMM_LIB) $(LDFLAGS) $(CXXFLAGS) -DJSON_NEST
clean:
	rm test
	rm initSuperWallet
