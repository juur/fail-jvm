package java.util;

import java.util.TimeZone;
import java.util.Date;
import java.util.Calendar;

public class SimpleTimeZone extends TimeZone {

  final static long serialVersionUID = 1L;

  public static final int STANDARD_TIME = 1;
  public static final int UTC_TIME      = 2;
  public static final int WALL_TIME     = 0;

  private int dstSavings;
  private int endDay;
  private int endDayOfWeek;
  private int endMonth;
  private int endTime;
  private int endTimeMode;
  private int rawOffset;
  private int startDay;
  private int startDayOfWeek;
  private int startMonth;
  private int startTime;
  private int startTimeMode;

  public SimpleTimeZone(final int rawOffset, final String ID) {
    System.out.println("SimpleTimeZone(" + rawOffset + "," + ID + ")");
    setRawOffset(rawOffset);
    setID(ID);
    setDSTSavings(0);
  }

  public SimpleTimeZone(
    final int rawOffset, final String ID, final int startMonth, final int startDay, final int startDayOfWeek, final int startTime,
    final int endMonth, final int endDay, final int endDayOfWeek, final int endTime
    ) {
    this(rawOffset, ID, startMonth, startDay, startDayOfWeek, startTime, SimpleTimeZone.WALL_TIME, endMonth, endDay, endDayOfWeek, endTime,
      SimpleTimeZone.WALL_TIME, 3600000);
  }

  public SimpleTimeZone(
    final int rawOffset, final String ID, final int startMonth, final int startDay, final int startDayOfWeek, final int startTime,
    final int endMonth, final int endDay, final int endDayOfWeek, final int endTime, final int dstSavings
    ) {
    this(rawOffset, ID, startMonth, startDay, startDayOfWeek, startTime, SimpleTimeZone.WALL_TIME, endMonth, endDay, endDayOfWeek, endTime,
      SimpleTimeZone.WALL_TIME, dstSavings);
  }

  public SimpleTimeZone(
    final int rawOffset, final String ID, final int startMonth, final int startDay, final int startDayOfWeek, final int startTime,
    final int startTimeMode, final int endMonth, final int endDay, final int endDayOfWeek, final int endTime, final int endTimeMode,
    final int dstSavings
    ) {
	System.out.println("SimpleTimeZone("+rawOffset+","+ID+",lots)");
    if (startTimeMode < WALL_TIME || startTimeMode > UTC_TIME || endTimeMode < WALL_TIME || endTimeMode > UTC_TIME)
      throw new IllegalArgumentException("Invalid time mode");


    setRawOffset(rawOffset);
    setID(ID);
    setStartRule(startMonth, startDay, startDayOfWeek, startTime);
    this.startTimeMode = startTimeMode;
    setEndRule(endMonth, endDay, endDayOfWeek, endTime);
    this.endTimeMode = endTimeMode;
    setDSTSavings(dstSavings);
  }

  @Override
  public int getDSTSavings() {
    return dstSavings;
  }

  @Override
  public int getOffset(final int era, final int year, final int month, final int day, final int dayOfWeek, final int milliseconds) {
    // FIXME
    return getRawOffset();
  }

  @Override
  public int getRawOffset() {
    return rawOffset;
  }

  @Override
  public boolean inDaylightTime(final Date date) {
    final Calendar check = Calendar.getInstance(this);
    check.setTime(date);

    final Calendar start = Calendar.getInstance(this);
    // TODO populate start based on date

    final Calendar end = Calendar.getInstance(this);
    // TODO populate end based on date

    if (start.before(check) && end.after(check))
      return true;
    return false;
  }

  public void setDSTSavings(final int dstSavings) {
    this.dstSavings = dstSavings;
  }

  public void setEndRule(final int endMonth, final int endDay, final int endTime) {
    setEndRule(endMonth, endDay, 0, endTime);
  }

  public void setEndRule(final int endMonth, final int endDay, final int endDayOfWeek, final int endTime) {
    if (endMonth < Calendar.JANUARY || endMonth > Calendar.DECEMBER)
      throw new IllegalArgumentException("Invalid month");
    if (endDayOfWeek != 0 && (endDayOfWeek < -Calendar.SUNDAY || endDayOfWeek > -Calendar.SATURDAY)
      && (endDayOfWeek < -Calendar.SUNDAY || endDayOfWeek > -Calendar.SATURDAY))
      throw new IllegalArgumentException("Invalid day of week");
    this.endMonth = endMonth;
    this.endDay = endDay;
    this.endDayOfWeek = endDayOfWeek;
    this.endTime = endTime;
    endTimeMode = WALL_TIME;
  }

  public void setEndRule(final int endMonth, final int endDay, final int endDayOfWeek, final int endTime, final boolean after) {
    // TODO
  }

  @Override
  public void setRawOffset(final int offsetMillis) {
    rawOffset = offsetMillis;
  }

  public void setStartRule(final int startMonth, final int startDay, final int startTime) {
    setStartRule(startMonth, startDay, 0, startTime);
  }

  public void setStartRule(final int startMonth, final int startDay, final int startDayOfWeek, final int startTime) {
    if (startMonth < Calendar.JANUARY || startMonth > Calendar.DECEMBER)
      throw new IllegalArgumentException("Invalid month");
    if (startDayOfWeek != 0 && (startDayOfWeek < -Calendar.SUNDAY || startDayOfWeek > -Calendar.SATURDAY)
      && (startDayOfWeek < -Calendar.SUNDAY || startDayOfWeek > -Calendar.SATURDAY))
      throw new IllegalArgumentException("Invalid day of week");
    this.startMonth = startMonth;
    this.startDay = startDay;
    this.startDayOfWeek = startDayOfWeek;
    this.startTime = startTime;
    startTimeMode = WALL_TIME;
  }

  public void setStartRule(final int startMonth, final int startDay, final int startDayOfWeek, final int startTime, final boolean after) {
    // TODO
  }

  @Override
  public boolean useDaylightTime() {
    return getDSTSavings() != 0;
  }

  @Override
  protected Object clone() {
	  System.out.println("clone()");
    return new SimpleTimeZone(getRawOffset(), getID(), startMonth, startDay, startDayOfWeek, startTime, endMonth, endDay, endDayOfWeek,
      endTime);
  }
}
