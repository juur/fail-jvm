package java.text;

import java.util.Locale;

public class DecimalFormatSymbols {

  public static DecimalFormatSymbols getInstance() {
    return new DecimalFormatSymbols();
  }

  public static DecimalFormatSymbols getInstance(final Locale locale) {
    return new DecimalFormatSymbols(locale);
  }

  public DecimalFormatSymbols() {
    this(Locale.getDefault());
  }

  public DecimalFormatSymbols(final Locale aLocale) {}

}
