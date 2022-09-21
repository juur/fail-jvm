package java.nio.channels;

import java.io.IOException;
import java.net.ServerSocket;
import java.nio.channels.spi.AbstractSelectableChannel;
import java.nio.channels.spi.SelectorProvider;


public abstract class ServerSocketChannel extends AbstractSelectableChannel {

  public static ServerSocketChannel open()
    throws IOException
  {
    final SelectorProvider defaultProvider = SelectorProvider.provider();
    if (defaultProvider == null) throw new IOException();
    return defaultProvider.openServerSocketChannel();
  }

  protected ServerSocketChannel(final SelectorProvider newProvider) {
    super(newProvider);
  }

  public abstract SocketChannel accept() throws IOException;

  public abstract ServerSocket socket();

  @Override
  public final int validOps() {
    return SelectionKey.OP_ACCEPT;
  }
}
