package nats.share;

import java.io.IOException;
import java.io.UncheckedIOException;

import io.nats.client.Connection;
import io.nats.client.Connection.Status;
import io.nats.client.Nats;
import io.nats.client.Options;

public class NatsHandle implements AutoCloseable {

	private final Options opts;
	volatile Connection conn;

	public NatsHandle(Options opts) {
		this.opts = opts;
	}

	public final Connection connection() {
		var c = conn;
		if (c == null) {
			connect();
			c = conn;
		}
		return c;
	}

	public boolean available() {
		try {
			return connection().getStatus() == Status.CONNECTED;
		} catch (Exception e) {
			return false;
		}
	}

	private void connect() {
		try {
			this.conn = Nats.connect(opts);
		} catch (InterruptedException e) {
			Thread.currentThread().interrupt();
		} catch (IOException e) {
			throw new UncheckedIOException(e);
		}
	}

	@Override
	public void close() {
		var c = conn;
		if (c != null) {
			try {
				c.close();
			} catch (InterruptedException e) {
				Thread.currentThread().interrupt();
			}
			conn = null;
		}
	}
}
