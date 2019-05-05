package java.lang;

public final class Short extends Number {

  public static final short MIN_VALUE, MAX_VALUE;

  public static final int          SIZE;
  public static final Class<Short> TYPE;

  static {
    TYPE = new Class<Short>();
    TYPE.isPrimitive = true;
    TYPE.simpleName = "short";
    TYPE.canonicalName = "short";
    TYPE.name = "short";
    MAX_VALUE = 32767;
    MIN_VALUE = -32768;
    SIZE = 16;
  }

  private short value;

  @Override
  public byte byteValue() {
    return (byte) value;
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

  @Override
  public short shortValue() {
    return value;
  }
}
