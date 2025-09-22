package java.io;

public abstract class Reader implements Closeable, Readable {

  protected Object _lock;

  protected Reader() {
    this._lock = this;
  }

  protected Reader(Object lock) {
    this._lock = lock;
  }

  public void mark(int readAheadLimit) throws IOException {
    throw new IOException("Unsupported");
  }

  public boolean markSupported() {
    return false;
  }

  public int read() {
    char cbuf[] = new char[1];
    if (read(cbuf, 0, 1) == -1)
      return -1;
    return cbuf[0];
  }

  public int read(char[] cbuf) {
    return read(cbuf, 0, cbuf.length);
  }

  abstract public int read(char[] cbuf, int off, int len);

  boolean ready() {
    return false;
  }

  void reset() throws IOException {
    throw new IOException("Unsupported");
  }

  long skip(long n) throws IOException {
    if (n < 0L)
      throw new IllegalArgumentException();
    throw new IOException("TODO");
  }

  abstract public void close() throws IOException;
}
