package java.lang;

public final class Character {

  public static final char MAX_VALUE, MIN_VALUE;
  public static final int  SIZE;

  public static final Class<Character> TYPE;

  static {
    TYPE = new Class<Character>();
    TYPE.isPrimitive = true;
    TYPE.simpleName = "char";
    TYPE.canonicalName = "char";
    TYPE.name = "char";
    MAX_VALUE = 65535;
    MIN_VALUE = 0;
    SIZE = 16;
  }

  public static char toLowerCase(final char c) {
    return c;
  }
}
