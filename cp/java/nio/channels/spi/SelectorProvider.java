package java.nio.channels.spi;

import java.io.IOException;
import java.nio.channels.Channel;
import java.nio.channels.DatagramChannel;
import java.nio.channels.Pipe;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;

public abstract class SelectorProvider {

  private static SelectorProvider defaultProvider;

  static {
    defaultProvider = null; // FIXME
  }

  public static SelectorProvider provider() {
    return defaultProvider;
  }

  protected SelectorProvider() {

  }

  public Channel inheritedChannel() throws IOException {
    return null; // TODO
  }

  public abstract DatagramChannel openDatagramChannel() throws IOException;

  public abstract Pipe openPipe() throws IOException;

  public abstract AbstractSelector openSelector() throws IOException;

  public abstract ServerSocketChannel openServerSocketChannel() throws IOException;

  public abstract SocketChannel openSocketChannel()
    throws IOException;
}
