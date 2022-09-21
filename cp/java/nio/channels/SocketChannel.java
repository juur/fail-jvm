package java.nio.channels;

import java.io.IOException;
import java.net.SocketAddress;
import java.nio.channels.spi.AbstractSelectableChannel;
import java.nio.channels.spi.SelectorProvider;


public abstract class SocketChannel extends AbstractSelectableChannel
implements ByteChannel, ScatteringByteChannel, GatheringByteChannel {

  protected SocketChannel(final SelectorProvider newProvider) {
    super(newProvider);
  }

  public abstract boolean connect(SocketAddress remote) throws IOException;

  public abstract boolean finishConnect() throws IOException;
}
