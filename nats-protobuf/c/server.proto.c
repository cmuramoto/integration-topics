#include "server.proto.h"

typedef unsigned long thread_id;

int max = 1000;
volatile int64_t dropped = 0;

bool async = true;
bool auto_unsubscribe = false;

const char *subj = "sub";
const char *queue_name = "c-workers";

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

void create_subscriber(const char **servers, uint32_t num_servers)
{

    natsOptions *opts;
    natsConnection *conn = NULL;
    natsSubscription *sub = NULL;
    natsStatistics *stats = NULL;
    natsMsg *msg = NULL;
    natsStatus s;

    natsStatus st = natsOptions_Create(&opts);
    natsOptions_SetServers(opts, servers, num_servers);
    natsOptions_SetAllowReconnect(opts, true);
    natsOptions_SetSendAsap(opts, true);
    natsOptions_SetTimeout(opts, 15000);
    natsOptions_SetMaxPendingMsgs(opts, 1000000);
    natsOptions_SetIOBufSize(opts, 1024 * 1024);

    thread_id thread = (thread_id)pthread_self();

    printf("[Worker#%lu]Listening %s for requests on %s::%s\n",
           thread,
           (async ? "asynchronously" : "synchronously"),
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
    NatsUrls *urls = (NatsUrls *)args;
    const char **servers = as_const(urls->urls);
    const uint32_t num_servers = urls->length;

    create_subscriber(servers, num_servers);
}

void create_subscriber_threads(int max_stack)
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
        pthread_create(&threads[i], &stack_attr, thread_run, &(server_config->nats_urls));
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

    create_subscriber_threads(256 * 1024);

    // To silence reports of memory still in used with valgrind
    nats_Close();

    return 0;
}
