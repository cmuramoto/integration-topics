#pragma once

#include "../resources/test_zmq.pb-c.h"

void *unpack_zmq(const uint8_t *data, int len)
{
    NatsModel__MsgSyncPlataoStvdT *unpacked = nats_model__msg_sync_platao_stvd_t__unpack(NULL, len, data);

    return unpacked;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
size_t pack_zmq(void *unpacked, uint8_t *dst)
{
    NatsModel__MsgSyncPlataoStvdT *u = (NatsModel__MsgSyncPlataoStvdRespT *)unpacked;

    size_t packed_len = nats_model__msg_sync_platao_stvd_t__pack(u, dst);

    nats_model__msg_sync_platao_stvd_t__free_unpacked(u, NULL);

    return packed_len;
}
#pragma GCC diagnostic pop