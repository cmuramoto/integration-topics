package nats.test.j2c;

import java.nio.file.Paths;

import org.junit.jupiter.api.MethodOrderer;
import org.junit.jupiter.api.TestMethodOrder;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.testcontainers.containers.DockerComposeContainer;
import org.testcontainers.junit.jupiter.Container;
import org.testcontainers.junit.jupiter.Testcontainers;

@Testcontainers
@TestMethodOrder(MethodOrderer.MethodName.class)
@SuppressWarnings("java:S2699")
class TestJavaRequestorToCReplierWithCompose extends TestJavaRequestorToCReplier {

	static final Logger LOG = LoggerFactory.getLogger(TestJavaRequestorToCReplierWithCompose.class);

	static final Logger NATS_LOGGER = LoggerFactory.getLogger("nats");

	static final Logger C_SERVER_LOGGER = LoggerFactory.getLogger("c-server");

	@Container
	public static DockerComposeContainer<?> compose = new DockerComposeContainer<>(Paths.get("src/test/resources/docker-compose.yml").toFile())
			.withLogConsumer("nats", frame -> {
				var msg = frame.getUtf8String();
				if (msg != null) {
					msg = msg.trim();
				}
				NATS_LOGGER.info(msg);
			})
			.withLogConsumer("platao-c-server", frame -> {
				var msg = frame.getUtf8String();
				if (msg != null) {
					msg = msg.trim();
				}
				C_SERVER_LOGGER.info(msg);
			});

}
