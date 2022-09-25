package java.nio.channels;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.spi.AbstractInterruptibleChannel;

public abstract class FileChannel extends AbstractInterruptibleChannel implements ByteChannel, GatheringByteChannel, ScatteringByteChannel {

  protected FileChannel() {
    super();
  }

  abstract public void force(boolean metaData);

  public FileLock lock() throws IOException {
    return lock(0, Integer.MAX_VALUE, false);
  }

  abstract public FileLock lock(long position, long size, boolean shared) throws IOException;

  abstract public long position();

  abstract public FileChannel position(long newPosition);

  abstract public long read(ByteBuffer dst, long position) throws IOException;

  @Override
  public long read(final ByteBuffer[] dsts) throws IOException {
    return read(dsts, 0, dsts.length);
  }

  abstract public int write(ByteBuffer src, long position) throws IOException;

  @Override
  public long write(final ByteBuffer[] srcs) throws IOException {
    return write(srcs, 0, srcs.length);
  }
}
