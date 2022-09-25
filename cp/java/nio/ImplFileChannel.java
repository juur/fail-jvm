package java.nio;

import java.io.FileDescriptor;
import java.io.IOException;
import java.nio.channels.FileChannel;
import java.nio.channels.FileLock;
import java.nio.channels.spi.ClosedChannelException;

public class ImplFileChannel extends FileChannel {

  private final FileDescriptor fdObj;
  private final Object         readLock, writeLock;

  public ImplFileChannel(final FileDescriptor fd) {
    fdObj = fd;
    readLock = new Object();
    writeLock = new Object();
  }

  @Override
  public void force(final boolean metaData) {
    // TODO Auto-generated method stub

  }

  @Override
  public FileLock lock(final long position, final long size, final boolean shared) throws IOException {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public long position() {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public FileChannel position(final long newPosition) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  native public int read(final ByteBuffer dst) throws IOException;

  @Override
  native public long read(final ByteBuffer dst, final long position) throws IOException;

  @Override
  public long read(final ByteBuffer[] dsts, final int offset, final int length) throws IOException {
    if (offset < 0 || length < 0 || offset >= dsts.length || length >= dsts.length - offset)
      throw new IndexOutOfBoundsException();

    if (!isOpen())
      throw new ClosedChannelException();

    synchronized (readLock) {
      long r         = 0;
      long totalRead = 0;

      for (int i = offset; i < dsts.length; i++)
        r += dsts[i].remaining();

      for (int i = offset; i < dsts.length;) {
        if (!isOpen())
          throw new ClosedChannelException();

        final long rd = read(dsts[i]);

        if (rd == -1 && totalRead == 0)
          return -1;

        if (rd == -1 || rd == 0) /* FIXME is 0 right here? */
          break;

        totalRead += rd;

        if (totalRead >= r)
          break;

        if (dsts[i].remaining() == 0)
          i++;
      }

      return totalRead;
    }
  }

  @Override
  native public int write(final ByteBuffer src) throws IOException;

  @Override
  native public int write(final ByteBuffer src, final long position) throws IOException;

  @Override
  public long write(final ByteBuffer[] srcs, final int offset, final int length) throws IOException {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  protected void implCloseChannel() throws IOException {
  }

}
