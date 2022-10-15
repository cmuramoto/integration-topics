FROM cmuramoto/alpine-dev-tools:1.0

RUN apk update \
&& apk add --no-cache \
protobuf-c-compiler protobuf-c-dev cmake \
&& git clone https://github.com/nats-io/nats.c.git \
&& cd nats.c \
&& mkdir build \
&& cd build \
&& cmake .. -DNATS_BUILD_STREAMING=OFF -DNATS_BUILD_WITH_TLS=OFF \
&& make install \
&& cd ../../ \
&& rm -rf nats.c

