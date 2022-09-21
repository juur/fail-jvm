package java.math;

import java.io.Serializable;

public class MathContext implements Serializable {

  private static final long serialVersionUID = 1L;

  public static final MathContext UNLIMITED = new MathContext(0, RoundingMode.HALF_UP);
  public static final MathContext DECIMAL32 = new MathContext(7, RoundingMode.HALF_EVEN);
  public static final MathContext DECIMAL64 = new MathContext(16, RoundingMode.HALF_EVEN);
  public static final MathContext DECIMAL128 = new MathContext(34, RoundingMode.HALF_EVEN);
  
  private final int precision;
  private final RoundingMode roundingMode;
  
  public MathContext(int setPrecision) {
    this(setPrecision, RoundingMode.HALF_UP);
  }
  
  public MathContext(int setPrecision, RoundingMode setRoundingMode) {
    super();
    this.precision = setPrecision;
    this.roundingMode = setRoundingMode;
  }

  public int getPrecision() {
    return precision;
  }

  public RoundingMode getRoundingMode() {
    return roundingMode;
  }
  
  
  
}
