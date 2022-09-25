package java.io;

public class FilterOutputStream extends OutputStream {

  protected OutputStream out;

  public FilterOutputStream(final OutputStream newOut) {
    super();
    out = newOut;
  }

  @Override
  public void close() throws IOException {
    flush();
    out.close();
  }

  @Override
  public void flush() throws IOException {
    out.flush();
  }

  @Override
  public void write(final byte[] b) throws IOException {
    write(b, 0, b.length);
  }

  @Override
  public void write(final byte[] b, final int off, final int len) throws IOException {
    out.write(b, off, len);
  }

  @Override
  public void write(final int b) throws IOException {
    out.write(b);
  }
}
