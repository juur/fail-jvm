package java.nio.channels;

import java.io.FileDescriptor;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.spi.ClosedChannelException;

public class ImplFileChannel extends FileChannel {

  private final FileDescriptor fdObj;
  private final Object         readLock, writeLock;

  public ImplFileChannel() {
    throw new IllegalStateException();
  }

  public ImplFileChannel(final FileDescriptor fd) {
    if (fd == null)
      throw new NullPointerException();
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
  public int read(final ByteBuffer dst) throws IOException {
    if (dst.isDirect())
      return nativeRead(dst);

    int ret = 0;

    while (dst.capacity() > 0) {
      final int rd = nativeRead(position());
      if (rd == -1)
        break;
      position(position() + 1);
      ret++;
      dst.put((byte) rd);
    }

    return ret;
  }

  @Override
  public long read(final ByteBuffer dst, final long position) throws IOException {
    if (dst.isDirect())
      return nativeRead(dst, position);

    int ret = 0;

    while (dst.capacity() > 0) {
      final int rd = nativeRead(position);
      if (rd == -1)
        break;
      position(position() + 1);
      ret++;
      dst.put((byte) rd);
    }

    return ret;
  }

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
  public int write(final ByteBuffer src) throws IOException {
    if (src.isDirect())
      return nativeWrite(src);

    int ret = 0;
    int rd  = 0;

    while (src.remaining() > 0) {
      rd = nativeWrite(src.get(), position());
      if (rd == -1)
        break;
      position(position() + 1);
      ret++;

    }

    return ret;
  }

  @Override
  public int write(final ByteBuffer src, final long position) throws IOException {
    if (src.isDirect())
      return nativeWrite(src, position);

    int ret = 0;

    while (src.remaining() > 0) {
      final int rd = nativeWrite(src.get(), position);
      if (rd == -1)
        break;
      position(position() + 1);
      ret++;
    }

    return ret;
  }

  @Override
  public long write(final ByteBuffer[] srcs, final int offset, final int length) throws IOException {
    // TODO Auto-generated method stub
    return 0;
  }

  native private int nativeRead(final ByteBuffer dst) throws IOException;

  native private long nativeRead(final ByteBuffer dst, final long position) throws IOException;

  native private int nativeRead(final long position) throws IOException;

  native private int nativeWrite(final byte b, final long position) throws IOException;

  native private int nativeWrite(final ByteBuffer src) throws IOException;

  native private int nativeWrite(final ByteBuffer src, final long position) throws IOException;

  @Override
  protected void implCloseChannel() throws IOException {
    fdObj.close();
  }

}
