package java.util;

public class GregorianCalendar extends Calendar {

  private static class Tuple {

    int year, month, day;

    public Tuple(final int year, final int month, final int day) {
      super();
      this.year = year;
      this.month = month;
      this.day = day;
    }
  }

  private long change = -12219292725L * 1000;

  public GregorianCalendar() {
    this(TimeZone.getDefault(), Locale.getDefault());
  }

  public GregorianCalendar(final int year, final int month, final int dayOfMonth) {
    this(year, month, dayOfMonth, 0, 0, 0);
  }

  public GregorianCalendar(final int year, final int month, final int dayOfMonth, final int hourOfDay, final int minute) {
    this(year, month, dayOfMonth, hourOfDay, minute, 0);
  }

  public GregorianCalendar(final int year, final int month, final int dayOfMonth, final int hourOfDay, final int minute, final int second) {
    this();
    set(YEAR, year);
    set(DAY_OF_MONTH, dayOfMonth);
    set(MONTH, month);
    set(HOUR_OF_DAY, hourOfDay);
    set(MINUTE, minute);
    set(SECOND, second);
  }

  public GregorianCalendar(final Locale aLocale) {
    this(TimeZone.getDefault(), aLocale);
  }

  public GregorianCalendar(final TimeZone zone) {
    this(zone, Locale.getDefault());
  }

  public GregorianCalendar(final TimeZone zone, final Locale aLocale) {
    super(zone, aLocale);
  }

  @Override
  public void add(final int field, final int amount) {

  }

  public Tuple civil_from_days(int z) {
    z += 719468;
    final int era   = (z >= 0 ? z : z - 146096) / 146097;
    final int doe   = z - era * 146097;
    final int yoe   = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    int       year  = yoe + era * 400;
    final int doy   = doe - (365 * yoe + yoe / 4 - yoe / 100);
    final int mp    = (5 * doy + 2) / 153;
    final int day   = doy - (153 * mp + 2) / 5 + 1;
    final int month = mp + (mp < 10 ? 3 : -9);
    year += month <= 2 ? 1 : 0;
    return new Tuple(year, month, day);
  }

  public int days_from_civil(int y, final int m, final int d) {
    y -= m <= 2 ? 1 : 0;
    final int era = (y >= 0 ? y : y - 399) / 400;
    final int yoe = y - era * 400;
    final int doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    final int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
  }

  @Override
  public int getGreatestMinimum(final int field) {
    return 0;
  }

  public Date getGregorianChange() {
    return new Date(change);
  }

  @Override
  public int getLeastMaximum(final int field) {
    return 0;
  }

  @Override
  public int getMaximum(final int field) {
    return 0;
  }

  @Override
  public int getMinimum(final int field) {
    return 0;
  }

  public boolean isLeapYear(final int year) {
    return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
  }

  @Override
  public void roll(final int field, final boolean up) {

  }

  public void setGregorianChange(final Date date) {
    change = date.getTime();
  }

  public int weekday_from_days(final int z) {
    return z >= -4 ? (z + 4) % 7 : (z + 5) % 7 + 6;
  }

  @Override
  protected void computeFields() {
    final int   days      = (int) (time / (24 * 60 * 60 * 1000));
    // final int remainder = (int) (time % (24 * 60 * 60 * 1000));
    final Tuple a         = civil_from_days(days);
    set(YEAR, a.year);
    set(MONTH, a.month);
    set(DAY_OF_MONTH, a.day);
    set(DAY_OF_WEEK, weekday_from_days(days));
  }

  @Override
  protected void computeTime() {
    final int year  = get(YEAR);
    final int month = get(MONTH);
    final int day   = get(DAY_OF_MONTH);
    time = days_from_civil(year, month, day) * 24 * 60 * 60 * 1000;
    time += 1000 * 60 * 60 * get(HOUR_OF_DAY);
    time += 1000 * 60 * get(MINUTE);
    time += 1000 * get(SECOND);
    time += get(MILLISECOND);
  }
}
