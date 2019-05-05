package java.util;

import java.util.TimeZone;
import java.util.Locale;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.Map;

public abstract class Calendar {

  public static final int ALL_STYLES           = 0;
  public static final int AM                   = 0;
  public static final int AM_PM                = 9;
  public static final int APRIL                = 3;
  public static final int AUGUST               = 7;
  public static final int DATE                 = 5;
  public static final int DAY_OF_MONTH         = 5;
  public static final int DAY_OF_WEEK          = 7;
  public static final int DAY_OF_WEEK_IN_MONTH = 8;
  public static final int DAY_OF_YEAR          = 6;
  public static final int DECEMBER             = 11;
  public static final int DST_OFFSET           = 16;
  public static final int ERA                  = 0;
  public static final int FEBRUARY             = 1;
  public static final int FIELD_COUNT          = 17;
  public static final int FRIDAY               = 6;
  public static final int HOUR                 = 10;
  public static final int HOUR_OF_DAY          = 11;
  public static final int JANUARY              = 0;
  public static final int JULY                 = 6;
  public static final int JUNE                 = 5;
  public static final int LONG                 = 2;
  public static final int MARCH                = 2;
  public static final int MAY                  = 4;
  public static final int MILLISECOND          = 14;
  public static final int MINUTE               = 12;
  public static final int MONDAY               = 2;
  public static final int MONTH                = 2;
  public static final int NOVEMBER             = 10;
  public static final int OCTOBER              = 9;
  public static final int PM                   = 1;
  public static final int SATURDAY             = 7;
  public static final int SECOND               = 13;
  public static final int SEPTEMBER            = 8;
  public static final int SHORT                = 1;
  public static final int SUNDAY               = 1;
  public static final int THURSDAY             = 5;
  public static final int TUESDAY              = 3;
  public static final int UNDECIMBER           = 12;
  public static final int WEDNESDAY            = 4;
  public static final int WEEK_OF_MONTH        = 4;
  public static final int WEEK_OF_YEAR         = 3;
  public static final int YEAR                 = 1;
  public static final int ZONE_OFFSET          = 15;

  public static Locale[] getAvailableLocales() {
    return new Locale[0];
  }

  public static Calendar getInstance() {
    return getInstance(TimeZone.getDefault(), Locale.getDefault());
  }

  public static Calendar getInstance(final Locale aLocale) {
    return getInstance(TimeZone.getDefault(), aLocale);
  }

  public static Calendar getInstance(final TimeZone zone) {
    return getInstance(zone, Locale.getDefault());
  }

  public static Calendar getInstance(final TimeZone zone, final Locale aLocale) {
	  System.out.println("Requesting GregorianCalendar("+zone.toString()+","+aLocale.toString()+")");
    return new GregorianCalendar(zone, aLocale);
  }

  private int           firstDOW;
  private final boolean lenient;
  @SuppressWarnings("unused")
  private Locale        locale;
  private int           minDaysInFirstWeek;
  private TimeZone      zone;

  protected boolean[] areFieldsSet;

  protected int[] fields;

  protected boolean[] isSet;

  protected boolean isTimeSet;

  protected long time;

  protected Calendar() {
    super();
    lenient = true;
    isSet = new boolean[FIELD_COUNT];
    fields = new int[FIELD_COUNT];
    areFieldsSet = new boolean[FIELD_COUNT];
    firstDOW = SUNDAY;
    minDaysInFirstWeek = 0;

    clear();
    setTimeInMillis(System.currentTimeMillis());
    computeFields();
  }

  protected Calendar(final TimeZone zone, final Locale aLocale) {
    this();
    setTimeZone(zone);
    locale = aLocale;
    computeFields();
  }

  public abstract void add(int field, int amount);

  public boolean after(final Object when) {
    if (when == null || !(when instanceof Calendar))
      return false;
    if (getTimeInMillis() > ((Calendar) when).getTimeInMillis())
      return true;
    return false;
  }

