package java.util;

import java.io.Closeable;
import java.io.File;
import java.io.FileOutputStream;
import java.io.Flushable;
import java.io.IOException;
import java.io.OutputStream;
import java.io.PrintStream;
import java.nio.charset.Charset;

public final class Formatter implements Closeable, Flushable {

  public static enum BigDecimalLayoutForm {
    SCIENTIFIC, DECIMAL_FLOAT
  }

  private Locale      locale;
  private Appendable  appendable;
  private Charset     charset;
  private IOException ioEx;

  public Formatter() {}

  public Formatter(final Appendable a) {
    this(a, Locale.getDefault());
  }

  public Formatter(final Appendable a, final Locale l) {
    this(l);
    if (a == null)
      appendable = new StringBuilder();
    else
      appendable = a;
  }

  public Formatter(final File file) throws IOException {
    this(file, Charset.defaultCharset().displayName(), Locale.getDefault());
  }

  public Formatter(final File file, final String csn) throws IOException {
    this(file, csn, Locale.getDefault());
  }

  public Formatter(final File file, final String csn, final Locale l) throws IOException {
    this(new FileOutputStream(file), csn, l);
  }

  public Formatter(final Locale l) {
    locale = l;
    if (charset == null)
      charset = Charset.defaultCharset();
  }

  public Formatter(final OutputStream os) {
    this(os, Charset.defaultCharset().displayName(), Locale.getDefault());
  }

  public Formatter(final OutputStream os, final String csn) {
    this(os, csn, Locale.getDefault());
  }

  public Formatter(final OutputStream os, final String csn, final Locale l) {
    this(new PrintStream(os), csn, l);
  }

  public Formatter(final PrintStream ps) {
    this(Locale.getDefault());
    appendable = ps;
  }

  private Formatter(final PrintStream ps, final String csn, final Locale l) {
    this(l);
    appendable = ps;
  }

  public Formatter(final String fileName) throws IOException {
    this(fileName, Charset.defaultCharset().displayName(), Locale.getDefault());
  }

  public Formatter(final String fileName, final String csn) throws IOException {
    this(fileName, csn, Locale.getDefault());
  }

  public Formatter(final String fileName, final String csn, final Locale l) throws IOException {
    this(new File(fileName), csn, l);
  }

  @Override
  public void close() throws IOException {
    if (appendable instanceof Closeable)
      ((Closeable) appendable).close();
  }

  @Override
  public void flush() throws IOException {
    if (appendable instanceof Flushable)
      ((Flushable) appendable).flush();
  }

  public Formatter format(final Locale l, final String format, final Object... args) {
    try {
      appendable.append("\n");
    } catch (final IOException e) {
      ioEx = e;
    }
    return this;
  }

  public Formatter format(final String format, final Object... args) {
    return format(locale, format, args);
  }

  public IOException ioException() {
    return ioEx;
  }

  public Locale locale() {
    return locale;
  }

  public Appendable out() {
    return appendable;
  }

  @Override
  public String toString() {
    return appendable.toString();
  }

}
