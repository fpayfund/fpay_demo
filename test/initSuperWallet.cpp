#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include "Helper.h"
#include "RedisCache.h"
#include "FPProto.h"

using namespace std;

static const std::string init_address =  "0000000000000000000000000000000000000000000000000000000000000000";
static const std::string init_block_id = "block00000000000000000000000000000000000000000000000000000000000";
static const std::string init_tx_id =    "tx00000000000000000000000000000000000000000000000000000000000000";

void usage(char** argv)
{
    std::cout << argv[0] << "set value to the super wallet"
              << "\r\n\t--dbConfig:\tdb configuration file path"
              //<< "\r\n\t--init_block:\tID of init block"
              //<< "\r\n\t--init_address:\taddress of the super wallet"
              << "\r\n\t--initValue:\tvalue that set to the super wallet"
              << std::endl;
}

int main(int argc, char *argv[])
{
    const char* conf = NULL;
    //const char* init_block_id = NULL;
    //const char* init_address = NULL;
    int64_t init_value = 10000000000;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--dbConfig") == 0) {
            conf = argv[++i];
        //} else if (strcmp(argv[i], "--init_block") == 0) {
        //    init_block_id = argv[++i];
        //} else if (strcmp(argv[i], "--init_address") == 0) {
        //    init_address = argv[++i];
        } else if (strcmp(argv[i], "--initValue") == 0) {
            sscanf(argv[++i], "%lu", &init_value);
        } else {
            usage(argv);
            return 0;
        }
    }

    if (!conf) {
        cout << "ERROR: configuration: " << conf << endl;
        return -1;
    }

    RedisCache* dbService = RedisCache::create(conf);
    if (!dbService) {
        cout << "redis init fails: conf=" << conf << endl;
        return -1;
    }

    Transaction tx;
    tx._id = init_tx_id;
    tx._to = init_address;
    tx._value = init_value;

    Block block;
    block._id = init_block_id;
    block._txList.push_back(tx);
    string json;
    if (!block.toString(json)) {
        cout << "ERROR: block serialization fails" << endl;
        return -1;
    }
    if (!dbService->Set(block._id, json, uint32_t(-1))) {
        cout << "ERROR: block store fails" << endl;
        return -1;
    }

    return 0;
}
