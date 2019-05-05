package java.nio.channels;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.spi.AbstractInterruptibleChannel;

public abstract class FileChannel extends AbstractInterruptibleChannel implements ByteChannel, GatheringByteChannel, ScatteringByteChannel {

  protected FileChannel() {

  }

  public final FileLock lock() throws IOException {
    return lock(0L, Long.MAX_VALUE, false);
  }

  @Override
  public long read(final ByteBuffer[] dsts) throws IOException {
    return read(dsts, 0, dsts.length);
  }

  @Override
  public long write(final ByteBuffer[] srcs) throws IOException {
    return write(srcs, 0, srcs.length);
  }

  abstract void force(boolean metaData);

  abstract FileLock lock(long position, long size, boolean shared) throws IOException;
}
