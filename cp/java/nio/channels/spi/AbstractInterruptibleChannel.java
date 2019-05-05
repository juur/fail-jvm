package java.nio.channels.spi;

import java.io.IOException;
import java.nio.channels.InterruptibleChannel;

public abstract class AbstractInterruptibleChannel implements InterruptibleChannel {

  protected AbstractInterruptibleChannel() {

  }

  @Override
  public final void close() throws IOException {

  }

  @Override
  public final boolean isOpen() {
    return false;
  }

  protected final void begin() {

  }

  protected final void end(final boolean completed) throws AsynchronousCloseException {

  }

  protected abstract void implCloseChannel() throws IOException;
}
