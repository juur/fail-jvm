package spod;

import java.util.SimpleTimeZone;
import java.util.TimeZone;

public abstract class TimeZones extends SimpleTimeZone {

  final static long serialVersionUID = 1L;

  private static String[]   timeZoneIDs;
  private static TimeZone[] timeZones;

  public synchronized static String[] getTimeZoneIDs() {
    if (timeZoneIDs == null) {
      final TimeZone[] zones   = getTimeZones();
      final String[]   zoneIDs = new String[zones.length];

      for (int i = 0; i < zones.length; i++)
        zoneIDs[i] = zones[i].getID();

      timeZoneIDs = zoneIDs;
    }
    return timeZoneIDs;
  }

  public synchronized static TimeZone[] getTimeZones() {
    if (timeZones == null) {
      final TimeZone[] zones = new SimpleTimeZone[] {
        new SimpleTimeZone(0, "UTC"),
        new SimpleTimeZone(0, "GMT"),
        // new SimpleTimeZone(0, "Europe/London", Calendar.MARCH, -1, Calendar.SUNDAY,
        // 3600000, Calendar.OCTOBER, -1, Calendar.SUNDAY, 3600000)
      };
      timeZones = zones;
    }
    return timeZones;
  }

  private TimeZones(final int rawOffset, final String ID) {
    super(rawOffset, ID);
    throw new IllegalStateException();
  }

}
