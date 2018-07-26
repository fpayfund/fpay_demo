Requirement

OS: Linux kernel version 2.6+ x86_64, g++ 4.8+
redis: recommend 3.0+

How to build
1) ./setup.sh # it will create "build" dir in the current diretory

How to run
1) run redis-server (require that Redis has been installed)
   $ redis-server --port 13679
2) $ cd build/FPDemo/bin
3) run node_server. you can modify the configuration (dir: ${project_home}/build/FPDemo/conf)
   run as stanalone server:
   $ ./node_server --id 9090 --port 9090 --conf ../conf/<root|miner>_server.conf
   run as master-worker:
   ./run_root.sh (as root node) or ./run_miner.sh (as miner node)

