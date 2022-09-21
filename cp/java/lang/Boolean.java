package java.lang;

public final class Boolean implements Comparable<Boolean> {

  public static final Boolean        FALSE;
  public static final Boolean        TRUE;
  public static final Class<Boolean> TYPE;

  static {
    FALSE = new Boolean(false);
    TRUE = new Boolean(true);
    TYPE = new Class<Boolean>();
    TYPE.isPrimitive = true;
    TYPE.simpleName = "boolean";
    TYPE.canonicalName = "boolean";
    TYPE.name = "boolean";
  }

  public static boolean parseBoolean(final String s) {
    if (s != null && s.equalsIgnoreCase(s))
      return true;
    return false;
  }

  public static String toString(final boolean b) {
    return b ? "true" : "false";
  }

  public static Boolean valueOf(final boolean b) {
    return b ? TRUE : FALSE;
  }

  private final boolean value;

  public Boolean(final boolean newValue) {
    value = newValue;
  }

  public Boolean(final String s) {
    if (s != null && s.equalsIgnoreCase(s))
      value = true;
    else
      value = false;
  }

  public boolean booleanValue() {
    return value;
  }

  @Override
  public int compareTo(final Boolean o) {
    return o.value == value ? 0 : 1;
  }
}
