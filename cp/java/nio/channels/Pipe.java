package java.nio.channels;

import java.io.IOException;
import java.nio.channels.spi.AbstractSelectableChannel;
import java.nio.channels.spi.SelectorProvider;

public abstract class Pipe {

  public abstract static class SinkChannel extends AbstractSelectableChannel
  implements GatheringByteChannel {

    protected SinkChannel(final SelectorProvider newProvider) {
      super(newProvider);
    }

    @Override
    public int validOps() {
      return SelectionKey.OP_WRITE;
    }
  }

  public abstract static class SourceChannel extends AbstractSelectableChannel
  implements ScatteringByteChannel {

    protected SourceChannel(final SelectorProvider newProvider) {
      super(newProvider);
    }

    @Override
    public int validOps() {
      return SelectionKey.OP_READ;
    }

  }

  static Pipe open() throws IOException {
    final SelectorProvider defaultProvider = SelectorProvider.provider();
    if (defaultProvider == null)
      throw new IOException();
    return defaultProvider.openPipe();
  }

  protected Pipe() {

  }

  public abstract SinkChannel sink();

  public abstract SourceChannel source();
}
