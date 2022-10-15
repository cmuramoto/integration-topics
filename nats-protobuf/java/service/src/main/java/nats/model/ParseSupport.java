package nats.model;

import com.google.protobuf.InvalidProtocolBufferException;

import io.nats.client.Message;
import nats.model.ZmqTestProtos.MsgSyncPlataoStvd_t;

public class ParseSupport {

	public static MsgSyncPlataoStvd_t parse(byte[] chunk) {
		try {
			return MsgSyncPlataoStvd_t.parseFrom(chunk);
		} catch (InvalidProtocolBufferException e) {
			throw new RuntimeException(e);
		}
	}

	public static MsgSyncPlataoStvd_t parse(Message message) {
		return parse(message.getData());
	}

}
