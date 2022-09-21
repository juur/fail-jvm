package java.nio.channels;

import java.io.IOException;
import java.nio.channels.spi.AbstractInterruptibleChannel;
import java.nio.channels.spi.ClosedChannelException;
import java.nio.channels.spi.SelectorProvider;

public abstract class SelectableChannel extends AbstractInterruptibleChannel {

  protected SelectableChannel() {
    super();
  }

  public abstract Object blockingLock();

  public abstract SelectableChannel configureBlocking(boolean block) throws IOException;

  public abstract boolean isBlocking();

  public abstract boolean isRegistered();

  public abstract SelectionKey keyFor(Selector sel);

  public abstract SelectorProvider provider();

  public final SelectionKey register(final Selector sel, final int ops)
    throws ClosedChannelException {
    return register(sel, ops, null);
  }

  public abstract SelectionKey register(Selector sel, int ops, Object att)
    throws ClosedChannelException;

  public abstract int validOps();
}
