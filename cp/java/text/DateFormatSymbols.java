package java.text;

import java.util.Locale;

public class DateFormatSymbols {

  public static DateFormatSymbols getInstance() {
    return new DateFormatSymbols();
  }

  public static DateFormatSymbols getInstance(final Locale aLocale) {
    return new DateFormatSymbols(aLocale);
  }

  public DateFormatSymbols() {
    this(Locale.getDefault());
  }

  public DateFormatSymbols(final Locale default1) {}
}
