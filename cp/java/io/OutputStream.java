package java.io;

import java.util.Arrays;

public abstract class OutputStream implements Closeable, Flushable {

  @Override
  public void close() throws IOException {}

  @Override
  public void flush() throws IOException {}

  public abstract void write(byte[] b) throws IOException;

  public void write(final byte[] b, final int off, final int len) throws IOException {
    if (off == 0 && len == b.length)
      write(b);
    else {
      if (off < 0 || len < 0 || off + len > b.length)
        throw new IndexOutOfBoundsException();

      write(Arrays.copyOfRange(b, off, off + len));
    }
  }

  public abstract void write(int b) throws IOException;
}
