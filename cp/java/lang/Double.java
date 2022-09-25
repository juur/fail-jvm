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

  public static native long doubleToLongBits(double value);

  public static native long doubleToRawLongBits(double value);

  public static native double longBitsToDouble(long bits);

  public static String toString(final double a) {
    System.out.println("Double.toString(" + doubleToRawLongBits(a) + ")");
    if (isNaN(a))
      return "NaN";

    final Double        d  = a;
    final StringBuilder sb = new StringBuilder();

    if (d.sign != 0)
      sb.append("-");
    sb.append(d.sig);
    sb.append("e");
    sb.append(d.exp);
    return sb.toString();

  }

  public static Double valueOf(final double d) {
    return new Double(d);
  }

  public static Double valueOf(final String s) {
    final double val = 0d;

    return valueOf(val);
  }

  private static boolean isInfinity(final double a) {
    return a == NEGATIVE_INFINITY || a == POSITIVE_INFINITY;
  }

  private static boolean isNaN(final double a) {
    return a == NaN;
  }

  private final double value;
  private final long   raw;
  private final int    sign;
  private final int    exp;
  private final long   sig;

  public Double(final double val) {
    value = val;

    raw = doubleToRawLongBits(val);
    System.out.println("raw:" + raw);

    sign = (int) ((0x8000000000000000L & raw) >> 63);

    exp = (int) ((0x7ff0000000000000L & raw) >> 62);
    System.out.println("exp:" + exp);
    sig = 0x000fffffffffffffL & raw;
  }

  public Double(final float val) {
    this((double) val);
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

  public boolean isInfinity() {
    return isInfinity(value);
  }

  public boolean isNaN() {
    return isNaN(value);
  }

  @Override
  public long longValue() {
    return (long) value;
  }

  @Override
  public String toString() {
    return toString(value);
  }
}
