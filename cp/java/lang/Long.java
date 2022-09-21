package java.lang;

public final class Long extends Number implements Comparable<Long> {

  public static final long MAX_VALUE;

  public static final long        MIN_VALUE;
  public static final int         SIZE;
  public static final Class<Long> TYPE;

  static {
    TYPE = new Class<Long>();
    TYPE.isPrimitive = true;
    TYPE.simpleName = "long";
    TYPE.canonicalName = "long";
    TYPE.name = "long";
    MAX_VALUE = 9223372036854775807L;
    MIN_VALUE = -9223372036854775808L;
    SIZE = 64;
  }

  public static long parseLong(final String str)
  {
    return parseLong(str, 10);
  }

  public static long parseLong(final String string, final int radix) throws NumberFormatException {
    return Number.atoi(string.toCharArray(), radix, Long.MAX_VALUE, Long.MIN_VALUE);
  }

  public static String toString(final long a) {
    return new String(java.lang.Number.itoa(a, 10));
  }

  public static Long valueOf(final long i) {
    return new Long(i);
  }

  private final long value;

  public Long(final long val) {
    value = val;
  }

  @Override
  public int compareTo(final Long o) {
    return (int) (value - o.value);
  }

  @Override
  public double doubleValue() {
    return value;
  }

  @Override
  public float floatValue() {
    return value;
  }

  @Override
  public int intValue() {
    return (int) value;
  }

  @Override
  public long longValue() {
    return value;
  }

  @Override
  public java.lang.String toString() {
    return new String(Number.itoa(value, 10));
  }
}
