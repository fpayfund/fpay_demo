#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <thread>
#include "FPProto.h"

static const char account_file_path[] = "accounts.txt";
static const std::string init_address = "0000000000000000000000000000000000000000000000000000000000000000";
// request format:
// [----4-bytes-sesssionId----][----4-bytes-length----][----1-byte-cmd----][json-object]

typedef enum {
    COMMAND_NONE = 0,
    COMMAND_QUERY = 1,
    COMMAND_CONNECT = 2,
    COMMAND_CONFIRMATION = 3,
    COMMAND_PAY = 4,
    COMMAND_BROADCAST = 5,
} Command;

static void netItoa(int paraIn, char paraOut[])
{
    uint32_t *num = (uint32_t*)paraOut;
    *num = htonl(paraIn);
}

static int netAtoi(const char paraIn[])
{
    uint32_t *num = (uint32_t*)paraIn;
    return ntohl(*num);
}

static uint32_t JShash(const std::string& key)
{
    uint32_t hash = 1315423911;
    if (!key.empty()) {
        const char* start = key.c_str();
        const char* end = start + key.size();
        for(const char* key = start + 7; key < end; key++) {
            hash ^= ((hash << 5) + (*key) + (hash >> 2));
        }
    }
    return (hash & 0x7FFFFFFF);
}

void usage(char** argv)
{
    std::cout << argv[0] << "\r\n\t--host:\tserver host"
              << "\r\n\t--port:\tserver port"
              << "\r\n\t--cmd:\t<init> or <tx> or <query>"
              << "\r\n\t--addr:\tuser address for query"
              << "\r\n\t--reqs:\tnumber of requests to be send"
              << "\r\n\t--conns:\tnumber of concurrent connections"
              << std::endl;
}

bool buildQueryRequest(const std::string& address, std::string& reqData)
{
    QueryBalance query;
    query._address = address;
    std::string json;
    if (!query.toString(json)) {
        return false;
    }

    char cmd = COMMAND_QUERY;
    uint32_t userHash = JShash(address);
    char buf[4];
    netItoa(userHash, buf);
    reqData.assign(buf, 4);
    netItoa(json.size() + 1, buf);
    std::string size;
    size.assign(buf, 4);
    reqData += size;
    reqData += (unsigned char)cmd;
    reqData += json;

    return true;
}

bool buildTxRequest(const std::string& from, const std::string& to, uint32_t value, std::string& reqData)
{
    int r = rand();
    char buf[16];
    sprintf(buf, "%d", r);
    Transaction tx;
    tx._id = buf;
    tx._from = from;
    tx._to = to;
    tx._value = value;
    tx._timestamp = clock_t();

    std::string json;
    if (!tx.toString(json)) {
        return false;
    }
    char cmd = COMMAND_PAY;
    uint32_t userHash = JShash(from);
    netItoa(userHash, buf);
    reqData.assign(buf, 4);
    netItoa(json.size() + 1, buf);
    std::string size;
    size.assign(buf, 4);
    reqData += size;
    reqData += (unsigned char)cmd;
    reqData += json;

    return true;
}

int request(const char* host, int port, const std::string& reqData)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        std::cout << "create socket failed" << std::endl;
        return -1;
    }
  
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if(inet_aton(host, &server_addr.sin_addr) == 0) {
        std::cout << "ip address error" << std::endl;
        return -1;
    }
  
    server_addr.sin_port = htons(port);
    socklen_t server_addr_length = sizeof(server_addr);

    if (connect(fd, (struct sockaddr*)&server_addr, server_addr_length) < 0) {
        std::cout << "connect fails" << std::endl;
        return 2;
    }

    int on = 1;
    if (ioctl(fd, FIONBIO, &on) < 0) {
        std::cout << "set NBIO fails" << std::endl;
        return 1;
    }
    
    const char *reqBuf = reqData.c_str();
    size_t reqSize = reqData.size();
    //std::cout << "req data: " << reqData << std::endl;
    //std::cout << "sending request, size=" << reqSize << std::endl;

    fd_set rfds;
    fd_set wfds;
    struct timeval tv;
    int retval;

    FD_ZERO(&wfds);
    FD_SET(fd, &wfds);

    tv.tv_sec = 3;
    tv.tv_usec = 0;

    retval = select(fd+1, NULL, &wfds, NULL, &tv);
    if (retval <= 0 || !FD_ISSET(fd, &wfds)) {
        std::cout << "failed to send request, retval=" << retval << std::endl;
        return -1;
    }

    ssize_t sendCount = write(fd, reqBuf, reqSize);
    if (sendCount < 0) {
        std::cout << "failed to send request" << std::endl;
        return -1;
    }

    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    retval = select(fd+1, &rfds, NULL, NULL, &tv);
    if (retval <= 0 || !FD_ISSET(fd, &rfds)) {
        std::cout << "failed to read response, retval=" << retval << std::endl;
        return -1;
    }

    char* buf = (char*)calloc(1024, 1);
    ssize_t count = read(fd, buf, 1024);       
    if (count == 0) {
        std::cout << "server close connection" << std::endl;
        return -1;
    }

    close(fd);

    //std::string rsp(buf, count);
    //std::cout << "response: " << rsp << std::endl;
    
    return 0;
}


