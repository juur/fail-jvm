package java.text;

public class DecimalFormat extends NumberFormat {

  public DecimalFormat(final String pattern) {
    this(pattern, DecimalFormatSymbols.getInstance());
  }

  public DecimalFormat(final String pattern, final DecimalFormatSymbols symbols) {
    // TODO Auto-generated constructor stub
  }

  @Override
  public StringBuffer format(final double number, final StringBuffer toAppendTo, final FieldPosition pos) {
    return null;
  }

  @Override
  public StringBuffer format(final long number, final StringBuffer toAppendTo, final FieldPosition pos) {
    return null;
  }

  @Override
  public StringBuffer format(final Object obj, final StringBuffer toAppendTo, final FieldPosition pos) {
    return null;
  }

  @Override
  public Number parse(final String source, final ParsePosition parsePosition) {
    return null;
  }

  @Override
  public Object parseObject(final String source, final ParsePosition pos) {
    return null;
  }

}
