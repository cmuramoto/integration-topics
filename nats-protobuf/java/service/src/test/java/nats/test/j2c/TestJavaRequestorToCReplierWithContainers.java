package nats.test.j2c;

import java.util.Map;
import java.util.concurrent.locks.LockSupport;

import org.junit.jupiter.api.MethodOrderer;
import org.junit.jupiter.api.TestMethodOrder;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.testcontainers.containers.GenericContainer;
import org.testcontainers.containers.Network;
import org.testcontainers.junit.jupiter.Container;
import org.testcontainers.junit.jupiter.Testcontainers;
import org.testcontainers.utility.DockerImageName;

@Testcontainers
@TestMethodOrder(MethodOrderer.MethodName.class)
@SuppressWarnings("java:S2699")
class TestJavaRequestorToCReplierWithContainers extends TestJavaRequestorToCReplier {

	static final Logger LOG = LoggerFactory.getLogger(TestJavaRequestorToCReplierWithContainers.class);

	static final Logger NATS_LOGGER = LoggerFactory.getLogger("nats");

	static final Logger C_SERVER_LOGGER = LoggerFactory.getLogger("c-server");

	public static final DockerImageName NATS_IMAGE = DockerImageName.parse("nats:2.9.3");

	public static final DockerImageName C_SERVER_IMAGE = DockerImageName.parse("cmuramoto/platao-c-server:1.0-static-alpine");

	static final Network NET = Network.builder().createNetworkCmdModifier(cmd -> {
		cmd.withName("nats-proto-net");
	}).build();

	@Container
	public static GenericContainer<?> nats = new GenericContainer<>(NATS_IMAGE)
			.withExposedPorts(4222, 6222, 8222)
			.withNetwork(NET)
			.withCreateContainerCmdModifier(cmd -> {
				cmd.withName("nats").withHostName("nats");
			})
			.withLogConsumer(frame -> {
				var msg = frame.getUtf8String();
				if (msg != null) {
					msg = msg.trim();
				}
				NATS_LOGGER.info(msg);
			});

	@Container
	public static GenericContainer<?> cServer = new GenericContainer<>(C_SERVER_IMAGE)
			.dependsOn(nats)
			.withNetwork(NET)
			.withEnv(Map.of(
					"NATS_SERVERS", internalNatsSocket(),
					"WORKERS", "8"))
			.withLogConsumer(frame -> {
				var msg = frame.getUtf8String();
				if (msg != null) {
					msg = msg.trim();
				}
				C_SERVER_LOGGER.info(msg);
			});

	static String resolvedNatsAddress;

	static void ensureNatsInitialized() {
		if (!nats.isRunning()) {
			LOG.info("Forcing nats to boot");
			nats.start();

			int max = 10;
			while (!nats.isRunning()) {
				if (--max <= 0) {
					LOG.info("Nats didn't boot!");
					System.exit(1);
				}
				LOG.info("Awaiting nats to transition to running");

				LockSupport.parkNanos(1_000_000_000L);

			}
		}
	}

	static String internalNatsSocket() {
		var s = "nats" + ":" + 4222;

		return s;
	}

	@Override
	String externalNatsSocket() {
		var r = resolvedNatsAddress;

		if (r == null) {
			ensureNatsInitialized();
			r = nats.getHost() + ":" + nats.getMappedPort(4222);
			resolvedNatsAddress = r;
		}

		return r;
	}

}
