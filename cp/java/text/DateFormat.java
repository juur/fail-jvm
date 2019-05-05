package java.text;

import java.util.Calendar;
import java.util.Date;
import java.util.Locale;
import java.util.TimeZone;

public abstract class DateFormat extends Format {

  public static class Field extends Format.Field {

    public static final Field AM_PM, DAY_OF_MONTH, DAY_OF_WEEK, DAY_OF_WEEK_IN_MONTH;
    public static final Field DAY_OF_YEAR, ERA, HOUR_OF_DAY0, HOUR_OF_DAY1, HOUR0, HOUR1;
    public static final Field MILLISECOND, MINUTE, MONTH, SECOND, TIME_ZONE, WEEK_OF_MONTH;
    public static final Field WEEK_OF_YEAR, YEAR;

    static {
      AM_PM = new Field("AM_PM");
      DAY_OF_MONTH = new Field("DAY_OF_MONTH");
      DAY_OF_WEEK = new Field("DAY_OF_WEEK");
      DAY_OF_WEEK_IN_MONTH = new Field("DAY_OF_WEEK_IN_MONT");
      DAY_OF_YEAR = new Field("DAY_OF_YEAR");
      ERA = new Field("ERA");
      HOUR_OF_DAY0 = new Field("HOUR_OF_DAY0");
      HOUR_OF_DAY1 = new Field("HOUR_OF_DAY1");
      HOUR0 = new Field("HOUR0");
      HOUR1 = new Field("HOUR1");
      MILLISECOND = new Field("MILLISECOND");
      MINUTE = new Field("MINUTE");
      MONTH = new Field("MONT");
      SECOND = new Field("SECOND");
      TIME_ZONE = new Field("TIME_ZONE");
      WEEK_OF_MONTH = new Field("WEEK_OF_MONTH");
      WEEK_OF_YEAR = new Field("WEEK_OF_YEAR");
      YEAR = new Field("YEAR");
    }

    public Field(final String name) {
      super(name);
    }

  }

  public static final int AM_PM_FIELD = 14;

  public static final int DATE_FIELD                 = 3;
  public static final int DAY_OF_WEEK_FIELD          = 9;
  public static final int DAY_OF_WEEK_IN_MONTH_FIELD = 11;
  public static final int DAY_OF_YEAR_FIELD          = 10;
  public static final int DEFAULT                    = 2;
  public static final int ERA_FIELD                  = 0;
  public static final int FULL                       = 0;
  public static final int HOUR_OF_DAY0_FIELD         = 5;
  public static final int HOUR_OF_DAY1_FIELD         = 4;
  public static final int HOUR0_FIELD                = 16;
  public static final int HOUR1_FIELD                = 15;
  public static final int LONG                       = 1;
  public static final int MEDIUM                     = 2;
  public static final int MILLISECOND_FIELD          = 8;
  public static final int MINUTE_FIELD               = 6;
  public static final int MONTH_FIELD                = 2;
  public static final int SECOND_FIELD               = 7;
  public static final int SHORT                      = 3;
  public static final int TIMEZONE_FIELD             = 17;
  public static final int WEEK_OF_MONTH_FIELD        = 13;
  public static final int WEEK_OF_YEAR_FIELD         = 12;
  public static final int YEAR_FIELD                 = 1;

  public static Locale[] getAvailableLocales() {
    return new Locale[0];
  }

  public static DateFormat getDateInstance() {
    return getDateInstance(DEFAULT, Locale.getDefault());
  }

  public static DateFormat getDateInstance(final int style) {
    return getDateInstance(style, Locale.getDefault());
  }

  public static DateFormat getDateInstance(final int style, final Locale aLocale) {
    final DateFormat ret = new SimpleDateFormat("", aLocale);
    ret.setCalendar(Calendar.getInstance(aLocale));
    ret.setNumberFormat(NumberFormat.getInstance(aLocale));
    return ret;
  }

  public static DateFormat getDateTimeInstance() {
    return getDateTimeInstance(DEFAULT, DEFAULT, Locale.getDefault());
  }

  public static DateFormat getDateTimeInstance(final int dateStyle, final int timeStyle) {
    return getDateTimeInstance(dateStyle, timeStyle, Locale.getDefault());
  }

  public static DateFormat getDateTimeInstance(final int dateStyle, final int timeStyle, final Locale aLocale) {
    final DateFormat ret = new SimpleDateFormat("", aLocale);
    ret.setCalendar(Calendar.getInstance(aLocale));
    ret.setNumberFormat(NumberFormat.getInstance(aLocale));
    return ret;
  }

  public static DateFormat getInstance() {
    return getDateTimeInstance();
  }

  public static DateFormat getTimeInstance() {
    return getTimeInstance(DEFAULT, Locale.getDefault());
  }

  public static DateFormat getTimeInstance(final int style) {
    return getTimeInstance(style, Locale.getDefault());
  }

  public static DateFormat getTimeInstance(final int style, final Locale aLocale) {
    final DateFormat ret = new SimpleDateFormat("", aLocale);
    ret.setCalendar(Calendar.getInstance(aLocale));
    ret.setNumberFormat(NumberFormat.getInstance(aLocale));
    return ret;
  }

  protected Calendar     calendar;
  protected NumberFormat numberFormat;

  protected DateFormat() {

  }

  public abstract StringBuffer format(Date date, StringBuffer toAppendTo, FieldPosition fieldPosition);

  public Calendar getCalendar() {
    return calendar;
  }

  public NumberFormat getNumberFormat() {
    return numberFormat;
  }

  public TimeZone getTimeZone() {
    return calendar.getTimeZone();
  }

  public boolean isLenient() {
    return calendar.isLenient();
  }

  public Date parse(final String source) {
    return null;
  }

  public abstract Date parse(String source, ParsePosition pos);

  public void setCalendar(final Calendar newCalendar) {
    calendar = newCalendar;
  }

  public void setLenient(final boolean lenient) {
    calendar.setLenient(lenient);
  }

  public void setNumberFormat(final NumberFormat newNumberFormat) {
    numberFormat = newNumberFormat;
  }

  public void setTimeZone(final TimeZone zone) {
    calendar.setTimeZone(zone);
  }
}
