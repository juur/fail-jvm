package java.nio.channels.spi;

import java.io.IOException;
import java.nio.channels.InterruptibleChannel;

public abstract class AbstractInterruptibleChannel implements InterruptibleChannel {

  protected AbstractInterruptibleChannel() {

  }

  protected final void begin() {

  }

  @Override
  public final void close() throws IOException {

  }

  /**
   * @throws AsynchronousCloseException
   */
  protected final void end(final boolean completed) throws AsynchronousCloseException {

  }

  protected abstract void implCloseChannel() throws IOException;

  @Override
  public final boolean isOpen() {
    return false;
  }
}
