package java.util;

public final class Locale {

  private static class langCls {

    String iso2, iso3, display;

    langCls(final String a, final String b, final String c) {
      iso2 = a;
      iso3 = b;
      display = c;
    }
  }

  private static final langCls[] countries;
  private static final Locale    defLocale;
  private static final langCls[] langs;

  static {
	  System.out.println("Locale.clinit");
    countries = new langCls[] { new langCls("GB", "GBR", "United Kingdom") };
    langs = new langCls[] { new langCls("en", "eng", "English") };
    defLocale = new Locale("en", "GB");
  }

  public static Locale getDefault() {
    return defLocale;
  }

  private String country;

  private String displayLanguage, displayCountry;
  private String isoLanguage, isoCountry;

  private String language;

  private final String variant;

  public Locale(final String language) {
    this(language, null, null);
  }

  public Locale(final String language, final String country) {
    this(language, country, null);
  }

  public Locale(final String language, final String country, final String variant) {
    setupLang(language);
    setupCountry(country);
    this.variant = variant;
  }

  public String getCountry() {
    return country;
  }

  public String getDisplayCountry() {
    return displayCountry;
  }

  public String getDisplayLanguage() {
    return displayLanguage;
  }

  public String getDisplayVariant() {
    return variant;
  }

  public String getISO3Country() {
    return isoCountry;
  }

  public String getISO3Language() {
    return isoLanguage;
  }

  public String getLanguage() {
    return language;
  }

  @Override
  public String toString() {
    return "Locale [" + getISO3Country() + "," + getISO3Language() + "]";
  }

  private void setupCountry(final String str) {
    if (str != null)
      for (final langCls lang2 : countries) {
        if (lang2.iso2.equalsIgnoreCase(str)) {
          country = lang2.iso2;
          isoCountry = lang2.iso3;
          displayCountry = lang2.display;
          return;
        }
      }
    country = null;
    isoCountry = null;
    displayCountry = null;
  }

  private void setupLang(final String str) {
    if (langs == null) {
      System.err.println("langs is null mate");
    } else if (str != null)
      for (final langCls lang2 : langs) {
        if (lang2 == null) {
          System.err.println("null entry in array langs");
        } else if (lang2.iso2 == null) {
          System.err.println("null iso2 in lang2");
        } else if (lang2.iso2.equalsIgnoreCase(str)) {
          language = lang2.iso2;
          isoLanguage = lang2.iso3;
          displayLanguage = lang2.display;
          return;
        }
      }
    language = null;
    isoLanguage = null;
    displayLanguage = null;
  }
}
