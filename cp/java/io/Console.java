package java.io;


public final class Console implements Flushable {

  @Override
  public void flush() throws IOException {}

  public Console format(final String fmt, final Object... args) {
    return this;
  }

  public Console printf(final String format, final Object... args) {
    return format(format, args);
  }

  public Reader reader() {
    return null;
  }

  public PrintWriter writer() {
    return null;
  }

}
