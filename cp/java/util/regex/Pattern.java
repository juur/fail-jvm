package java.util.regex;

import java.io.Serializable;

public final class Pattern implements Serializable {

  public static final int CANON_EQ         = 128;
  public static final int CASE_INSENSITIVE = 2;
  public static final int COMMENTS         = 4;
  public static final int DOTALL           = 32;
  public static final int LITERAL          = 16;
  public static final int MULTILINE        = 8;
  public static final int UNICODE_CASE     = 64;
  public static final int UNIX_LINES       = 1;
  
  private static final long serialVersionUID = 1L;

  public static Pattern compile(final String regex) {
    return compile(regex, 0);
  }

  public static Pattern compile(final String regex, final int flags) {
    return new Pattern(regex, flags);
  }

  public static boolean matches(final String regex, final CharSequence input) {
    return false;
  }

  public static String quote(final String s) {
    return "";
  }

  private final String origRegex;
  private final int    origFlags;

  public Pattern(final String regex, final int flags) {
    origRegex = regex;
    origFlags = flags;
  }

  public int flags() {
    return origFlags;
  }

  public Matcher matcher(final CharSequence input) {
    return new Matcher(this, input);
  }

  public String pattern() {
    return origRegex;
  }

  public String[] split(final CharSequence input) {
    return split(input, 0);
  }

  public String[] split(final CharSequence input, final int limit) {
    return new String[0];
  }

  @Override
  public String toString() {
    return pattern();
  }
}
