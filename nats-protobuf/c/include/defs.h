#pragma once

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <nats/nats.h>
#include <protobuf-c/protobuf-c.h>

typedef unsigned long thread_id;

typedef struct
{
    char **urls;
    uint32_t length;
} NatsUrls;

typedef struct
{
    bool log_receive;
    bool log_deserialize;
    bool log_serialize;
    int workers;
    NatsUrls nats_urls;
} ServerConfig;

typedef struct
{
    uint8_t *buffer;
    size_t len;
    thread_id owner;
} GrowableArray;

ServerConfig sc;
ServerConfig *server_config = &sc;
const char *WATERMARK = "THE_WATERMARK";

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
const char **as_const(char **c)
{
    return c;
}
#pragma GCC diagnostic pop

bool should_trim(char c)
{
    return (c == ' ' || c == '\t' || c == '\n');
}

char *trim(char *s)
{
    char *p = s;

    while (should_trim(*p))
    {
        p++;
    }

    char *q = s + (strlen(s) - 1);

    while (should_trim(*q))
    {
        *q = '\0';
        q--;
    }

    return p;
}

bool get_env_bool(const char *key)
{
    char *val = getenv(key);

    return val != NULL && strcmp("true", val) == 0;
}

int get_env_int(const char *key, int def)
{
    char *val = getenv(key);

    return val != NULL ? atoi(val) : def;
}

char *get_env_str(const char *key, char *def)
{
    char *val = getenv(key);

    return val != NULL ? val : def;
}

const char *to_string(bool b)
{
    return b ? "true" : "false";
}

void resolve_nats_urls(char *ns)
{
    int len = strlen(ns) + 1;
    char servers[len];
    memccpy(servers, ns, 0, len);

    int tokens = 0;
    char *ptr = strtok(servers, ",");

    char *urls[32];

    while (ptr != NULL)
    {
        char *p = trim(ptr);

        char *array = (char *)malloc(strlen(p) + 1);
        memccpy(array, p, 0, strlen(p) + 1);
        urls[tokens++] = array;

        ptr = strtok(NULL, ",");

        if (tokens >= 32)
        {
            printf("Too many servers");
            exit(1);
        }
    }

    const size_t address_size = sizeof(void *);

    char **rv = (char **)(malloc(tokens * address_size));

    for (int i = 0; i < tokens; i++)
    {
        rv[i] = urls[i];
    }

    NatsUrls *nats_urls = &(server_config->nats_urls);
    nats_urls->urls = rv;
    nats_urls->length = (uint32_t)tokens;
}

void set_env()
{
    ServerConfig *cfg = server_config;
    cfg->log_deserialize = get_env_bool("LOG_DESERIALIZE");
    cfg->log_receive = get_env_bool("LOG_RECEIVE");
    cfg->log_serialize = get_env_bool("LOG_SERIALIZE");
    cfg->workers = get_env_int("SERVER_WORKERS", 1);
    char *urls = get_env_str("NATS_SERVERS", "localhost:4222");

    resolve_nats_urls(urls);

    printf("ServerConfig: {log: {receive: %s, serialize: %s, deserialize: %s}, workers: %d, server_urls: %s}. Version Watermark: %s\n",
           to_string(cfg->log_receive),
           to_string(cfg->log_serialize),
           to_string(cfg->log_deserialize),
           cfg->workers,
           urls,
           WATERMARK);
}

void print_watermark()
{
    printf("Version Watermark: %s\n", WATERMARK);
}

size_t align(size_t req, size_t alignment)
{
    assert((alignment & (alignment - 1)) == 0);
    size_t aligned_size = (req + alignment - 1) & -alignment;

    return aligned_size;
}

void require(GrowableArray *array, size_t min)
{
    if (array->len < min)
    {
        if (array->buffer)
        {
            free(array->buffer);
        }

        size_t req = align(min, 4096);

        void *mem = malloc(req);

        if (!mem)
        {
            printf("OOM");
            exit(1);
        }

        array->buffer = (uint8_t *)mem;
        array->len = req;
    }
}

GrowableArray *allocate_growable_array(size_t initial)
{
    thread_id tid = (thread_id)pthread_self();

    printf("Creating growable array for [%lu] with initial size [%lu]\n", tid, initial);

    const size_t sz = sizeof(GrowableArray);

    GrowableArray *array = (GrowableArray *)malloc(sz);
    array->owner = (thread_id)pthread_self();
    require(array, initial);
}