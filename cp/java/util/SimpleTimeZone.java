package java.util;

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

  public SimpleTimeZone(final int newRawOffset, final String ID) {
    setRawOffset(newRawOffset);
    setID(ID);
    setDSTSavings(0);
  }

  public SimpleTimeZone(
    final int newRawOffset, final String ID, final int newStartMonth, final int newStartDay,
    final int newStartDayOfWeek, final int newStartTime, final int newEndMonth, final int newEndDay,
    final int newEndDayOfWeek, final int newEndTime
    ) {
    this(newRawOffset, ID, newStartMonth, newStartDay, newStartDayOfWeek, newStartTime,
      SimpleTimeZone.WALL_TIME, newEndMonth, newEndDay, newEndDayOfWeek, newEndTime,
      SimpleTimeZone.WALL_TIME, 3600000);
  }

  public SimpleTimeZone(
    final int newRawOffset, final String ID, final int newStartMonth, final int newStartDay,
    final int newStartDayOfWeek, final int newStartTime, final int newEndMonth, final int newEndDay,
    final int newEndDayOfWeek, final int newEndTime, final int newDstSavings
    ) {
    this(newRawOffset, ID, newStartMonth, newStartDay, newStartDayOfWeek, newStartTime, SimpleTimeZone.WALL_TIME,
      newEndMonth, newEndDay, newEndDayOfWeek, newEndTime, SimpleTimeZone.WALL_TIME, newDstSavings);
  }

  public SimpleTimeZone(
    final int newRawOffset, final String ID, final int newStartMonth, final int newStartDay,
    final int newStartDayOfWeek, final int newStartTime, final int newStartTimeMode,
    final int newEndMonth1, final int newEndDay, final int newEndDayOfWeek, final int newEndTime,
    final int newEndTimeMode, final int newDstSavings
    ) {
    if (newStartTimeMode < WALL_TIME || newStartTimeMode > UTC_TIME || newEndTimeMode < WALL_TIME
      || newEndTimeMode > UTC_TIME)
      throw new IllegalArgumentException("Invalid time mode");

    setRawOffset(newRawOffset);
    setID(ID);
    setStartRule(newStartMonth, newStartDay, newStartDayOfWeek, newStartTime);
    startTimeMode = newStartTimeMode;
    setEndRule(newEndMonth1, newEndDay, newEndDayOfWeek, newEndTime);
    endTimeMode = newEndTimeMode;
    setDSTSavings(newDstSavings);
  }

  @Override
  protected Object clone() {
    System.out.println("clone()");
    return new SimpleTimeZone(getRawOffset(), getID(), startMonth, startDay, startDayOfWeek,
      startTime, endMonth, endDay, endDayOfWeek, endTime);
  }

  @Override
  public int getDSTSavings() {
    return dstSavings;
  }

  @Override
  public int getOffset(
    final int era,
    final int year,
    final int month,
    final int day,
    final int dayOfWeek,
    final int milliseconds
    ) {
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
    start.set(check.get(Calendar.YEAR), startMonth, startDay, startTime, 0, 0);
    // FIXME populate start based on date should use startMode

    final Calendar end = Calendar.getInstance(this);
    end.set(check.get(Calendar.YEAR), endMonth, endDay, endTime, 0, 0);
    // FIXME populate end based on date should use endMode

    if (start.before(check) && end.after(check))
      return true;
    return false;
  }

  public void setDSTSavings(final int newDstSavings) {
    dstSavings = newDstSavings;
  }

  public void setEndRule(final int newEndMonth, final int newEndDay, final int newEndTime) {
    setEndRule(newEndMonth, newEndDay, 0, newEndTime);
  }

  public void setEndRule(
    final int newEndMonth,
    final int newEndDay,
    final int newEndDayOfWeek,
    final int newEndTime
    ) {
    if (newEndMonth < Calendar.JANUARY || newEndMonth > Calendar.DECEMBER)
      throw new IllegalArgumentException("Invalid month");
    if (newEndDayOfWeek != 0 && (newEndDayOfWeek < -Calendar.SUNDAY || newEndDayOfWeek > -Calendar.SATURDAY)
      && (newEndDayOfWeek < -Calendar.SUNDAY || newEndDayOfWeek > -Calendar.SATURDAY))
      throw new IllegalArgumentException("Invalid day of week");
    endMonth = newEndMonth;
    endDay = newEndDay;
    endDayOfWeek = newEndDayOfWeek;
    endTime = newEndTime;
    endTimeMode = WALL_TIME;
  }

  public void setEndRule(
    final int newEndMonth,
    final int newEndDay,
    final int newEndDayOfWeek,
    final int newEndTime,
    final boolean after
    ) {
    // TODO
  }

  @Override
  public void setRawOffset(final int offsetMillis) {
    rawOffset = offsetMillis;
  }

  public void setStartRule(final int newStartMonth, final int newStartDay, final int newStartTime) {
    setStartRule(newStartMonth, newStartDay, 0, newStartTime);
  }

  public void setStartRule(
    final int newStartMonth,
    final int newStartDay,
    final int newStartDayOfWeek,
    final int newStartTime
    ) {
    if (newStartMonth < Calendar.JANUARY || newStartMonth > Calendar.DECEMBER)
      throw new IllegalArgumentException("Invalid month");
    if (newStartDayOfWeek != 0
      && (newStartDayOfWeek < -Calendar.SUNDAY || newStartDayOfWeek > -Calendar.SATURDAY)
      && (newStartDayOfWeek < -Calendar.SUNDAY || newStartDayOfWeek > -Calendar.SATURDAY))
      throw new IllegalArgumentException("Invalid day of week");
    startMonth = newStartMonth;
    startDay = newStartDay;
    startDayOfWeek = newStartDayOfWeek;
    startTime = newStartTime;
    startTimeMode = WALL_TIME;
  }

  public void setStartRule(
    final int newStartMonth,
    final int newStartDay,
    final int newStartDayOfWeek,
    final int newStartTime,
    final boolean after
    ) {
    // TODO
  }

  @Override
  public boolean useDaylightTime() {
    return getDSTSavings() != 0;
  }
}
