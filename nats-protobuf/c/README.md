## Environment Variables

See [defs.h](https://github.com/cmuramoto/integration-topics/blob/2c5035ba89949435d9fa9f981c80fdd6b4af0cf3/nats-protobuf/c/include/defs.h)

- **NATS_SERVERS**: nats urls (Will split by , eventually. Just use 1 server for now). Default **"localhost:4222"**
- **LOG_RECEIVE**: Logs when a message is received, before any handler actions. Default **false**
- **LOG_DESERIALIZE**: Logs deserialization status (conversion **of opaque payloads to a typed protobuf object). Default **false**
- **LOG_SERIALIZE**: Logs serialization status (conversion of typed** protobuf object to an opaque payload). Default **false**
- **SERVER_WORKERS**: Number of pthreads (listeners). Default **1**

## Debug support

VSCode files are kind of pre-configured to build and debug **server.c**, assuming your env is pre-configured with protobuf and nats libs.

The debug task will build server.c by linking the protobuf model statically, so it expects **libtest_zmq.pb-c_static.a** to exist on root dir.

If the server fails to start check the *compile-proto-files* task.

