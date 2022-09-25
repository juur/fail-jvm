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

  public static native int floatToRawIntBits(float value);

  public static native float intBitsToFloat(int bits);

  public static String toString(final float a) {
    if (isNaN(a))
      return "NaN";

    final Float         f  = a;
    final StringBuilder sb = new StringBuilder();

    if (f.sign != 0)
      sb.append("-");
    sb.append(f.sig);
    sb.append("e");
    sb.append(f.exp);
    return sb.toString();
  }

  public static Float valueOf(final float f) {
    return new Float(f);
  }

  public static Float valueOf(final String s) {
    final float val = 0f;

    return valueOf(val);
  }

  private static boolean isInfinity(final float a) {
    return a == NEGATIVE_INFINITY || a == POSITIVE_INFINITY;
  }

  private static boolean isNaN(final float a) {
    return a == NaN;
  }

  private final float value;
  private final int   raw;
  private final int   sign;
  private final int   exp;
  private final int   sig;

  public Float(final float val) {
    value = val;

    raw = floatToRawIntBits(val);

    sign = (0x80000000 & raw) >> 31;
    exp = (0x7f800000 & raw) >> 30;
    sig = 0x007fffff & raw;
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
