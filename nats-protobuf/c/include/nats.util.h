#pragma once

#include "./defs.h"
#define STATS_IN 0x1
#define STATS_OUT 0x2
#define STATS_COUNT 0x4

static natsStatus
printStats(int mode, natsConnection *conn, natsSubscription *sub,
           natsStatistics *stats)
{
    natsStatus s = NATS_OK;
    uint64_t inMsgs = 0;
    uint64_t inBytes = 0;
    uint64_t outMsgs = 0;
    uint64_t outBytes = 0;
    uint64_t reconnected = 0;
    int pending = 0;
    int64_t delivered = 0;
    int64_t sdropped = 0;

    s = natsConnection_GetStats(conn, stats);
    if (s == NATS_OK)
        s = natsStatistics_GetCounts(stats, &inMsgs, &inBytes,
                                     &outMsgs, &outBytes, &reconnected);
    if ((s == NATS_OK) && (sub != NULL))
    {
        s = natsSubscription_GetStats(sub, &pending, NULL, NULL, NULL,
                                      &delivered, &sdropped);

        // May reach here when using AutoUnsubscribe(), just ignore.
        if (s == NATS_INVALID_SUBSCRIPTION)
        {
            s = NATS_OK;
            pending = 0;
        }
    }

    if (s == NATS_OK)
    {
        if (mode & STATS_IN)
        {
            printf("In Msgs: %9" PRIu64 " - "
                   "In Bytes: %9" PRIu64 " - ",
                   inMsgs, inBytes);
        }
        if (mode & STATS_OUT)
        {
            printf("Out Msgs: %9" PRIu64 " - "
                   "Out Bytes: %9" PRIu64 " - ",
                   outMsgs, outBytes);
        }
        if (mode & STATS_COUNT)
        {
            printf("Delivered: %9" PRId64 " - ", delivered);
            printf("Pending: %5d - ", pending);
            printf("Dropped: %5" PRId64 " - ", sdropped);
        }
        printf("Reconnected: %3" PRIu64 "\n", reconnected);
    }

    return s;
}

typedef struct
{
    char **array;
    int length;
} StringArray;