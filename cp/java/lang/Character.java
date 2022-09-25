package java.lang;

import java.io.Serializable;

public final class Character implements Serializable, Comparable<Character> {

  public static class Subset {

    private final String name;

    protected Subset(final String nName) {
      name = nName;
    }

  }

  public static final class UnicodeBlock extends Subset {

    public static final UnicodeBlock forName(final String blockName) {
      return null;
    }

    public static final UnicodeBlock of(final char c) {
      return null;
    }

    public static final UnicodeBlock of(final int codePoint) {
      return null;
    }

    private UnicodeBlock(final java.lang.String nName) {
      super(nName);
    }

  }

  public static final byte COMBINING_SPACING_MARK = 8;
  public static final byte CONNECTOR_PUNCTUATION  = 23;
  public static final byte CONTROL                = 15;
  public static final byte CURRENCY_SYMBOL        = 26;
  public static final byte DASH_PUNCTUATION       = 20;

  public static final byte DECIMAL_DIGIT_NUMBER                      = 9;
  public static final byte DIRECTIONALITY_ARABIC_NUMBER              = 6;
  public static final byte DIRECTIONALITY_BOUNDARY_NEUTRAL           = 9;
  public static final byte DIRECTIONALITY_COMMON_NUMBER_SEPARATOR    = 7;
  public static final byte DIRECTIONALITY_EUROPEAN_NUMBER            = 3;
  public static final byte DIRECTIONALITY_EUROPEAN_NUMBER_SEPARATOR  = 4;
  public static final byte DIRECTIONALITY_EUROPEAN_NUMBER_TERMINATOR = 5;
  public static final byte DIRECTIONALITY_LEFT_TO_RIGHT              = 0;
  public static final byte DIRECTIONALITY_LEFT_TO_RIGHT_EMBEDDING    = 14;
  public static final byte DIRECTIONALITY_LEFT_TO_RIGHT_OVERRIDE     = 15;
  public static final byte DIRECTIONALITY_NONSPACING_MARK            = 8;
  public static final byte DIRECTIONALITY_OTHER_NEUTRALS             = 13;
  public static final byte DIRECTIONALITY_PARAGRAPH_SEPARATOR        = 10;
  public static final byte DIRECTIONALITY_POP_DIRECTIONAL_FORMAT     = 18;
  public static final byte DIRECTIONALITY_RIGHT_TO_LEFT              = 1;
  public static final byte DIRECTIONALITY_RIGHT_TO_LEFT_ARABIC       = 2;
  public static final byte DIRECTIONALITY_RIGHT_TO_LEFT_EMBEDDING    = 16;
  public static final byte DIRECTIONALITY_RIGHT_TO_LEFT_OVERRIDE     = 17;
  public static final byte DIRECTIONALITY_SEGMENT_SEPARATOR          = 11;
  public static final byte DIRECTIONALITY_UNDEFINED                  = -1;

  public static final byte DIRECTIONALITY_WHITESPACE    = 12;
  public static final byte ENCLOSING_MARK               = 7;
  public static final byte END_PUNCTUATION              = 22;
  public static final byte FINAL_QUOTE_PUNCTUATION      = 30;
  public static final byte FORMAT                       = 16;
  public static final byte INITIAL_QUOTE_PUNCTUATION    = 29;
  public static final byte LETTER_NUMBER                = 10;
  public static final byte LINE_SEPARATOR               = 13;
  public static final byte LOWERCASE_LETTER             = 2;
  public static final byte MATH_SYMBOL                  = 25;
  public static final int  MAX_CODE_POINT               = 1114111;
  public static final char MAX_HIGH_SURROGATE           = 56319;
  public static final char MAX_LOW_SURROGATE            = 57343;
  public static final int  MAX_RADIX                    = 36;
  public static final char MAX_SURROGATE                = 57343;
  public static final char MAX_VALUE                    = 65535;
  public static final int  MIN_CODE_POINT               = 0;
  public static final char MIN_HIGH_SURROGATE           = 55296;
  public static final char MIN_LOW_SURROGATE            = 56320;
  public static final int  MIN_RADIX                    = 2;
  public static final int  MIN_SUPPLEMENTARY_CODE_POINT = 65536;
  public static final char MIN_SURROGATE                = 55296;
  public static final char MIN_VALUE                    = 0;
  public static final byte MODIFIER_LETTER              = 4;
  public static final byte MODIFIER_SYMBOL              = 27;
  public static final byte NON_SPACING_MARK             = 6;
  public static final byte OTHER_LETTER                 = 5;
  public static final byte OTHER_NUMBER                 = 11;
  public static final byte OTHER_PUNCTUATION            = 24;
  public static final byte OTHER_SYMBOL                 = 28;
  public static final byte PARAGRAPH_SEPARATOR          = 14;
  public static final byte PRIVATE_USE                  = 18;
  public static final int  SIZE                         = 16;
  public static final byte SPACE_SEPARATOR              = 12;
  public static final byte START_PUNCTUATION            = 21;
  public static final byte SURROGATE                    = 19;
  public static final byte TITLECASE_LETTER             = 3;
  public static final byte UNASSIGNED                   = 0;