void initAccountBalance(const char* host, int port, uint32_t value)
{
    std::ifstream fin(account_file_path);
    std::string s;  
    std::string reqData;
    while (fin >> s) {    
        std::cout << "Read from file: " << s << std::endl;  
        if (buildTxRequest(init_address, s, value, reqData)) {
            request(host, port, reqData);
        }
    }
}

int testRandomTx(const char* host, int port, int reqs)
{
    std::vector<std::string> addressList;
    std::ifstream fin(account_file_path);
    std::string s;  
    while (fin >> s) {    
        addressList.push_back(s);
    }
    
    int reqCnt = 0;
    int count = addressList.size();
    for (int i = 0; i < reqs; i++) {
        int fromAccount = rand() % count;
        int toAccount = rand() % count;

        std::string reqData;
        if (buildTxRequest(addressList[fromAccount], addressList[toAccount], 100, reqData)) {
            request(host, port, reqData);
            ++reqCnt;
        }
    }

    //std::cout << reqCnt << " requests finished" << std::endl;
    return reqCnt;
}

void testQuery(const char* host, int port, const char* address)
{
    std::string reqData;
    if (buildQueryRequest(address, reqData)) {
        request(host, port, reqData);
    }
}

int main(int argc, char *argv[])
{
    const char* address = NULL;
    const char* host = NULL;
    int port = -1;
    int reqs = 1;
    int conns = 1;
    std::string cmd;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--host") == 0) {
            host = argv[++i];
        } else if (strcmp(argv[i], "--port") == 0) {
            sscanf(argv[++i], "%d", &port);
        } else if (strcmp(argv[i], "--reqs") == 0) {
            sscanf(argv[++i], "%d", &reqs);
        } else if (strcmp(argv[i], "--conns") == 0) {
            sscanf(argv[++i], "%d", &conns);
        } else if (strcmp(argv[i], "--addr") == 0) {
            address = argv[++i];
        } else if (strcmp(argv[i], "--cmd") == 0) {
            cmd = argv[++i];
        } else {
            usage(argv);
            return 0;
        }
    }

    if (!host || port <= 0 || reqs <= 0) {
        std::cout << "invalid arguments" << std::endl;
        return -1;
    }
  
    struct timeval t1,t2;
    double timeuse;
    gettimeofday(&t1, NULL);

    if (cmd == "init") {
        initAccountBalance(host, port, 100);
        return 0;
    } 
    if (cmd == "query") {
        testQuery(host, port, address);
        return 0;
    } 
    if (cmd == "tx") {
        if (conns > 0) {
            int thread_reqs = reqs / conns;
            std::vector<std::thread*> threadPool;
            for (int i = 0; i < conns; i++) {
                std::thread* t = new std::thread(testRandomTx, host, port, thread_reqs);
                threadPool.push_back(t);
            }

            for (int i = 0; i < threadPool.size(); i++) {
                threadPool[i]->join();
            }
        }
    } 

    gettimeofday(&t2, NULL);
    timeuse = t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec)/1000000.0;
    std::cout << "Req: " << reqs << ", Use Time: " << timeuse
              << ", TPS: " << reqs / timeuse
              << std::endl;

    return 0;
}

//g++ -Werror -std=c++11 -o test -lpthread -I. test.cpp
