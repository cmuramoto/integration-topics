#ifndef INCLUDE_TEST_ZMQ_PB
#define INCLUDE_TEST_ZMQ_PB 1
#include "../resources/test_zmq.pb-c.h"
#endif

#ifndef INCLUDE_DEFS
#define INCLUDE_DEFS 1
#include "../include/defs.h"
#endif

#define USE_GROWABLE_ARRAY 0

void echo_template(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *unpack(const uint8_t *, int), size_t pack(void *, uint8_t *))
{
    thread_id tid = (thread_id)pthread_self();
#if USE_GROWABLE_ARRAY
    static __thread GrowableArray *array = NULL;

    if (!array)
    {
        array = allocate_growable_array(256);
    }
    assert(array->owner == tid);
#else
    uint8_t out[4096];
#endif

    const char *subject = natsMsg_GetSubject(msg);

    const char *reply = natsMsg_GetReply(msg);

    const uint8_t *data = (uint8_t *)natsMsg_GetData(msg);

    int len = natsMsg_GetDataLength(msg);

    if (server_config->log_receive)
    {
        printf("[%lu]Received msg: %s - %d(bytes)\n",
               tid,
               subject,
               len);
    }

#if USE_GROWABLE_ARRAY
    require(array, len + 256);
#endif

    void *unpacked = unpack(data, len);

    size_t packed_len;

#if USE_GROWABLE_ARRAY
    packed_len = pack(unpacked, array->buffer);
#else
    packed_len = pack(unpacked, out);
#endif

    if (server_config->log_serialize)
    {
        printf("[%lu]Serialized packet to %d bytes\n", tid, (int)packed_len);
    }

    natsMsg *reply_msg;
    char *reply_buffer;

#if USE_GROWABLE_ARRAY
    reply_buffer = (char *)array->buffer;
#else
    reply_buffer = out;
#endif
    natsStatus create_status = natsMsg_Create(&reply_msg, reply, subject, reply_buffer, packed_len);

    /* We can only destroy the message after using info extracted with natsMsg_GetXXX,
     * otherwise we might end up with stale memory that was reclaimed somewhere else.
     */
    natsMsg_Destroy(msg);

    if (create_status != NATS_OK)
    {
        printf("Unable to create reply message!\n");
        return;
    }

    natsStatus reply_status = natsConnection_PublishMsg(nc, reply_msg);

    if (reply_status != NATS_OK)
    {
        printf("Error replying to %s\n", "");
    }

    natsMsg_Destroy(reply_msg);
}