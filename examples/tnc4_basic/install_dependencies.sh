#!/bin/sh

# install compiler toolchain and dependencies
apt-get install -y make cmake ninja-build gcc-arm-none-eabi

# install boost
apt-get install -y libboost-all-dev

# install blaze
cd /opt
git clone https://bitbucket.org/blaze-lib/blaze.git
cp -r /opt/blaze/blaze /usr/include/