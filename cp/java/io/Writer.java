package java.io;


public abstract class Writer implements Appendable, Closeable, Flushable {

  protected Object lock;

  protected Writer() {
    this(new Object());
  }

  protected Writer(final Object newLock) {
    lock = newLock;
  }

  @Override
  public Writer append(final char c) throws IOException {
    write(c);
    return this;
  }

  @Override
  public Writer append(final CharSequence csq) throws IOException {
    if (csq == null)
      write("null");
    else
      write(csq.toString());
    return this;
  }

  @Override
  public Writer append(final CharSequence csq, final int start, final int end) throws IOException {
    if (csq == null)
      write("null", start, end);
    else
      write(csq.subSequence(start, end).toString());
    return this;
  }

  @Override
  public abstract void close() throws IOException;

  @Override
  public abstract void flush() throws IOException;

  void write(final char[] cbuf) throws IOException {
    write(cbuf, 0, cbuf.length);
  }

  abstract void write(char[] cbuf, int off, int len) throws IOException;

  /**
   * @throws IOException
   */
  void write(final int c) throws IOException {
    final char[] cbuf = new char[1];
    cbuf[0] = (char) c;
    write(cbuf);
  }

  void write(final String str) throws IOException {
    write(str.toCharArray());
  }

  void write(final String str, final int off, final int len) throws IOException {
    write(str.toCharArray(), off, len);
  }
}