  public static final byte UPPERCASE_LETTER = 1;

  public static final Class<Character> TYPE;

  static {
    TYPE = new Class<Character>();
    TYPE.isPrimitive = true;
    TYPE.simpleName = "char";
    TYPE.canonicalName = "char";
    TYPE.name = "char";
  }

  public static int getNumericValue(final int codePoint) {
    return -1;
  }

  public static int getType(final char ch) {
    return getType((int) ch);
  }

  public static int getType(final int codePoint) {
    return UNASSIGNED;
  }

  public static boolean isDigit(final char ch) {
    return isDigit((int)ch);
  }

  public static boolean isDigit(final int codePoint) {
    return false;
  }

  public static boolean isISOControl(final char ch) {
    if (ch >= 0x00 && ch <= 0x1f)
      return true;
    if (ch >= 0x7f && ch <= 0x9f)
      return true;
    return false;
  }

  public static boolean isISOControl(final int ch) {
    if (ch >= 0x00 && ch <= 0x1f)
      return true;
    if (ch >= 0x7f && ch <= 0x9f)
      return true;
    return false;
  }

  public static boolean isLetter(final char ch) {
    return isLetter((int) ch);
  }

  public static boolean isLetter(final int codePoint) {
    return false;
  }

  public static boolean isLetterOrDigit(final char ch) {
    return isLetter(ch) || isDigit(ch);
  }

  public static boolean isLetterOrDigit(final int codePoint) {
    return isLetter(codePoint) || isDigit(codePoint);
  }

  public static boolean isSpaceChar(final int codePoint) {
    /* kludge */
    switch (codePoint) {
      case 0x20:
      case 0x0a:
      case 0x0d:
        return true;
    }
    return false;
  }

  public static boolean isWhitespace(final int codePoint) {
    /*
     * TODO check 'type' is SPACE_SEPARATOR, LINE_SEPARATOR, or PARAGRAPH_SEPARATOR
     */

    switch(codePoint) {
      case 0x00a0:
      case 0x2007:
      case 0x202f:
        return false;
    }

    switch (codePoint) {
      /* begin kludge */
      case 0x20:
        /* end kludge */
      case 0x09:
      case 0x0a:
      case 0x0b:
      case 0x0c:
      case 0x0d:
      case 0x1c:
      case 0x1d:
      case 0x1e:
      case 0x1f:
        return true;
    }

    return false;
  }

  public static char toLowerCase(final char c) {
    return c; /* TODO */
  }

  public static int toLowerCase(final int c) {
    // TODO
    return c;
  }

  public static String toString(final char c) {
    return "" + c;
  }

  public static char toUpperCase(final char c) {
    return c; /* TODO */
  }

  public static int toUpperCase(final int c) {
    // TODO
    return c;
  }

  private final char val;

  public Character(final char value) {
    val = value;
  }

  public char charValue() {
    return val;
  }

  @Override
  public int compareTo(final Character o) {
    return charValue() - o.charValue();
  }

  @Override
  public boolean equals(final Object obj) {
    if (obj == null)
      return false;
    if (!(obj instanceof Character))
      return false;
    final Character c = (Character) obj;

    return c.charValue() == charValue();
  }

  @Override
  public int hashCode() {
    return charValue();
  }

  @Override
  public java.lang.String toString() {
    return "" + charValue();
  }
}
