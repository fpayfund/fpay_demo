#o!/bin/sh

project_home=$(pwd)
depend_path=$project_home/3parts
install_path=$project_home/build/FPDemo

mkdir -p $install_path

cd $depend_path
rm -rf $depend_path/include
rm -rf $depend_path/lib
mkdir -p $depend_path/include
mkdir -p $depend_path/lib

rm -rf tinyxml
tar xzvf tinyxml.tar.gz
cd tinyxml
make && make install
cp -r ./include/* $depend_path/include
cp -r ./lib/* $depend_path/lib

cd $depend_path
rm -rf hiredis
tar xzvf hiredis.tar.gz
cd hiredis
make && make install PREFIX=$depend_path

cd $depend_path
rm -rf cereal
tar xzvf cereal.tar.gz
mv cereal $depend_path/include

cd $depend_path
rm -rf  gflags-2.2.0
tar xzvf gflags-2.2.0.tar.gz
cd gflags-2.2.0
rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=$depend_path
make && make install

cd $depend_path
rm -rf glog
tar xvf glog.tar.gz
cd glog
sh ./autogen.sh
./configure --prefix=$depend_path
make && make install

cd $depend_path
rm -rf $depend_path/bin
rm -rf $depend_path/share

rm -r $install_path/bin
rm -r $install_path/conf
rm -r $install_path/logs
mkdir -p $install_path/bin
mkdir -p $install_path/logs

echo "compile core modules"
cd $project_home/core
make
cd $project_home

echo "compile cache module"
cd $project_home/cache
make
cd $project_home

echo "compile daemon server"
cd $project_home/daemon
make
cp daemon_server $install_path/bin
cp *.sh $install_path/bin

echo "compile node server"
cd $project_home/server
make
cp node_server $install_path/bin
cp -r $project_home/conf $install_path

echo "compile test"
cd $project_home/test
make

echo "done."
