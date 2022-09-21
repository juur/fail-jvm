package java.io;

import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.util.Formatter;
import java.util.Locale;

public class PrintStream extends FilterOutputStream implements Appendable {

  private boolean   error;
  private Formatter formatter;

  public PrintStream(final OutputStream outStream) {
    super(outStream);
    error = false;
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
  public Appendable append(final CharSequence csq, final int start, final int end)
    throws IOException {
    write(Charset.defaultCharset().encode(CharBuffer.wrap(csq.subSequence(start, end))).array());
    return this;
  }

  public boolean checkError() {
    return error;
  }

  protected void clearError() {
    error = false;
  }

  public PrintStream format(final Locale l, final String format, final Object... args) {
    synchronized (this) {
      if (formatter == null) {
        formatter = new Formatter(this);
      }
      formatter.format(l, format, args);
    }
    return this;
  }

  public PrintStream format(final String format, final Object... args) {
    return format(Locale.getDefault(), format, args);
  }

  public void print(final boolean b) {
    try {
      write(String.valueOf(b).getBytes());
    } catch (final IOException e) {
      setError();
    }
    clearError();
  }

  public void print(final char c) {
    try {
      append(c); // FIXME should be default encoding
    } catch (final IOException e) {
      setError();
    }
    clearError();
  }

  public void print(final char[] s) {
    try {
      append(new String(s));
    } catch (final IOException e) {
      setError();
    }
    clearError();
  }

  public void print(final double b) {
    try {
      write(String.valueOf(b).getBytes());
    } catch (final IOException e) {
      setError();
    }
    clearError();
  }

  public void print(final float b) {
    try {
      write(String.valueOf(b).getBytes());
    } catch (final IOException e) {
      setError();
    }
    clearError();
  }

  public void print(final int b) {
    try {
      write(String.valueOf(b).getBytes());
    } catch (final IOException e) {
      setError();
    }
    clearError();
  }

  public void print(final long b) {
    try {
      write(String.valueOf(b).getBytes());
    } catch (final IOException e) {
      setError();
    }
    clearError();
  }

  public void print(final Object b) {
    print(String.valueOf(b));
  }

  public void print(final String string) {
    try {
      append(string);
    } catch (final IOException e) {
      setError();
    }
    clearError();
  }

  public PrintStream printf(final Locale l, final String format, final Object... args) {
    return format(l, format, args);
  }

  public PrintStream printf(final String format, final Object... args)
  {
    return format(format, args);
  }

  public void println() {
    print("\n");
  }

  public void println(final String arg) {
    print(arg);
    println();
  }

  protected void setError() {
    error = true;
  }
}
