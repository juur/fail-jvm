package java.io;

public class BufferedInputStream extends FilterInputStream {

  /* TODO: buffering */

  protected byte[] buf;
  protected int    count;
  protected int    marklimit;
  protected int    markpos;
  protected int    pos;

  public BufferedInputStream(final InputStream newIn) {
    this(newIn, 1024);
  }

  public BufferedInputStream(final InputStream newIn, final int size) {
    super(newIn);
    buf = new byte[size];
    count = 0;
    marklimit = -1;
    pos = 0;
  }

  @Override
  public int available() throws IOException {
    return count - pos + in.available();
  }

  @Override
  public void mark(final int readlimit) {
    marklimit = readlimit; /* FIXME is this right? */
  }

  @Override
  public boolean markSupported() {
    return true;
  }

  @Override
  public void reset() throws IOException {
    if (markpos == -1)
      throw new IOException();
    pos = markpos;
  }

}
