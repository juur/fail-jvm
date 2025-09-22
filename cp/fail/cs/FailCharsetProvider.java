package fail.cs;

import java.nio.charset.Charset;
import java.nio.charset.spi.CharsetProvider;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

public class FailCharsetProvider extends CharsetProvider {

  private static final List<Charset> charsets;

  static {
    final List<Charset> newCharsets = new ArrayList<Charset>(10);
    newCharsets.add(ISO_8859_1.INSTANCE);
    newCharsets.add(ASCII.INSTANCE);
    newCharsets.add(UTF_16.INSTANCE);
    newCharsets.add(UTF_16LE.INSTANCE);
    newCharsets.add(UTF_16BE.INSTANCE);
    newCharsets.add(UTF_8.INSTANCE);
    newCharsets.add(CP1252.INSTANCE);
    charsets = Collections.unmodifiableList(newCharsets);
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
