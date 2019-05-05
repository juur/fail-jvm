package java.util;

import spod.TimeZones;

public abstract class TimeZone {

  public static final int LONG = 1, SHORT = 0;
  private static TimeZone defTZ;

  public static synchronized String[] getAvailableIDs() {
    return TimeZones.getTimeZoneIDs();
  }

  public static synchronized String[] getAvailableIDs(final int rawOffset) {
    return TimeZones.getTimeZoneIDs();
  }

  public static TimeZone getDefault() {
    if (defTZ == null) {
      defTZ = setDefaultZone();
    }
    return (TimeZone) defTZ.clone();
  }

  public static TimeZone getTimeZone(final String ID) {
    // FIXME custom time zones
    if (TimeZones.getTimeZones() == null) {
      System.err.println("warning: no timezones");
    }
    for (final TimeZone tz : TimeZones.getTimeZones()) {
      if (tz == null)
        System.err.println("warning: tz is null?");
      else if (tz.getID() == null)
        System.err.println("warning: tz ID is null");
      else if (tz.getID().equals(ID))
        return tz;
    }
    return getTimeZone("GMT");
  }

  public static void setDefault(final TimeZone zone) {
    defTZ = (TimeZone) (zone == null ? null : zone.clone());
  }

  private static synchronized TimeZone setDefaultZone() {
    return getTimeZone("GMT");
  }

  private String ID;

  public String getDisplayName() {
    return getDisplayName(Locale.getDefault());
  }

  public String getDisplayName(final boolean daylight, final int style) {
    return getDisplayName(daylight, style, Locale.getDefault());
  }

  public String getDisplayName(final boolean daylight, final int style, final Locale locale) {
    return getID();
  }

  public String getDisplayName(final Locale locale) {
    return getDisplayName(false, LONG, locale);
  }

  public int getDSTSavings() {
    if (useDaylightTime())
      return 3600000;
    return 0;
  }

  public String getID() {
    return ID;
  }

  public abstract int getOffset(int era, int year, int month, int day, int dayOfWeek, int milliseconds);

  public int getOffset(final long date) {
    // FIXME
    return getRawOffset();
  }

  public abstract int getRawOffset();

  public boolean hasSameRules(final TimeZone other) {
    if (other == null)
      return false;
    if (other.getRawOffset() != getRawOffset())
      return false;
    if (other.getDSTSavings() != getDSTSavings())
      return false;
    return true;
  }

  public abstract boolean inDaylightTime(Date date);

  public void setID(final String ID) {
    this.ID = ID;
  }

  public abstract void setRawOffset(int offsetMillis);

  @Override
  public String toString() {
    return getDisplayName();
  }

  public abstract boolean useDaylightTime();

  @Override
  protected Object clone() {
    return null;
  }
}
