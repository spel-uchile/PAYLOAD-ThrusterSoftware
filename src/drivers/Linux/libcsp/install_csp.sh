#!/usr/bin/env bash
echo "Downloading libcsp v1.4"
[ ! -e libcsp ] && git clone https://github.com/libcsp/libcsp 

cd libcsp
git pull origin master

[ -e ../wscript ] && cp -rf ../wscript ./

echo "Build libcsp"
./waf configure --with-os=posix --enable-if-zmqhub --install-csp --prefix=../ build install

cd -