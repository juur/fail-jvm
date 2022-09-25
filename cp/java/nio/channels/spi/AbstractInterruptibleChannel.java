package java.nio.channels.spi;

import java.io.IOException;
import java.nio.channels.InterruptibleChannel;

public abstract class AbstractInterruptibleChannel implements InterruptibleChannel {

  private boolean      isOpen    = false;
  private final Object closeSync = new Object();

  protected AbstractInterruptibleChannel() {

  }

  @Override
  public final void close() throws IOException {
    synchronized (closeSync) {
      if (isOpen)
        return;

      isOpen = false;
      implCloseChannel();
    }
  }

  @Override
  public final boolean isOpen() {
    return isOpen;
  }

  protected final void begin() {

  }

  /**
   * @throws AsynchronousCloseException
   */
  protected final void end(final boolean completed) throws AsynchronousCloseException {

  }

  protected abstract void implCloseChannel() throws IOException;
}
