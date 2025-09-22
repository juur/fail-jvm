package java.io;

public abstract class FilterWriter extends Writer {

  protected Writer _out;

  protected FilterWriter(Writer out) {
    super();
    if (out == null)
      throw new NullPointerException();
    this._out = out;
  }

  @Override
  public void close() throws IOException {
    _out.close();
  }

  @Override
  public void flush() throws IOException {
    _out.flush();
  }

  @Override
  public void write(char[] cbuf, int off, int len) throws IOException {
    _out.write(cbuf, off, len);
  }

}
