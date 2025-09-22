package java.util.logging;

import java.io.Serializable;
import java.util.ResourceBundle;

public class LogRecord implements Serializable {

  public static final long serialVersionUID = 5372048053134512534L;
  private String           msg;
  private Level            level;
  private Throwable        thrown;
  private int              threadId;
  private ResourceBundle   resourceBundle;
  private Object[]         parameters;

  public LogRecord(final Level newLevel, final String newMsg) {
    level = newLevel;
    msg = newMsg;
  }

  public Level getLevel() {
    return level;
  }

  public String getMessage() {
    return msg;
  }

  public int getThreadId() {
    return threadId;
  }

  public Throwable getThrown() {
    return thrown;
  }

  public void setLevel(final Level newLevel) {
    level = newLevel;
  }

  public void setMessage(final String message) {
    msg = message;
  }

  public void setParameters(final Object[] newParameters) {
    parameters = newParameters;
  }

  public void setThreadId(final int threadID) {
    threadId = threadID;
  }

  public void setThrown(final Throwable newThrown) {
    thrown = newThrown;
  }
}
