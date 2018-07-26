#!/bin/sh

./daemon_server -conf ../conf/root_server.conf -exec ./node_server -logPath ../logs -logLevel 2 -port 9090 -workers 1
