#!/bin/sh

./daemon_server -conf ../conf/miner_server.conf -exec ./node_server -logPath ../logs -logLevel 2 -port 9080 -workers 1
