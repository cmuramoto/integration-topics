FROM cmuramoto/ubuntu-nats-proto:1.0

COPY include/ /include 
COPY resources/ /resources
COPY closures/ /closures
COPY server.proto.h .
COPY server.proto.c .

RUN cp resources/test_zmq.proto .  && protoc-c --c_out=. test_zmq.proto && cp test_zmq.pb-c.h resources

RUN gcc -c -Wall -Werror -fpic test_zmq.pb-c.c
RUN gcc -shared -o libtest_zmq.pb-c.so test_zmq.pb-c.o
RUN gcc -L. -L/usr/local/lib/ -L/usr/lib/x86_64-linux-gnu server.proto.c  -ltest_zmq.pb-c -lnats -lprotobuf-c -lpthread

ENV NATS_SERVERS "localhost:4222" 

CMD export LD_LIBRARY_PATH=/usr/local/lib/:/ && /a.out $NATS_SERVERS
