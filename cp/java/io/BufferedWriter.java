package java.io;

import java.nio.CharBuffer;

public class BufferedWriter extends Writer {

  final private Writer     writer;
  final private CharBuffer buffer;
  final private String     lineSep;

  public BufferedWriter(final Writer out) {
    this(out, 512);
  }

  public BufferedWriter(final Writer out, final int sz) {
    super();
    writer = out;
    buffer = CharBuffer.allocate(sz);
    buffer.clear();
    lineSep = System.getProperty("line.separator");
  }

  @Override
  public void close() throws IOException {
    flush();
    writer.close();
  }

  public void newLine() throws IOException {
    write(lineSep);
  }

  @Override
  public void flush() throws IOException {
    writer.append(buffer);
    buffer.clear();
    writer.flush();
  }

  @Override
  public void write(final char[] cbuf, final int off, final int len) throws IOException {
    /* TODO implement buffering */
    writer.write(cbuf, off, len);

    /* total number of chars to write */
    int toBuffer = len - off;

    /* current position in cbuf */
    int pos = off;

    while (toBuffer > 0) {
      /* if the buffer is full, clear it out */
      if (buffer.remaining() == 0) {
        writer.write(buffer.array());
        buffer.clear();
      }
      int next = Math.min(toBuffer, buffer.remaining());

      buffer.put(cbuf, off + pos, next);
      toBuffer -= next;
      pos += next;
    }
  }

}
