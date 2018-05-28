#!/bin/bash

sudo apt-get update

sudo apt-get install -f -y git autoconf libtool build-essential pkg-config libevent-dev libssl-dev libtbb-dev libboost-all-dev libpcap-dev cmake doxygen

read -p "Press enter to install libfluid_msg"

# libfluid_msg
echo " \n\n "
echo "----------------"
echo "LIBFLUID MSG "
git clone https://github.com/OpenNetworkingFoundation/libfluid_msg.git
cd libfluid_msg
sudo ./autogen.sh
sudo ./configure --prefix=/usr
sudo make
sudo make install

cd ..

read -p "Press enter to install libfluid_base"

# libfluid_base
echo " \n\n "
echo "----------------"
echo "LIBFLUID base "
git clone https://github.com/OpenNetworkingFoundation/libfluid_base.git
cd libfluid_base
sudo ./autogen.sh
sudo ./autogen.sh
sudo ./configure --prefix=/usr
sudo make
sudo make install

cd ..

read -p "Press enter to install tclap"

echo "\n\n"
echo "---------------------"
echo "TCLAP"
#tclap
git clone https://github.com/eile/tclap.git
cd tclap
sudo ./autotools.sh
sudo ./configure --prefix=/usr
sudo make
sudo make install

cd ..

read -p "Press enter to install libcrafter"

echo "\n\n"
echo "---------------------"
echo "LIBCRAFTER"
#libcrafter
git clone https://github.com/pellegre/libcrafter
cd libcrafter/libcrafter
sudo ./autogen.sh
sudo ./autogen.sh
sudo ./configure --prefix=/usr
sudo make
sudo make install

cd ..
cd ..

read -p "Press enter to install nanomsg"

echo "\n\n"
echo "---------------------"
echo "NANOMSG"
#nanomsg
git clone https://github.com/nanomsg/nanomsg.git
cd nanomsg
sudo ./configure --prefix=/usr
sudo make
sudo make install

cd ..

read -p "Press enter to compile perfbench"

echo "\n\n"
echo "---------------------"
echo "COMPILE"
#Compile
cmake .
cmake --build . --target perfbench -- -j 4






