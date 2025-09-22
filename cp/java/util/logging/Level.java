package java.util.logging;

import java.io.Serializable;

public class Level implements Serializable {

  public static final Level ALL = new Level("ALL", Integer.MIN_VALUE);

  public static final Level SEVERE  = new Level("SEVERE", 1000);
  public static final Level WARNING = new Level("WARNING", 900);
  public static final Level INFO    = new Level("INFO", 800);
  public static final Level CONFIG  = new Level("CONFIG", 700);
  public static final Level FINE    = new Level("FINE", 500);
  public static final Level FINER   = new Level("FINER", 400);
  public static final Level FINEST  = new Level("FINEST", 300);

  public static final Level OFF = new Level("MAX", Integer.MAX_VALUE);

  private final String name;
  private final int    value;

  protected Level(final String newName, final int newValue) {
    name = newName;
    value = newValue;
  }

  public int intValue() {
    return value;
  }
}
