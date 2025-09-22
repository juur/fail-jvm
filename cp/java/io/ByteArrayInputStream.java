package java.io;

public class ByteArrayInputStream extends InputStream {

  protected byte[] buf;
  protected int    count;
  protected int    mark;
  protected int    pos;

  public ByteArrayInputStream(byte[] buf) {
    this.buf = buf;
    this.pos = 0;
    this.count = buf.length;
  }

  public ByteArrayInputStream(byte[] buf, int offset, int length) {
    this.buf = buf;
    this.count = offset + length;
    this.pos = offset;
  }

  public synchronized int read() {
    if (this.pos > this.count)
      return -1;

    return this.buf[pos++];
  }
}
