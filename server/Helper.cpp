#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>
#include <sstream>
#include "Helper.h"

namespace Helper
{

void GetRandStr(char* str, const int len)
{
    srand(time(NULL));
    int i = 0;
    for (; i < len - 1; i++) {
        int x = rand();
        int y = x % 3;
        if (y == 0) {
            str[i] = 'A' + x % 26;
        } else if (y == 1) {
            str[i] = 'a' + x % 26;
        } else {
            str[i] = '0' + x % 10;
        }
    }
    str[i] ='\0';
}

int NetAtoi(const char paraIn[])
{
    int paraOut = 0;
    for(int i = 0; i < 4; ++i) {
        paraOut += (uint8_t)(paraIn[i]) << (24 - 8*i);
    }
    return paraOut;
}

void NetItoa(int paraIn, char paraOut[])
{
    for(int i = 0; i < 4; ++i) {
        paraOut[i] = ((paraIn >> (24 - 8*i))&0xff);
    }
}

} // Helper
