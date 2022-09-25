package fail.cs;

import java.nio.charset.Charset;
import java.nio.charset.spi.CharsetProvider;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

public class SpodCharsetProvider extends CharsetProvider {

  private static final List<Charset> charsets;

  static {
    charsets = new ArrayList<Charset>(10);
    charsets.add(ISO_8859_1.INSTANCE);
    charsets.add(ASCII.INSTANCE);
    charsets.add(UTF_16.INSTANCE);
    charsets.add(UTF_16LE.INSTANCE);
    charsets.add(UTF_16BE.INSTANCE);
    charsets.add(UTF_8.INSTANCE);
    charsets.add(CP1252.INSTANCE);
  }

  @Override
  public Charset charsetForName(final String charsetName) {
    for (final Charset cs : charsets)
      if (cs.name().equals(charsetName))
        return cs;
    return null;
  }

  @Override
  public Iterator<Charset> charsets() {
    return charsets.iterator();
  }

}
