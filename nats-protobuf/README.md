# Protocol Buffers + Nats Demo

This setup demonstrates how to create a C server listener and a Java client to excersise Nats request-reply semantics by passing messages serialized with Protocol Buffers.

## C setup

In order to compile C code there are some tedious pre-steps to fetch and compile the libraries, which are described in [image-recipes](https://github.com/cmuramoto/integration-topics/tree/main/nats-protobuf/base-images-recipes)

For exemple, for ubuntu you have to install protobuf libs and compile nats c client:

```
apt update \
&& apt install -y g++ \
&& apt install -y protobuf-compiler \
&& apt install protobuf-c-compiler \
&& apt install -y cmake \
&& apt install -y libprotoc-dev \
&& apt install -y libprotobuf-c-dev \
&& apt install -y openssl \
&& apt install -y git \
&& git clone https://github.com/nats-io/nats.c.git \
&& cd nats.c \
&& mkdir build \
&& cd build \
&& cmake .. -DNATS_BUILD_STREAMING=OFF -DNATS_BUILD_WITH_TLS=OFF \
&& make install \
&& cd ../../ \
&& rm -rf nats.c
```

Once that's done, headers will be available at */usr/local/include/nats* and shared libraries at /usr/local/lib.

In the **docker** dir there are some recipes to create the image for the executable. 

The static recipe produces a almost 100% statically linked binary, pthreads being the exception. 
This artifact is suitable for execution on a minimal alpine image, which weighs just 6.1MB.

If pthread dependency is removed we can set up a fully 100% statically linked binary that could be hoisted on a scratch image.

## Java Setup

Only maven is required. 

Tests will run using test-containers, which requires the docker server to be pre-built first.





