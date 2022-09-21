package java.nio.charset;

public class CoderResult {

  public static final CoderResult OVERFLOW, UNDERFLOW;

  static {
    OVERFLOW = new CoderResult();
    OVERFLOW.overflow = true;
    UNDERFLOW = new CoderResult();
    UNDERFLOW.underflow = true;
  }

  public static CoderResult malformedForLength(final int length) {
    final CoderResult res = new CoderResult();
    res.malformed = true;
    res.length = length;
    return res;
  }

  public static CoderResult unmappableForLength(final int length) {
    final CoderResult res = new CoderResult();
    res.unmappable = true;
    res.length = length;
    return res;
  }

  private int length;

  private boolean malformed, overflow, underflow, unmappable;

  CoderResult() {

  }

  public boolean isError() {
    return isMalformed() || isUnmappable();
  }

  public boolean isMalformed() {
    return malformed;
  }

  public boolean isOverflow() {
    return overflow;
  }

  public boolean isUnderflow() {
    return underflow;
  }

  public boolean isUnmappable() {
    return unmappable;
  }

  public int length() {
    return length;
  }

  /**
   * @throws CharacterCodingException
   */
  public void throwException() throws CharacterCodingException {

  }

  @Override
  public String toString() {
    return "Overflow: " + isOverflow() + ", Underflow: " + isUnderflow();
  }
}
