package java.util.logging;

import java.util.Formatter;

public abstract class Handler {

  private Formatter formatter = null;
  private final Level level     = Level.ALL;

  protected Handler() {
  }

  public abstract void close() throws SecurityException;

  public abstract void flush();

  public Formatter getFormatter() {
    return formatter;
  }

  public boolean isLoggable(final LogRecord record) {
    if (record.getLevel().intValue() < level.intValue())
      return false;
    return true;
  }

  public abstract void publish(LogRecord record);

  public void setFormatter(final Formatter newFormatter) throws SecurityException {
    formatter = newFormatter;
  }
}
