package java.io;

public class StringWriter extends Writer {

  final private StringBuffer sb;

  public StringWriter() {
    super();
    sb = new StringBuffer();
  }

  public StringWriter(int initialSize) {
    super();
    sb = new StringBuffer(initialSize);
  }

  @Override
  public void close() throws IOException {}

  @Override
  public void flush() throws IOException {}

  @Override
  public String toString() {
    return getBuffer().toString();
  }

  @Override
  public void write(final char[] cbuf, final int off, final int len) throws IOException {
    getBuffer().append(cbuf, off, len);
  }

  public StringBuffer getBuffer() {
    return sb;
  }
}
