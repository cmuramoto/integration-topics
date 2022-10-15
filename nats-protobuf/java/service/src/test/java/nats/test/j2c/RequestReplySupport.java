package nats.test.j2c;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assumptions.assumeTrue;

import java.util.Arrays;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ThreadLocalRandom;
import java.util.stream.Collectors;
import java.util.stream.IntStream;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import io.nats.client.Options;
import nats.client.RequestReplyClient;
import nats.model.ParseSupport;
import nats.model.ZmqTestProtos;

public class RequestReplySupport {

	static final Logger LOG = LoggerFactory.getLogger(RequestReplySupport.class);

	String subject = "sub";

	RequestReplyClient[] clients;

	private CompletableFuture<Integer> callMany(RequestReplyClient cli, int max, boolean check) {
		return CompletableFuture.supplyAsync(() -> {
			var tlr = ThreadLocalRandom.current();
			var assertions = IntStream.range(0, max).map(__ -> {
				var message = ZmqTestProtos.MsgSyncPlataoStvd_t.newBuilder()
						.setComando(tlr.nextInt())
						.setMsgId(tlr.nextInt())
						.setNumExe(tlr.nextInt())
						.setHoraMaquina(System.currentTimeMillis())
						.setTamanho(tlr.nextInt())
						.build();

				try {
					var reply = cli.request(message, subject, 1000, ParseSupport::parse);

					return check
							? message.equals(reply)
									? 1
									: 0
							: 1;

				} catch (Exception e) {
					return 0;
				}
			}).sum();
			return assertions;
		});
	}

	public void destroy() {
		if (clients == null) {
			return;
		}
		for (var cli : clients) {
			if (cli != null) {
				cli.close();
			}
		}
	}

	@SuppressWarnings("resource")
	public void init(String nats) {
		var servers = new String[]{ nats };
		var opts = new Options.Builder().servers(servers).build();
		clients = new RequestReplyClient[Runtime.getRuntime().availableProcessors()];
		for (var i = 0; i < clients.length; i++) {
			var cli = new RequestReplyClient(opts);
			assumeTrue(cli.available());
			clients[i] = cli;
		}
	}

	public void many(int max, boolean check) {
		var now = System.currentTimeMillis();

		var allAssertions = Arrays.stream(clients).parallel().mapToInt(cli -> {
			var tlr = ThreadLocalRandom.current();
			var assertions = IntStream.range(0, max).map(__ -> {
				var message = ZmqTestProtos.MsgSyncPlataoStvd_t.newBuilder()
						.setComando(tlr.nextInt())
						.setMsgId(tlr.nextInt())
						.setNumExe(tlr.nextInt())
						.setHoraMaquina(System.currentTimeMillis())
						.setTamanho(tlr.nextInt())
						.build();

				try {
					var reply = cli.request(message, subject, 1000, ParseSupport::parse);

					return check
							? message.equals(reply)
									? 1
									: 0
							: 1;

				} catch (Exception e) {
					return 0;
				}
			}).sum();
			return assertions;
		}).sum();

		var elapsed = System.currentTimeMillis() - now;

		assertEquals(max * clients.length, allAssertions);

		LOG.info("(Streams#check_eq={})Total Messages: {}. Elapsed: {}. Throughput: {} msgs/s", check, allAssertions, +elapsed, ((1000d * allAssertions) / elapsed));
	}

	public void manyWithCompletableFuture(int max, boolean check) {

		var now = System.currentTimeMillis();

		var allAssertions = Arrays.stream(clients)
				.map(cli -> callMany(cli, max, check))
				.collect(Collectors.toList())
				.stream()
				.mapToInt(cf -> {
					try {
						return cf.get();
					} catch (Exception e) {
						return 0;
					}
				}).sum();

		var elapsed = System.currentTimeMillis() - now;

		assertEquals(max * clients.length, allAssertions);

		LOG.info("(CompletableFutures)Total Messages: {}. Elapsed: {}. Throughput: {} msgs/s", allAssertions, +elapsed, ((1000d * allAssertions) / elapsed));
	}

	public void one() {
		var message = ZmqTestProtos.MsgSyncPlataoStvd_t.newBuilder()
				.setComando(1)
				.setMsgId(42)
				.setNumExe(12)
				.setHoraMaquina(System.currentTimeMillis())
				.setTamanho(10)
				.build();

		var reply = clients[0].request(message, subject, 10000, ParseSupport::parse);

		assertEquals(message, reply);
	}
}
