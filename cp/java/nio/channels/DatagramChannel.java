package java.nio.channels;

import java.io.IOException;
import java.net.SocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.spi.AbstractSelectableChannel;
import java.nio.channels.spi.SelectorProvider;

public abstract class DatagramChannel extends AbstractSelectableChannel
implements ByteChannel, ScatteringByteChannel, GatheringByteChannel {

  protected DatagramChannel(final SelectorProvider newProvider) {
    super(newProvider);
  }

  public abstract DatagramChannel connect(SocketAddress remote) throws IOException;

  public abstract DatagramChannel disconnect() throws IOException;

  public abstract SocketAddress receive(ByteBuffer dst) throws IOException;

  public abstract int send(ByteBuffer src, SocketAddress target) throws IOException;
}
