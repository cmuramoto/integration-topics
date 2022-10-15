#!/bin/bash

function cleanup {
    echo "Cleanup"
    rm -f test_zmq.* *.a
}

trap cleanup EXIT

function compile_proto {
    echo "Creating Protocol Buffers Sources"
    cp resources/test_zmq.proto .
    protoc --c_out=. test_zmq.proto
    cp test_zmq.pb-c.h resources
}

function make_proto_lib {
    echo "Compiling Protocol Buffers and creating Shared Library(static)"
    gcc -c -Wall -Werror test_zmq.pb-c.c
    ar -rc libtest_zmq.pb-c_static.a  test_zmq.pb-c.o
}

# To link libpthread statically we need to link /usr/lib/x86_64-linux-gnu/libc.a as well.
# There are some symbol confusion to take care of to achieve a fully static image to place in
# a scratch container
function compile_and_link_server {
    echo "Compiling And Linking Server(semi-static)"
    gcc -O3 -L/usr/local/lib/ server.proto.c  \
    libtest_zmq.pb-c_static.a \
    /usr/local/lib/libnats_static.a \
    /usr/lib/x86_64-linux-gnu/libprotobuf-c.a \
    /usr/lib/x86_64-linux-gnu/libpthread.so \
    -o server-static.bin
}

compile_proto
make_proto_lib
compile_and_link_server
