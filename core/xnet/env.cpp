#include <iostream>
#include "IOLoop.h"
#include "SignalHandler.h"
#include "env.h"

int env::SocketErrorLogLevel = 0;
time_t env::now = time(NULL);
std::string env::strTime = "";
unsigned int  env::msec = 0;
uint64_t env::usec = 0;

IOLoop* env::_loop = NULL;
SignalHandler* env::_signalHandler = NULL;
