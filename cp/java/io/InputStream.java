package java.io;

public abstract class InputStream implements Closeable, Flushable {

  /**
   * @throws IOException
   */
  public int available() throws IOException
  {
    return 0;
  }

  @Override
  public void close() throws IOException {}

  @Override
  public void flush() throws IOException {}

  public void mark(final int readlimit)
  {
  }

  public boolean markSupported() {
    return false;
  }

  public abstract int read() throws IOException;

  public int read(final byte[] b) throws IOException {
    return read(b, 0, b.length);
  }

  public int read(final byte[] b, final int off, final int len) throws IOException {
    if (b == null)
      throw new NullPointerException();
    if (off < 0 || len < 0 || len > b.length - off)
      throw new IndexOutOfBoundsException();
    for (int i = 0; i < len; i++)
      b[off + i] = (byte) read();
    return len;
  }

  public void reset() throws IOException {
    throw new IOException();
  }

  public long skip(final long n) {
    long i = 0;
    do {
      try {
        if (read() == -1)
          return i;
      } catch (final IOException e) {
        return i;
      }
    } while (++i < n);

    return n;
  }
}
