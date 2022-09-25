package java.io;

public class BufferedWriter extends Writer {

  final private Writer writer;

  public BufferedWriter(final Writer out) {
    this(out, 512);
  }

  public BufferedWriter(final Writer out, final int sz) {
    writer = out;
  }

  @Override
  public void close() throws IOException {
    writer.close();
  }

  @Override
  public void flush() throws IOException {
    writer.flush();
  }

  @Override
  void write(final char[] cbuf, final int off, final int len) throws IOException {
    writer.write(cbuf, off, len);
  }

}
