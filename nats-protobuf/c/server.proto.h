#ifndef INCLUDE_NATS_UTIL
#define INCLUDE_NATS_UTIL 1
#include "include/nats.util.h"
#endif

#ifndef INCLUDE_TEST_ZMQ_PB
#define INCLUDE_TEST_ZMQ_PB 1
#include "resources/test_zmq.pb-c.h"
#endif

#include "closures/echo_template.h"
#include "closures/echo_zmq.h"

void onZmq(natsConnection *nc, natsSubscription *sub, natsMsg *msg)
{
  echo_template(nc, sub, msg, &unpack_zmq, &pack_zmq);
}