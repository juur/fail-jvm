package java.nio.channels.spi;

import java.io.IOException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.HashSet;
import java.util.Set;

public abstract class AbstractSelector extends Selector {

  private final SelectorProvider provider;
  private final boolean          open;
  private final Set<SelectionKey> keys;
  private final Set<SelectionKey> cancelledKeys;
  private boolean                 blockingIO;

  protected AbstractSelector(final SelectorProvider newProvider) {
    super();
    provider = newProvider;
    open = false;
    keys = new HashSet<SelectionKey>();
    cancelledKeys = new HashSet<SelectionKey>();
  }

  protected final void begin()
  {
    blockingIO = true;
    // TODO
  }

  protected final Set<SelectionKey> cancelledKeys()
  {
    return cancelledKeys();
  }

  @Override
  public final void close() throws IOException {
    if (!isOpen())
      return;
    implCloseSelector();
  }

  protected final void deregister(final AbstractSelectionKey key) {
    synchronized (keys) {
      keys.remove(key);
    }
  }

  protected final void end()
  {
    blockingIO = false;
    // TODO
  }

  protected abstract void implCloseSelector() throws IOException;

  @Override
  public final boolean isOpen() {
    return open;
  }

  @Override
  public final SelectorProvider provider()
  {
    return provider;
  }

  protected abstract SelectionKey register(AbstractSelectableChannel ch,
    int ops,
    Object att);
}
