FROM ubuntu:22.04

RUN apt update \
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
