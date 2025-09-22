package java.util.logging;

import java.util.HashSet;
import java.util.Set;

public class Logger {

  public static String GLOBAL_LOGGER_NAME = "global";

  public static Logger getLogger(final String name) {
    return null;
  }

  public static Logger getLogger(final String name, final String resourceBundleName) {
    return null;
  }

  private Level              levelFilter;
  private final Set<Handler> handlers = new HashSet<Handler>();

  protected Logger(final String name, final String resourceBundleName) {

  }

  public void config(final String msg) {
    log(Level.CONFIG, msg);
  }

  public void fine(final String msg) {
    log(Level.FINE, msg);
  }

  public void finer(final String msg) {
    log(Level.FINER, msg);
  }

  public void finest(final String msg) {
    log(Level.FINEST, msg);
  }

  public void info(final String msg) {
    log(Level.INFO, msg);
  }

  public boolean isLoggable(final Level level) {
    if (level.intValue() < levelFilter.intValue())
      return false;
    return true;
  }

  public void log(final Level level, final String string) {
    log(new LogRecord(level, string));
  }

  public void log(final Level level, final String string, final Object param1) {
    final LogRecord record = new LogRecord(level, string);
    record.setParameters(new Object[] { param1 });
  }

  public void log(final LogRecord record) {
    if (!isLoggable(record.getLevel()))
      return;
  }

  public void setLevel(final Level newLevel) throws SecurityException {
    levelFilter = newLevel;
  }

  public void severe(final String msg) {
    log(Level.SEVERE, msg);
  }

  public void warning(final String msg) {
    log(Level.WARNING, msg);
  }
}
