#include "server.proto.h"

typedef unsigned long thread_id;

int max = 1000;
volatile int64_t dropped = 0;

bool async = true;
bool auto_unsubscribe = false;

const char *subj = "sub";
const char *queue_name = "c-workers";

static void onMsg(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *closure)
{
    const char *subject = natsMsg_GetSubject(msg);

    const char *reply = natsMsg_GetReply(msg);

    const char *data = natsMsg_GetData(msg);

    int len = natsMsg_GetDataLength(msg);

    thread_id thread = (thread_id)pthread_self();

    if (server_config->log_receive)
    {
        printf("[%lu]Received msg: %s - %d(bytes)\n",
               thread,
               subject,
               len);
    }

    NatsModel__MsgSyncPlataoStvdT *unpacked = nats_model__msg_sync_platao_stvd_t__unpack(NULL, len, data);

    if (server_config->log_deserialize)
    {
        printf("[%lu]Deserializing packet of %d bytes\n", thread, len);
    }

    char out[1024];

    size_t packed_len = nats_model__msg_sync_platao_stvd_t__pack(unpacked, out);

    if (server_config->log_serialize)
    {
        printf("[%lu]Serialized packet to %d bytes\n", thread, (int)packed_len);
    }

    nats_model__msg_sync_platao_stvd_t__free_unpacked(unpacked, NULL);

    natsMsg *reply_msg;

    natsStatus create_status = natsMsg_Create(&reply_msg, reply, subject, out, packed_len);

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

static void onMsgDelegate(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *closure(natsConnection *, natsSubscription *, natsMsg *))
{
    closure(nc, sub, msg);
}

natsStatus doOpts(natsOptions *opts, const char **servers, int numServers)
{
    natsStatus st = natsOptions_Create(&opts);
    natsOptions_SetServers(opts, servers, numServers);
    natsOptions_SetAllowReconnect(opts, true);
    natsOptions_SetSendAsap(opts, true);
    natsOptions_SetTimeout(opts, 15000);
    natsOptions_SetMaxPendingMsgs(opts, 1000000);
    natsOptions_SetIOBufSize(opts, 1024 * 1024);

    return st;
}

static void
asyncCb(natsConnection *nc, natsSubscription *sub, natsStatus err, void *closure)
{
    printf("Async error: %u - %s\n", err, natsStatus_GetText(err));

    natsSubscription_GetDropped(sub, (int64_t *)&dropped);
}

void create_subscriber(const char **servers, int num_servers)
{

    natsOptions *opts;
    natsConnection *conn = NULL;
    natsSubscription *sub = NULL;
    natsStatistics *stats = NULL;
    natsMsg *msg = NULL;
    natsStatus s;

    natsStatus st = natsOptions_Create(&opts);
    natsOptions_SetServers(opts, servers, 1);
    natsOptions_SetAllowReconnect(opts, true);
    natsOptions_SetSendAsap(opts, true);
    natsOptions_SetTimeout(opts, 15000);
    natsOptions_SetMaxPendingMsgs(opts, 1000000);
    natsOptions_SetIOBufSize(opts, 1024 * 1024);

    thread_id thread = (thread_id)pthread_self();

    printf("[Worker#%lu]Listening %ssynchronously for requests on %s::%s\n",
           thread,
           (async ? "a" : ""),
           queue_name,
           subj);

    s = natsOptions_SetErrorHandler(opts, asyncCb, NULL);
    // Since the replier is sending one message at a time, reduce
    // latency by making Publish calls send data right away
    // instead of buffering them.
    if (s == NATS_OK)
        s = natsOptions_SetSendAsap(opts, true);

    if (s == NATS_OK)
        s = natsConnection_Connect(&conn, opts);

    if (s == NATS_OK)
    {
        if (async)
        {
// s = natsConnection_QueueSubscribe(&sub, conn, subj, queue_name, onMsg, NULL);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
            s = natsConnection_QueueSubscribe(&sub, conn, subj, queue_name, onMsgDelegate, &onZmq);
#pragma GCC diagnostic pop
        }
        else
        {
            s = natsConnection_QueueSubscribeSync(&sub, conn, subj, queue_name);
        }
    }

    // For maximum performance, set no limit on the number of pending messages.
    if (s == NATS_OK)
        s = natsSubscription_SetPendingLimits(sub, -1, -1);

    if (s == NATS_OK && auto_unsubscribe)
    {
        s = natsSubscription_AutoUnsubscribe(sub, max);
    }

    if (s == NATS_OK)
    {
        s = natsStatistics_Create(&stats);
    }

    while (s == NATS_OK)
    {
        int cycles = 0;
        if (s == NATS_OK)
        {
            nats_Sleep(1000);

            if (++cycles % 60 == 0)
            {
                printStats(STATS_IN | STATS_COUNT, conn, sub, stats);
            }
        }
    }

    if (s == NATS_OK)
    {
        printStats(STATS_IN | STATS_COUNT, conn, sub, stats);
        // printPerf("Received");
    }
    else
    {
        printf("Error: %u - %s\n", s, natsStatus_GetText(s));
        nats_PrintLastErrorStack(stderr);
    }

    // Destroy all our objects to avoid report of memory leak
    natsStatistics_Destroy(stats);
    natsSubscription_Destroy(sub);
    natsConnection_Destroy(conn);
    natsOptions_Destroy(opts);
}

void *thread_run(void *args)
{
    const char **servers = (const char **)args;
    const int num_servers = sizeof(servers) / sizeof(char);

    create_subscriber(servers, num_servers);
}

void create_subscriber_threads(const char **servers, int max_stack)
{
    int n_threads = server_config->workers;

    printf("Allocating %d workers\n", n_threads);

    pthread_attr_t stack_attr;
    pthread_attr_init(&stack_attr);

    pthread_t threads[n_threads];
    char stacks[n_threads][max_stack];

    for (int i = 0; i < n_threads; i++)
    {
        // manually set stack space & stack size for each thread
        // to reduce virtual memory cost
        pthread_attr_setstack(&stack_attr, &stacks[i], max_stack);

        // create thread using customized stack space
        pthread_create(&threads[i], &stack_attr, thread_run, servers);
    }

    for (int i = 0; i < n_threads; i++)
    {
        // just block
        pthread_join(threads[i], NULL);
    }
}

int main(int argc, char **argv)
{
    set_env();
    
    create_subscriber_threads((const char **)server_config->servers, 256 * 1024);

    // To silence reports of memory still in used with valgrind
    nats_Close();

    return 0;
}
