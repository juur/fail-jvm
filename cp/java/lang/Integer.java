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

  public static int parseInt(final String str) {
    return parseInt(str, 10);
  }

  public static int parseInt(final String str, final int radix) {
    return (int) Number.atoi(str.toCharArray(), radix, MAX_VALUE, MIN_VALUE);
  }

  public static int reverseBytes(final int i) {
    return i&0xff000000>>24|i&0xff0000>>8|i&0xff00<<8|i&0xff<<24;
  }

  public static String toString(final int val) {
    return new String(Number.itoa(val, 10));
  }

  public static Integer valueOf(final int val) {
    return new Integer(val);
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
