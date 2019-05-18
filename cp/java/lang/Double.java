package java.lang;

public final class Double extends Number implements Comparable<Double> {

  public static final int MAX_EXPONENT;

  public static final double        MAX_VALUE;
  public static final int           MIN_EXPONENT;
  public static final double        MIN_NORMAL;
  public static final double        MIN_VALUE;
  public static final double        NaN;
  public static final double        NEGATIVE_INFINITY;
  public static final double        POSITIVE_INFINITY;
  public static final int           SIZE;
  public static final Class<Double> TYPE;

  static {
    TYPE = new Class<Double>();
    TYPE.isPrimitive = true;
    TYPE.name = "double";
    TYPE.simpleName = "double";
    TYPE.canonicalName = "double";
    MAX_VALUE = 1.7976931348623157E308;
    MIN_NORMAL = 2.2250738585072014E-308;
    MIN_VALUE = 4.9E-324;
    NaN = 0d / 0d;
    NEGATIVE_INFINITY = -1d / 0d;
    POSITIVE_INFINITY = 1d / 0d;
    MAX_EXPONENT = 1023;
    MIN_EXPONENT = -1022;
    SIZE = 64;
  }

  public static String toString(final long a) {
    return "<double>";
  }

  private final double value;

  public Double(final float val) {
    value = val;
  }

  public Double(final double val) {
    value = val;
  }

  @Override
  public int compareTo(final Double o) {
    return (int) (value - o.value);
  }

  @Override
  public double doubleValue() {
    return value;
  }

  @Override
  public float floatValue() {
    return (float) value;
  }

  @Override
  public int intValue() {
    return (int) value;
  }

  @Override
  public long longValue() {
    return (long) value;
  }

  public static Double valueOf(double d)
  {
	  return new Double(d);
  }

  public static Double valueOf(String s)
  {
	  double val = 0f;

	  return valueOf(val);
  }
}
