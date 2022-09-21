package java.io;

import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetEncoder;

public class OutputStreamWriter extends Writer {

  private final OutputStream os;
  private final CharsetEncoder enc;

  public OutputStreamWriter(final OutputStream out) {
    this(out, Charset.defaultCharset());
  }

  public OutputStreamWriter(final OutputStream out, final Charset cs) {
    os = out;
    enc = cs.newEncoder();
  }

  public OutputStreamWriter(final OutputStream out, final CharsetEncoder newEnc) {
    os = out;
    enc = newEnc;
  }

  public OutputStreamWriter(final OutputStream out, final String charsetName) {
    this(out, Charset.availableCharsets().get(charsetName));
  }

  @Override
  public void close() throws IOException {
    os.close();
  }

  @Override
  public void flush() throws IOException {
    os.flush();
  }

  @Override
  void write(final char[] cbuf, final int off, final int len) throws IOException {
    final CharBuffer cb = CharBuffer.wrap(cbuf);
    os.write(enc.encode(cb).array(), off, len);
  }

  @Override
  void write(final int c) throws IOException {
    final CharBuffer cb = CharBuffer.allocate(1);
    cb.put(0, (char) c);
    os.write(enc.encode(cb).array());
  }

}
