package java.lang;

public final class Byte extends Number {

  public static final byte MIN_VALUE, MAX_VALUE;

  public static final int         SIZE;
  public static final Class<Byte> TYPE;
  static {
    TYPE = new Class<Byte>();
    TYPE.isPrimitive = true;
    TYPE.simpleName = "byte";
    TYPE.name = "byte";
    TYPE.canonicalName = "byte";
    MAX_VALUE = 127;
    MIN_VALUE = -128;
    SIZE = 8;
  }

  public static int parseByte(final String str) {
    return parseByte(str, 10);
  }

  public static int parseByte(final String str, final int radix) {
    return (int) Number.atoi(str.toCharArray(), radix, MAX_VALUE, MIN_VALUE);
  }

  private byte value;

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
