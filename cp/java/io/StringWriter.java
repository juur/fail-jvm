package java.io;

public class StringWriter extends Writer {

  private StringBuffer sb;

  @Override
  public void close() throws IOException {}

  @Override
  public void flush() throws IOException {}

  @Override
  public String toString() {
    return sb.toString();
  }

  @Override
  void write(final char[] cbuf, final int off, final int len) throws IOException {
    sb.append(cbuf, off, len);
  }

  @Override
  void write(final String str, final int off, final int len) throws IOException {
    sb.append(str, off, len);
  }

}
