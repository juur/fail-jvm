package java.util;

import java.io.*;
import java.nio.charset.Charset;

public final class Formatter implements Closeable, Flushable {

  public static enum BigDecimalLayoutForm {
    SCIENTIFIC, DECIMAL_FLOAT
  }

  private final Locale     locale;
  private final Appendable appendable;
  private final Charset    charset;
  private IOException      ioEx;

  public Formatter() {
    this(new StringBuilder());
  }

  public Formatter(final Appendable a) {
    this(a, Locale.getDefault());
  }

  public Formatter(final Appendable a, final Locale l) {
    this(l, a);
  }

  public Formatter(final Appendable a, final String csn, final Locale l) throws UnsupportedEncodingException {
    this(l, csn, a);
  }

  public Formatter(final File file) throws IOException {
    this(new FileOutputStream(file));
  }

  public Formatter(final File file, final String csn) throws IOException, UnsupportedEncodingException {
    this(file, csn, Locale.getDefault());
  }

  public Formatter(final File file, final String csn, final Locale l) throws IOException, UnsupportedEncodingException {
    this(new FileOutputStream(file), csn, l);
  }

  public Formatter(final Locale l) {
    this(new StringBuilder(), l);
  }

  public Formatter(final OutputStream os) {
    this(Locale.getDefault(), new BufferedWriter(new OutputStreamWriter(os)));
  }

  public Formatter(final OutputStream os, final String csn) throws UnsupportedEncodingException {
    this(new BufferedWriter(new OutputStreamWriter(os)), csn, Locale.getDefault());
  }

  public Formatter(final OutputStream os, final String csn, final Locale l) throws UnsupportedEncodingException {
    this(l, csn, new PrintStream(os, false, csn));
  }

  public Formatter(final PrintStream ps) {
    this(Locale.getDefault(), ps);
  }

  public Formatter(final String fileName) throws IOException {
    this(new File(fileName));
  }

  public Formatter(final String fileName, final String csn) throws IOException, UnsupportedEncodingException {
    this(fileName, csn, Locale.getDefault());
  }

  public Formatter(final String fileName, final String csn, final Locale l)
    throws IOException, UnsupportedEncodingException {
    this(new File(fileName), csn, l);
  }

  private Formatter(final Locale newLocale, final Appendable newAppendable) {
    locale = newLocale == null ? Locale.getDefault() : newLocale;
    appendable = newAppendable == null ? new StringBuilder() : newAppendable;
    charset = Charset.defaultCharset();
  }

  private Formatter(final Locale newLocale, final Charset newCharset, final Appendable newAppendable)
    throws UnsupportedEncodingException {
    locale = newLocale == null ? Locale.getDefault() : newLocale;
    appendable = newAppendable == null ? new StringBuilder() : newAppendable;
    charset = newCharset == null ? Charset.defaultCharset() : newCharset;

    if (charset == null)
      throw new UnsupportedEncodingException();
  }

  private Formatter(final Locale newLocale, final String newCharset, final Appendable newAppendable)
    throws UnsupportedEncodingException {
    this(newLocale, Charset.forName(newCharset), newAppendable);
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
    final int arg = 0;

    try {
      for (final char c : format.toCharArray())
        ;
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
