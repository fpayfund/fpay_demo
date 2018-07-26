#ifndef __TYPE_DEF_H__
#define __TYPE_DEF_H__

#include <stdint.h>
#include <assert.h>

#define INVALID_SOCK  -1

typedef uint64_t sid_t;
typedef uint32_t netType_t;
typedef uint32_t groupId_t;

typedef uint8_t uri_t;
typedef uint16_t resCode_t;
typedef uint32_t cid_t;
typedef uint16_t appId_t;
typedef uint32_t sessionId_t;
typedef uint32_t rkey_t;
typedef uint32_t uid_t;

enum RES_CODE_T {
    RES_OK  = 200,
    RES_MAX,
};

enum CODEC_TYPE_T {
    TCP_ENCODE=0,
    TCP_UNCODE=1,
    UDP_ENCODE=2,
    UDP_UNCODE=3
};

#endif
