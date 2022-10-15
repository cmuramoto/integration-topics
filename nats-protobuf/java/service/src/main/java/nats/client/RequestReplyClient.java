package nats.client;

import java.time.Duration;
import java.util.function.Function;

import com.google.protobuf.MessageLite;

import io.nats.client.Message;
import io.nats.client.Options;
import nats.share.NatsHandle;

public final class RequestReplyClient implements AutoCloseable {

	final NatsHandle handle;

	public RequestReplyClient(Options opts) {
		this.handle = new NatsHandle(opts);
	}

	public boolean available() {
		return handle.available();
	}

	public <T extends MessageLite, V> V request(T message, String subject, long timeout, Function<Message, V> handler) {
		var chunk = message.toByteArray();
		try {
			var reply = handle.connection().request(subject, chunk, Duration.ofMillis(timeout));

			return handler.apply(reply);
		} catch (InterruptedException e) {
			Thread.currentThread().interrupt();
			throw new RuntimeException("Timed out");
		}
	}

	@Override
	public void close() {
		handle.close();
	}

}
