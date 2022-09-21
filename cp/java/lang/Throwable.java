package java.lang;

import java.io.PrintStream;

public class Throwable {

  private final Throwable               cause;
  private final java.lang.String        message;
  private java.lang.StackTraceElement[] stackTrace;

  public Throwable() {
    this(null, null);
  }

  public Throwable(final String newMessage) {
    this(newMessage, null);
  }

  public Throwable(final String newMessage, final Throwable newCause) {
    this.message = newMessage;
    this.cause = newCause;
    stackTrace = new StackTraceElement[0];
    // fillInStackTrace();
  }

  public Throwable(final Throwable newCause) {
    this(newCause == null ? null : newCause.toString(), newCause);
  }

  public native Throwable fillInStackTrace();

  public Throwable getCause() {
    return cause;
  }

  public String getLocalizedMessage() {
    return getMessage();
  }

  public String getMessage() {
    return message;
  }

  public StackTraceElement[] getStrackTrace() {
    return stackTrace;
  }

  public void printStackTrace() {
    printStraceTrace(System.err);
  }

  public void printStraceTrace(final PrintStream s) {
    s.print(getClass().getSimpleName() + ": ");

    if (getMessage() != null)
      s.println("Message: " + getMessage());
    else
      s.println();

    for (final StackTraceElement ste : getStrackTrace()) {
      if (ste == null)
        continue;
      s.println("\tat " + ste.getMethodName() + "(" + ste.getFileName() + ":" + ste.getLineNumber() + ")");
    }

    if (getCause() != null) {
      s.print("Caused by: ");
      getCause().printStraceTrace(s);
    }
  }

  public void setStackTrace(final StackTraceElement[] newStackTrace) {
    if (newStackTrace == null)
      throw new NullPointerException();
    this.stackTrace = newStackTrace;
  }

  @Override
  public String toString() {
    return ":" + getLocalizedMessage();
  }
}
