DEFINE_REACTOR=-DHAVE_EPOLL
CC = gcc 
CXX32 = g++ 
CC32 = gcc 

ARRU = ar -ru 

INCLUDE = -I./ \
          -I../xnet \
          -I../base \
          -I../conn \
          -I../dispatcher \
          -I../proto \
          -I../../3parts/include

gccVer = $(shell gcc -dumpversion | cut -f1 -d'.')

ifeq ($(gccVer), 4)
    CXXFLAG = -Wall -ggdb $(DEFINE_REACTOR)
    CXXFLAG_R = -Wall -O2 $(DEFINE_REACTOR)
else
    CXXFLAG = -Wall -ggdb $(DEFINE_REACTOR) -std=c++11
    CXXFLAG_R = -Wall -O2 $(DEFINE_REACTOR) -std=c++11
endif
