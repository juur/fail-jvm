package java.nio.channels;

import java.io.IOException;
import java.nio.channels.spi.SelectorProvider;
import java.util.Set;

public abstract class Selector {

  public static Selector open() throws IOException {
    final SelectorProvider defaultProvider = SelectorProvider.provider();
    if (defaultProvider == null)
      throw new IOException();
    return SelectorProvider.provider().openSelector();
  }

  protected Selector() {}

  public abstract void close() throws IOException;

  public abstract boolean isOpen();

  public abstract Set<SelectionKey> keys();

  public abstract SelectorProvider provider();

  public abstract int select() throws IOException;

  public abstract int select(long timeout) throws IOException;

  public abstract Set<SelectionKey> selectedKeys();

  public abstract int selectNow() throws IOException;

  public abstract Selector wakeup();
}
