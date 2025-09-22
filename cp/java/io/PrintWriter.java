package java.io;

import java.nio.charset.Charset;
import java.util.Locale;

public class PrintWriter extends Writer {

  protected Writer     _out;
  private boolean      _autoFlush = false;
  private boolean      hasError;
  final private String lineSep;

  public PrintWriter(File file) throws FileNotFoundException, IOException {
    this(file, Charset.defaultCharset().name());
  }

  public PrintWriter(File file, String csn) throws FileNotFoundException, IOException {
    super();
    _out = new OutputStreamWriter(new FileOutputStream(file), csn);
    lineSep = System.getProperty("line.separator");
  }

  public PrintWriter(OutputStream out) {
    this(out, false);
  }

  public PrintWriter(OutputStream out, boolean autoFlush) {
    super(out);
    this._out = new OutputStreamWriter(out);
    this._autoFlush = autoFlush;
    lineSep = System.getProperty("line.separator");
  }

  public PrintWriter(String fileName) throws FileNotFoundException, IOException {
    this(fileName, Charset.defaultCharset().name());
  }

  public PrintWriter(String fileName, String csn) throws FileNotFoundException, IOException {
    this(new File(fileName), csn);
  }

  public PrintWriter(Writer out) {
    this(out, false);
  }

  public PrintWriter(Writer out, boolean autoFlush) {
    super(out);
    this._out = out;
    this._autoFlush = autoFlush;
    lineSep = System.getProperty("line.separator");
  }

  @Override
  public void close() throws IOException {
    _out.close();
  }

  public boolean checkError() {
    flush();
    return hasError;
  }

  protected void setError() {
    hasError = true;
  }

  protected void clearError() {
    hasError = false;
  }

  public void print(boolean b) {
    write(String.valueOf(b));
  }

  public void print(char c) {
    write(String.valueOf(c));
  }

  public void print(String s) {
    write(s == null ? "null" : s);
  }

  public void print(Object o) {
    write(String.valueOf(o));
  }

  public void println(boolean b) {
    synchronized (lock) {
      print(b);
      println();
    }
  }

  public void write(String s) {
    write(s, 0, s.length());
  }

  public void write(String s, int off, int len) {
    try {
      _out.write(s, off, len);
    } catch (IOException e) {
      setError();
    }
  }

  public void println() {
    try {
      synchronized (lock) {
        _out.write(lineSep);
        if (_autoFlush)
          flush();
      }
    } catch (IOException e) {
      setError();
    }
  }

  @Override
  public void flush() {
    try {
      _out.flush();
    } catch (IOException e) {
      setError();
    }
  }

  @Override
  public void write(char[] cbuf, int off, int len) throws IOException {
    _out.write(cbuf, off, len);
  }

  public PrintWriter printf(String format, Object... args) {
    return printf(Locale.getDefault(), format, args);
  }

  public PrintWriter printf(Locale locale, String format, Object... args) {
    return format(locale, format, args);
  }

  public PrintWriter format(String format, Object... args) {
    return format(Locale.getDefault(), format, args);
  }

  public PrintWriter format(Locale locale, String format, Object[] args) {
    // TODO Auto-generated method stub
    return this;
  }

}
