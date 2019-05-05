package java.text;

import java.math.RoundingMode;
import java.util.Currency;
import java.util.Locale;

public abstract class NumberFormat extends Format {

  public static class Field extends Format.Field {

    public static final Field CURRENCY, DECIMAL_SEPARATOR, EXPONENT, EXPONENT_SIGN, EXPONENT_SYMBOL;
    public static final Field FRACTION, GROUPING_SEPARATOR, INTEGER, PERCENT, PERMILLE, SIGN;

    static {
      CURRENCY = new Field("CURRENCY");
      DECIMAL_SEPARATOR = new Field("DECIMAL_SEPARATOR");
      EXPONENT = new Field("EXPONENT");
      EXPONENT_SIGN = new Field("EXPONENT_SIGN");
      EXPONENT_SYMBOL = new Field("EXPONENT_SYMBOL");
      FRACTION = new Field("FRACTION");
      GROUPING_SEPARATOR = new Field("GROUPING_SEPARATOR");
      INTEGER = new Field("INTEGER");
      PERCENT = new Field("PERCENT");
      PERMILLE = new Field("PERMILLE");
      SIGN = new Field("SIGN");
    }

    public Field(final String name) {
      super(name);
    }

  }

  public static Locale[] getAvailableLocales() {
    // TODO
    return null;
  }

  public static NumberFormat getCurrencyInstance() {
    return getCurrencyInstance(Locale.getDefault());
  }

  public static NumberFormat getCurrencyInstance(final Locale inLocale) {
    return NumberFormat.getCurrencyInstance(inLocale);
  }

  public static NumberFormat getInstance() {
    return getInstance(Locale.getDefault());
  }

  public static NumberFormat getInstance(final Locale inLocale) {
    return getNumberInstance(inLocale);
  }

  public static NumberFormat getIntegerInstance() {
    return getIntegerInstance(Locale.getDefault());
  }

  public static NumberFormat getIntegerInstance(final Locale inLocale) {
    return NumberFormat.getIntegerInstance(inLocale);
  }

  public static NumberFormat getNumberInstance() {
    return getNumberInstance(Locale.getDefault());
  }

  public static NumberFormat getNumberInstance(final Locale inLocale) {
    return NumberFormat.getNumberInstance(inLocale);
  }

  public static NumberFormat getPercentInstance() {
    return getPercentInstance(Locale.getDefault());
  }

  public static NumberFormat getPercentInstance(final Locale inLocale) {
    return NumberFormat.getPercentInstance(inLocale);
  }

  private Currency     currency;
  private boolean      groupingUsed;
  private int          maxFractionDigit;
  private int          maxIntDigits;
  private int          minFractionDigits;
  private int          minIntDigits;
  private boolean      parseIntegerOnly;
  private RoundingMode roundingMode;

  protected NumberFormat() {}

  public abstract StringBuffer format(double number, StringBuffer toAppendTo, FieldPosition pos);

  public String format(final long number) {
    // TODO
    return null;
  }

  public abstract StringBuffer format(long number, StringBuffer toAppendTo, FieldPosition pos);

  public Currency getCurrency() {
    return currency;
  }

  public int getMaximumFractionDigits() {
    return maxFractionDigit;
  }

  public int getMaximumIntegerDigits() {
    return maxIntDigits;
  }

  public int getMinimumFractionDigits() {
    return minFractionDigits;
  }

  public int getMinimumIntegerDigits() {
    return minIntDigits;
  }

  public RoundingMode getRoundingMode() {
    return roundingMode;
  }

  public boolean isGroupingUsed() {
    return groupingUsed;
  }

  public boolean isParseIntegerOnly() {
    return parseIntegerOnly;
  }

  public Number parse(final String source) {
    // TODO
    return null;
  }

  public abstract Number parse(String source, ParsePosition parsePosition);

  public void setCurrency(final Currency currency) {
    this.currency = currency;
  }

  public void setGroupingUsed(final boolean newValue) {
    groupingUsed = newValue;
  }

  public void setMaximumFractionDigits(final int newValue) {
    maxFractionDigit = newValue;
  }

  public void setMaximumIntegerDigits(final int newValue) {
    maxIntDigits = newValue;
  }

  public void setMinimumFractionDigits(final int newValue) {
    minFractionDigits = newValue;
  }

  public void setMinimumIntegerDigits(final int newValue) {
    minIntDigits = newValue;
  }

  public void setParseIntegerOnly(final boolean value) {
    parseIntegerOnly = value;
  }

  public void setRoundingMode(final RoundingMode roundingMode) {
    this.roundingMode = roundingMode;
  }
}
