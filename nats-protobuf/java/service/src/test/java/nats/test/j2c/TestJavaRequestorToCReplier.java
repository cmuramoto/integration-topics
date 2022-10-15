package nats.test.j2c;

import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.MethodOrderer;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.TestMethodOrder;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

@TestMethodOrder(MethodOrderer.MethodName.class)
@SuppressWarnings("java:S2699")
class TestJavaRequestorToCReplier {

	static final Logger LOG = LoggerFactory.getLogger(TestJavaRequestorToCReplier.class);

	final RequestReplySupport sup = new RequestReplySupport();

	@Test
	void _00_sendOne() {
		sup.one();
	}

	@Test
	void _01_sendManyInParallel() {
		sup.manyWithCompletableFuture(10000, true);
	}

	@Test
	void _02_sendManyInParallelWithStreams() {
		sup.many(10000, true);
	}

	@AfterEach
	public void destroy() {
		sup.destroy();
	}

	String externalNatsSocket() {
		return "localhost:4222";
	}

	@BeforeEach
	public void init() {
		sup.init(externalNatsSocket());
	}
}
