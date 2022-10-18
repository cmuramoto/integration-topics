#pragma once

#include "resources/test_zmq.pb-c.h"
#include "include/nats.util.h"
#include "closures/echo_template.h"
#include "closures/echo_zmq.h"

void onZmq(natsConnection *nc, natsSubscription *sub, natsMsg *msg)
{
  echo_template(nc, sub, msg, &unpack_zmq, &pack_zmq);
}