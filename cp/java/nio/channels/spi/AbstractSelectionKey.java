package java.nio.channels.spi;

import java.nio.channels.SelectionKey;


public abstract class AbstractSelectionKey extends SelectionKey {

  private boolean cancelled;

  protected AbstractSelectionKey() {
    super();
    cancelled = false;
  }

  @Override
  public final void cancel()
  {
    if (cancelled)
      return;
    // TODO
    cancelled = true;
  }

  @Override
  public final boolean isValid() {
    if (cancelled)
      return false;
    return true;
  }
}
