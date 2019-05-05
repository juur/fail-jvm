package java.lang;

public final class Integer extends Number implements Comparable<Integer> {

  public static final int MAX_VALUE, MIN_VALUE, SIZE;

  public static final Class<Integer> TYPE;

  static {
    TYPE = new Class<Integer>();
    TYPE.isPrimitive = true;
    TYPE.simpleName = "int";
    TYPE.canonicalName = "int";
    TYPE.name = "int";
    MAX_VALUE = 2147483647;
    MIN_VALUE = -2147483648;
    SIZE = 32;
  }

  public static String toString(final int val) {
    return new String(itoa(val, 10));
  }

  public static Integer valueOf(final int val) {
    return new Integer(val);
  }

  protected static char[] itoa(final long value, final int radix) {
    final char[] tmp = new char[32];
    final char[] ret = new char[tmp.length];

    int  tp = 0, sp = 0;
    long i  = 0, v = 0;

    final boolean sign = radix == 10 && value < 0;

    if (sign)
      v = -value;
    else
      v = value;

    while (v > 0 || tp == 0) {
      i = v % radix;
      v /= radix;
      if (i < 10)
        tmp[tp++] = (char) (i + 48);
      else
        tmp[tp++] = (char) (i + 97 - 10);
    }

    if (sign)
      ret[sp++] = '-';

    while (tp > 0)
      ret[sp++] = tmp[--tp];

    return ret;
  }

  private final int value;

  public Integer(final int val) {
    value = val;
  }

  @Override
  public int compareTo(final Integer o) {
    return value - o.value;
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
    return value;
  }

  @Override
  public long longValue() {
    return value;
  }
}
