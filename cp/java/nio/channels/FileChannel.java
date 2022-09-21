package java.nio.channels;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.spi.AbstractInterruptibleChannel;

public abstract class FileChannel extends AbstractInterruptibleChannel implements ByteChannel, GatheringByteChannel, ScatteringByteChannel {

  protected FileChannel() {

  }

  public long read(ByteBuffer[] dsts) throws IOException {
    return read(dsts, 0, dsts.length);
  }

  abstract long read(ByteBuffer dst, long position);

  @Override
  public long write(final ByteBuffer[] srcs) throws IOException {
    return write(srcs, 0, srcs.length);
  }

  abstract int write(ByteBuffer src, long position);

  abstract void force(boolean metaData);

  public FileLock lock() throws IOException {
    return lock(0, Integer.MAX_VALUE, false);
  }

  abstract FileLock lock(long position, long size, boolean shared) throws IOException;
}
