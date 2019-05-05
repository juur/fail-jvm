package java.io;

import java.nio.CharBuffer;
import java.nio.charset.Charset;

public class PrintStream extends FilterOutputStream implements Appendable, Closeable {

  public PrintStream(final OutputStream out) {
    super(out);
  }

  @Override
  public Appendable append(final char c) throws IOException {
    write(Charset.defaultCharset().encode(CharBuffer.wrap(new char[] { c })).array());
    return this;
  }

  @Override
  public Appendable append(final CharSequence csq) throws IOException {
    write(Charset.defaultCharset().encode(CharBuffer.wrap(csq)).array());
    return this;
  }

  @Override
  public Appendable append(final CharSequence csq, final int start, final int end) throws IOException {
    write(Charset.defaultCharset().encode(CharBuffer.wrap(csq.subSequence(start, end))).array());
    return this;
  }

  public void print(final String arg) {
    if (arg == null)
      throw new NullPointerException("NULL passed to println");
    try {
      write(arg.getBytes());
    } catch (final IOException e) {
      return;
    }
  }

  public void println() {
    try {
      write("\n".getBytes());
    } catch (final IOException e) {
      return;
    }
  }

  public void println(final String arg) {
    print(arg);
    println();
  }
}
