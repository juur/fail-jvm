package java.lang;

public final class Float extends Number implements Comparable<Float> {

  public static final int MAX_EXPONENT;

  public static final float        MAX_VALUE;
  public static final int          MIN_EXPONENT;
  public static final float        MIN_NORMAL;
  public static final float        MIN_VALUE;
  public static final float        NaN;
  public static final float        NEGATIVE_INFINITY;
  public static final float        POSITIVE_INFINITY;
  public static final int          SIZE;
  public static final Class<Float> TYPE;

  static {
    TYPE = new Class<Float>();
    TYPE.isPrimitive = true;
    TYPE.name = "float";
    TYPE.simpleName = "float";
    TYPE.canonicalName = "float";
    MAX_VALUE = 3.4028234663852886E38f;
    MIN_NORMAL = 1.1754943508222875E-38f;
    MIN_VALUE = 1.401298464324817E-45f;
    NaN = 0f / 0f;
    NEGATIVE_INFINITY = -1f / 0f;
    POSITIVE_INFINITY = 1f / 0f;
    MAX_EXPONENT = 127;
    MIN_EXPONENT = -126;
    SIZE = 32;
  }

  public static String toString(final long a) {
    return "<float>";
  }

  private final float value;

  public Float(final float val) {
    value = val;
  }

  @Override
  public int compareTo(final Float o) {
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
    return (long) value;
  }

}
