package java.math;

public class BigDecimal extends Number implements Comparable<BigDecimal> {

  public static BigDecimal ONE = new BigDecimal(1, 0, 1, MathContext.UNLIMITED);

  private final int         scale;
  private final int         prec;
  private final int         value;
  private final MathContext context;

  public BigDecimal(final BigInteger val) {
    this(val, 0, MathContext.UNLIMITED);
  }

  public BigDecimal(final BigInteger unscaledVal, final int nScale) {
    this(unscaledVal, nScale, MathContext.UNLIMITED);
  }

  public BigDecimal(final BigInteger unscaledVal, final int nScale, final MathContext mc) {
    this(0, 0, 1, mc);
  }

  public BigDecimal(final char[] in) {
    this(in, 0, in.length, MathContext.UNLIMITED);
  }

  public BigDecimal(final char[] in, final int offset, final int len) {
    this(in, offset, len, MathContext.UNLIMITED);
  }

  public BigDecimal(final char[] in, final int offset, final int len, final MathContext mc) {
    this(0, 0, 1, mc);
  }

  public BigDecimal(final char[] in, final MathContext mc) {
    this(in, 0, in.length, mc);
  }

  public BigDecimal(final double val) {
    this(val, MathContext.UNLIMITED);
  }

  public BigDecimal(final double val, final MathContext mc) {
    this((int) val, 0, 1, mc);
  }

  public BigDecimal(final int val) {
    this(val, MathContext.UNLIMITED);
  }

  public BigDecimal(final int val, final MathContext mc) {
    this(val, 0, 1, mc);
  }

  public BigDecimal(final long val) {
    this(val, MathContext.UNLIMITED);
  }

  public BigDecimal(final long val, final MathContext mc) {
    this((int) val, 0, 1, mc);
  }

  public BigDecimal(final String val) {
    this(val, MathContext.UNLIMITED);
  }

  public BigDecimal(final String val, final MathContext mc) {
    this(0, 0, 1, mc);
  }

  BigDecimal(final int val, final int nScale, final int nPrec, final MathContext mc) {
    scale = nScale;
    prec = nPrec;
    value = val;
    context = mc;
  }

  @Override
  public int compareTo(final BigDecimal o) {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public double doubleValue() {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public float floatValue() {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public int intValue() {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public long longValue() {
    // TODO Auto-generated method stub
    return 0;
  }

  public int precision() {
    return prec;
  }

  public int scale() {
    return scale;
  }

}
