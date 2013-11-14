# Introduction

libtnet is a tiny high performance network lib, purpose is to simplify the network programming.

# Install

go to root source, then

    mkdir -p build
    cd build
    cmake ..
    make
    make install

# Requirement

- gcc >= 4.4, supports c++ 0x
- linux, supports eventfd, signalfd and timerfd 

