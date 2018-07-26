CC32 = gcc

ARRU = ar -ru

DEFINE_REACTOR=-DHAVE_EPOLL

DEF_PROTO=-D_JSON_PROTOCOL

LDFLAGS = -lpthread -lrt -lssl
CFLAGS =  -g -O3
CXXFLAGS = -DTIXML_USE_STL -fpermissive -g \
        -I../3parts/include \
        -I../3parts \
        -I../cache \
        -I../core/base \
        -I../core/conn \
        -I../core/proto \
        -I../core/writer \
        -I../core/dispatcher \
        -I../core/xnet 

COMM_LIB = \
       ../cache/libcache.a \
       ../core/lib/dispatcher.a \
       ../core/lib/writer.a \
       ../core/lib/conn.a \
       ../core/lib/proto.a \
       ../core/lib/base.a \
       ../core/lib/xnet.a \
       ../3parts/lib/libgflags.a \
       ../3parts/lib/libglog.a \
       ../3parts/lib/libhiredis.a \
       ../3parts/lib/libtinyxml.a