  public boolean before(final Object when) {
    if (when == null || !(when instanceof Calendar))
      return false;
    if (getTimeInMillis() < ((Calendar) when).getTimeInMillis())
      return true;
    return false;
  }

  public void clear() {
    for (int i = 0; i < FIELD_COUNT; i++) {
      fields[i] = Integer.MIN_VALUE;
      isSet[i] = false;
      areFieldsSet[i] = false;
    }
    time = Long.MIN_VALUE;
  }

  public void clear(final int field) {
    if (field < 0 || field >= FIELD_COUNT)
      throw new ArrayIndexOutOfBoundsException();
    fields[field] = Integer.MIN_VALUE;
    isSet[field] = false;
    areFieldsSet[field] = false;
  }

  public int get(final int field) {
    if (field < 0 || field >= FIELD_COUNT)
      throw new ArrayIndexOutOfBoundsException();
    final int val = internalGet(field);
    if (isLenient()) {
      return val;
    } else {
      return val;
    }
  }

  public int getActualMaximum(final int field) {
    if (field < 0 || field >= FIELD_COUNT)
      throw new ArrayIndexOutOfBoundsException();
    // TODO
    return 0;
  }

  public int getActualMinimum(final int field) {
    if (field < 0 || field >= FIELD_COUNT)
      throw new ArrayIndexOutOfBoundsException();
    // TODO
    return 0;
  }

  public String getDisplayName(final int field, final int style, final Locale locale) {
    return "Blah";
  }

  public Map<String, Integer> getDisplayNames(final int field, final int style, final Locale locale) {
    return null;
  }

  public int getFirstDayOfWeek() {
    return firstDOW;
  }

  public abstract int getGreatestMinimum(int field);

  public abstract int getLeastMaximum(int field);

  public abstract int getMaximum(int field);

  public int getMinimalDaysInFirstWeek() {
    return minDaysInFirstWeek;
  }

  public abstract int getMinimum(int field);

  public Date getTime() {
    return new Date(getTimeInMillis());
  }

  public long getTimeInMillis() {
    return time;
  }

  public TimeZone getTimeZone() {
    return zone;
  }

  public boolean isLenient() {
    return lenient;
  }

  public boolean isSet(final int field) {
    if (field < 0 || field >= FIELD_COUNT)
      throw new ArrayIndexOutOfBoundsException();
    return isSet[field];
  }

  public abstract void roll(int field, boolean up);

  public void roll(final int field, final int amount) {
    // TODO
  }

  public void set(final int field, final int value) {
    if (field < 0 || field >= FIELD_COUNT)
      throw new ArrayIndexOutOfBoundsException();
    fields[field] = value;
    areFieldsSet[field] = false;
    isSet[field] = true;
  }

  public void set(final int year, final int month, final int date) {
    set(YEAR, year);
    set(MONTH, month);
    set(DAY_OF_MONTH, date);
  }

  public void set(final int year, final int month, final int date, final int hourOfDay, final int minute) {
    set(year, month, date);
    set(HOUR_OF_DAY, hourOfDay);
    set(MINUTE, minute);
  }

  public void set(final int year, final int month, final int date, final int hourOfDay, final int minute, final int second) {
    set(year, month, date, hourOfDay, minute);
    set(SECOND, second);
  }

  public void setFirstDayOfWeek(final int value) {
    firstDOW = value;
  }

  public void setLenient(boolean lenient) {
    lenient = true;
  }

  public void setMinimalDaysInFirstWeek(final int value) {
    minDaysInFirstWeek = value;
  }

  public void setTime(final Date date) {
    time = date.getTime();
    computeFields();
  }

  public void setTimeInMillis(final long millis) {
    time = millis;
    computeFields();
  }

  public void setTimeZone(final TimeZone value) {
    zone = value;
    computeFields();
  }

  @Override
  public String toString() {
    return (getTimeZone() == null ? "(null TZ)" : getTimeZone()) + "," + getTimeInMillis();
  }

  protected void complete() {
    // TODO
  }

  protected abstract void computeFields();

  protected abstract void computeTime();

  protected int internalGet(final int field) {
    return fields[field];
  }
}
