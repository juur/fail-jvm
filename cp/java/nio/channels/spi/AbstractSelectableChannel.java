package java.nio.channels.spi;

import java.io.IOException;
import java.nio.channels.SelectableChannel;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;

public abstract class AbstractSelectableChannel extends SelectableChannel {

  private final SelectorProvider provider;
  private final int              registered;
  private boolean                blocking;
  private final Object           blockingObj;

  protected AbstractSelectableChannel(final SelectorProvider newProvider) {
    super();
    provider = newProvider;
    registered = 0;
    blocking = true;
    blockingObj = new Object();
  }

  @Override
  public final Object blockingLock()
  {
    return blockingObj;
  }

  @Override
  public final SelectableChannel configureBlocking(final boolean block) throws IOException {
    if (block != blocking) {
      blocking = block;
      implConfigureBlocking(blocking);
    }
    return this;
  }

  @Override
  protected final void implCloseChannel() throws IOException {
    implCloseSelectableChannel();
    // TODO;
  }

  protected abstract void implCloseSelectableChannel() throws IOException;

  protected abstract void implConfigureBlocking(boolean block)
    throws IOException;

  @Override
  public final boolean isBlocking() {
    return blocking;
  }

  @Override
  public final boolean isRegistered() {
    return registered > 0;
  }

  @Override
  public final SelectionKey keyFor(final Selector sel) {
    // TODO
    return null;
  }

  @Override
  public final SelectorProvider provider() {
    return provider;
  }

  @Override
  public final SelectionKey register(final Selector sel, final int ops, final Object att)
    throws ClosedChannelException {
    if (!isOpen())
      throw new ClosedChannelException();

    // TODO

    return null;
  }
}
