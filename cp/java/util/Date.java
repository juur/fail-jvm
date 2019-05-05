package java.util;

public class Date {

  private long date;

  public Date() {
    date = System.currentTimeMillis() / 1000;
  }

  public Date(final long date) {
    this.date = date;
  }

  public boolean after(final Date when) {
    return getTime() > when.getTime();
  }

  public boolean before(final Date when) {
    return getTime() < when.getTime();
  }

  public long getTime() {
    return date;
  }

  public void setTime(final long time) {
    date = time;
  }
}
