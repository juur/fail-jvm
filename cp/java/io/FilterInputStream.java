package java.io;

public class FilterInputStream extends InputStream {

  protected InputStream in;

  protected FilterInputStream(final InputStream newIn) {
    in = newIn;
  }

  @Override
  public int available() throws IOException {
    return 1;
  }

  @Override
  public void close() throws IOException {
    in.close();
  }

  @Override
  public void mark(final int readlimit)
  {
    in.mark(readlimit);
  }

  @Override
  public boolean markSupported()
  {
    return in.markSupported();
  }

  @Override
  public int read() throws IOException {
    return in.read();
  }

  @Override
  public int read(final byte[] b) throws IOException {
    return in.read(b, 0, b.length);
  }

  @Override
  public int read(final byte[] b, final int off, final int len) throws IOException {
    return in.read(b, off, len);
  }

  @Override
  public void reset() throws IOException
  {
    in.reset();
  }
}
