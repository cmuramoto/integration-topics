package nats.test.j2j;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assumptions.assumeTrue;

import java.util.concurrent.ThreadLocalRandom;
import java.util.stream.IntStream;

import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.testcontainers.containers.GenericContainer;
import org.testcontainers.junit.jupiter.Container;
import org.testcontainers.junit.jupiter.Testcontainers;
import org.testcontainers.utility.DockerImageName;

import io.nats.client.Message;
import io.nats.client.Options;
import nats.client.RequestReplyClient;
import nats.model.ParseSupport;
import nats.model.ZmqTestProtos;
import nats.server.EchoServerHandler;
import nats.server.EchoServerHandler.SubscriptionHandler;

@Testcontainers
class ClientServerTests {

	public static final DockerImageName NATS_IMAGE = DockerImageName.parse("nats:2.9.3");

	@Container
	public static GenericContainer<?> nats = new GenericContainer<>(NATS_IMAGE)
			.withExposedPorts(4222, 6222, 8222)
			.withLogConsumer(frame -> {
				System.out.print(frame.getUtf8String());
			});

	String[] servers = { "localhost:" + nats.getMappedPort(4222) };

	String subject = "sub";

	RequestReplyClient client;

	SubscriptionHandler handler;

	@AfterEach
	public void destroy() {
		client.close();
		handler.close();
	}

	@BeforeEach
	public void init() {
		var opts = new Options.Builder().servers(servers).build();
		client = new RequestReplyClient(opts);
		var server = new EchoServerHandler(opts);
		handler = server.allocate(subject, "queue", this::roundTrip);

		assumeTrue(client.available());
		assumeTrue(handler.available());

		handler.initialize();
	}

	byte[] roundTrip(Message message) {
		return ParseSupport.parse(message).toByteArray();
	}

	@Test
	void run() {
		var message = ZmqTestProtos.MsgSyncPlataoStvd_t.newBuilder()
				.setComando(1)
				.setMsgId(42)
				.setNumExe(12)
				.setHoraMaquina(System.currentTimeMillis())
				.setTamanho(10)
				.build();

		var reply = client.request(message, subject, 10000, ParseSupport::parse);

		assertEquals(message, reply);
	}

	@Test
	void runMany() {
		var max = 10000;
		var tlr = ThreadLocalRandom.current();

		var assertions = IntStream.range(0, 10000).map(__ -> {
			var message = ZmqTestProtos.MsgSyncPlataoStvd_t.newBuilder()
					.setComando(tlr.nextInt())
					.setMsgId(tlr.nextInt())
					.setNumExe(tlr.nextInt())
					.setHoraMaquina(System.currentTimeMillis())
					.setTamanho(tlr.nextInt())
					.build();

			var reply = client.request(message, subject, 10000, ParseSupport::parse);

			return message.equals(reply) ? 1 : 0;
		}).sum();

		assertEquals(max, assertions);
	}

}
