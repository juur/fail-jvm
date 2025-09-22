package java.util;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintStream;
import java.io.PrintWriter;
import java.io.Reader;
import java.io.Serializable;
import java.io.Writer;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;

public class Properties extends Hashtable<Object, Object> implements Serializable, Cloneable, Map<Object, Object> {

  private static final long serialVersionUID = -7657189080621360308L;
  protected Properties      _defaults;

  public Properties() {
    this(new Properties());
  }

  public Properties(final Properties defaults) {
    super();
    _defaults = defaults;
  }

  public String getProperty(final String key) {
    /*
     * 1. this 2. default property list 3. the default property list's defaults 4.
     * null
     */

    return null;
  }

  public String getProperty(final String key, final String defaultValue) {
    String ret;

    ret = getProperty(key);
    if (ret != null)
      return ret;

    return defaultValue;
  }

  public void list(final PrintStream out) {
    final Iterator<Object> it = keySet().iterator();
    while (it.hasNext()) {
      final Object o = it.next();
      out.printf("%s=%s\n", it.toString(), getProperty(o.toString()));
    }
  }

  public void list(final PrintWriter out) {
    final Iterator<Object> it = keySet().iterator();
    while (it.hasNext()) {
      final Object o = it.next();
      out.printf("%s=%s\n", it.toString(), getProperty(o.toString()));
    }
  }

  public void load(final InputStream inStream) throws IOException {
    final byte[] inByte = new byte[10000];
    inStream.read(inByte);

    Charset.defaultCharset().decode(ByteBuffer.wrap(inByte));
  }

  public void load(final Reader reader) throws IOException, IllegalArgumentException {
    final CharBuffer in = CharBuffer.allocate(100);
    reader.read(in);
    reader.close();
  }

  public void loadFromXML(final InputStream in) throws IOException, InvalidPropertiesFormatException {

  }

  public Enumeration<?> propertyNames() {
    return keys();
  }

  @Deprecated
  public void save(final OutputStream out, final String comments) {
    try {
      store(out, comments);
    } catch (final IOException e) {
      return;
    }
  }

  public Object setProperty(final String key, final String value) {
    return put(key, value);
  }

  public void store(final OutputStream out, final String comments) throws IOException {

  }

  public void store(final Writer writer, final String comments) throws IOException {

  }

  public void storeToXML(final OutputStream os, final String comment) throws IOException {

  }

  public void storeToXML(final OutputStream os, final String comment, final String encoding) throws IOException {

  }

  public Set<String> stringPropertyNames() {
    final Set<String>      ret  = new HashSet<String>();
    final Set<Object>      keys = keySet();
    final Iterator<Object> it   = keys.iterator();

    while (it.hasNext()) {
      final Object o = it.next();
      if (o instanceof String)
        ret.add((String) o);
    }

    ret.addAll(_defaults.stringPropertyNames());

    return ret;
  }
}
