package nats.server;

import java.util.function.Function;

import io.nats.client.Dispatcher;
import io.nats.client.Message;
import io.nats.client.MessageHandler;
import io.nats.client.Options;
import io.nats.client.Subscription;
import nats.share.NatsHandle;

public class EchoServerHandler {

	private final Options options;

	public EchoServerHandler(Options options) {
		this.options = options;
	}

	public static class SubscriptionHandler implements MessageHandler, AutoCloseable {
		final NatsHandle handle;
		final String subject;
		final String queue;
		final Function<Message, byte[]> conv;

		Dispatcher dispatcher;
		Subscription subscription;

		public SubscriptionHandler(Options opts, String subject, String queue, Function<Message, byte[]> conv) {
			this.handle = new NatsHandle(opts);
			this.conv = conv;
			this.subject = subject;
			this.queue = queue;
		}

		public void initialize() {
			unsubscribe();
			this.dispatcher = handle.connection().createDispatcher();
			this.subscription = this.dispatcher.subscribe(subject, queue, this);
		}

		@Override
		public void onMessage(Message msg) throws InterruptedException {
			msg.getConnection().publish(msg.getReplyTo(), conv.apply(msg));
		}

		private void unsubscribe() {
			var d = dispatcher;
			var s = subscription;

			if (d != null && s != null) {
				d.unsubscribe(s);
			}
			dispatcher = null;
			subscription = null;
		}

		@Override
		public void close() {
			unsubscribe();
			handle.close();
		}

		public boolean available() {
			return handle.available();
		}
	}

	public SubscriptionHandler allocate(String subject, String queue, Function<Message, byte[]> conv) {
		return new SubscriptionHandler(options, subject, queue, conv);
	}
}
