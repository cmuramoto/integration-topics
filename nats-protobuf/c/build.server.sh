#!/bin/bash

function cleanup {
    echo "Cleanup"
    rm -f test_zmq.*
}

trap cleanup EXIT

function compile_proto {
    echo "Creating Protocol Buffers Sources"
    cp resources/test_zmq.proto .
    protoc --c_out=. test_zmq.proto
    cp test_zmq.pb-c.h resources
}

function make_proto_lib {
    echo "Compiling Protocol Buffers and creating Shared Library(dynamic)"
    gcc -c -Wall -Werror -fpic test_zmq.pb-c.c
    gcc -shared -o libtest_zmq.pb-c.so test_zmq.pb-c.o
}

function compile_and_link_server {
    echo "Compiling And Linking Server(dynamic)"
    gcc -O3 -L. -L/usr/local/lib/ -L/usr/lib/x86_64-linux-gnu server.proto.c  -ltest_zmq.pb-c -lnats -lprotobuf-c -lpthread -o server.bin
}

compile_proto
make_proto_lib
compile_and_link_server

#export LD_LIBRARY_PATH=/usr/local/lib/:$PWD
