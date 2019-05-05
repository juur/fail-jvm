package java.text;

import java.util.Date;
import java.util.Locale;

public class SimpleDateFormat extends DateFormat {

  private DateFormatSymbols symbols;

  public SimpleDateFormat() {
    this("default", Locale.getDefault());
  }

  public SimpleDateFormat(final String pattern) {
    this(pattern, Locale.getDefault());
  }

  public SimpleDateFormat(final String pattern, final DateFormatSymbols formatSymbols) {
    super();
    symbols = formatSymbols;
  }

  public SimpleDateFormat(final String string, final Locale aLocale) {
    this(string, DateFormatSymbols.getInstance(aLocale));
  }

  public void applyLocalizedPattern(final String pattern) {}

  @Override
  public StringBuffer format(final Date date, final StringBuffer toAppendTo, final FieldPosition fieldPosition) {
    return null;
  }

  @Override
  public StringBuffer format(final Object obj, final StringBuffer toAppendTo, final FieldPosition pos) {
    return null;
  }

  public DateFormatSymbols getDateFormatSymbols() {
    return symbols;
  }

  @Override
  public Date parse(final String source, final ParsePosition pos) {
    return null;
  }

  @Override
  public Object parseObject(final String source, final ParsePosition pos) {
    return null;
  }

  public void setDateFormatSymboles(final DateFormatSymbols newFormatSymbols) {
    symbols = newFormatSymbols;
  }

}
