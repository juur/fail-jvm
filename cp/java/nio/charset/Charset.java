package java.nio.charset;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.spi.CharsetProvider;
import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Locale;
import java.util.Set;
import java.util.SortedMap;
import java.util.TreeMap;

public abstract class Charset implements Comparable<Charset> {

  private static final CharsetProvider defaultProvider;

  static {
    defaultProvider = new fail.cs.FailCharsetProvider();
  }

  public static SortedMap<String, Charset> availableCharsets() {
    final SortedMap<String, Charset> ret = new TreeMap<String, Charset>();
    final Iterator<Charset>          it  = defaultProvider.charsets();

    while (it.hasNext()) {
      final Charset c       = it.next();
      final String  names[] = (String[]) c.aliases().toArray();

      for (final String name : names)
        ret.put(name, c);
    }

    return ret;
  }

  public static Charset defaultCharset() {
    return fail.cs.UTF_8.INSTANCE;
  }

  public static Charset forName(final String charsetName) {
    return defaultProvider.charsetForName(charsetName);
  }

  static boolean isSupported(final String charsetName) {
    return defaultProvider.charsetForName(charsetName) != null;
  }

  private final String          name;
  private final HashSet<String> aliasesSet;

  protected Charset(final String canonicalName, final String[] aliases) {
    name = canonicalName;
    aliasesSet = new HashSet<String>();
    for (final String str : aliases)
      aliasesSet.add(str);
  }

  public final Set<String> aliases() {
    return Collections.unmodifiableSet(aliasesSet);
  }

  public boolean canEncode() {
    return true;
  }

  @Override
  public int compareTo(final Charset o) {
    return name().compareTo(o.name());
  }

  public abstract boolean contains(Charset cs);

  public final CharBuffer decode(final ByteBuffer cb) throws CharacterCodingException {
    return newDecoder().onMalformedInput(CodingErrorAction.REPLACE)
      .onUnmappableCharacter(CodingErrorAction.REPLACE)
      .decode(cb);
  }

  public String displayName() {
    return name;
  }

  public String displayName(final Locale locale) {
    return name;
  }

  public final ByteBuffer encode(final CharBuffer cb) {
    return newEncoder().onMalformedInput(CodingErrorAction.REPLACE)
      .onUnmappableCharacter(CodingErrorAction.REPLACE)
      .encode(cb);
  }

  public final ByteBuffer encode(final String str) {
    return encode(CharBuffer.wrap(str));
  }

  public boolean isRegistered() {
    if (name.startsWith("X-"))
      return false;
    if (name.startsWith("x-"))
      return false;
    return true;
  }

  public String name() {
    return name;
  }

  public abstract CharsetDecoder newDecoder();

  public abstract CharsetEncoder newEncoder();

  @Override
  public String toString() {
    return name();
  }
}
