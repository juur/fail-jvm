package java.lang;

import java.io.PrintStream;

public class Throwable {

  private final Throwable               cause;
  private final java.lang.String        message;
  private java.lang.StackTraceElement[] stackTrace;

  public Throwable() {
    this(null, null);
  }

  public Throwable(final String message) {
    this(message, null);
  }

  public Throwable(final String message, final Throwable cause) {
    this.message = message;
    this.cause = cause;
    stackTrace = new StackTraceElement[0];
    // fillInStackTrace();
  }

  public Throwable(final Throwable cause) {
    this(cause == null ? null : cause.toString(), cause);
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

  public void setStackTrace(final StackTraceElement[] stackTrace) {
    if (stackTrace == null)
      throw new NullPointerException();
    this.stackTrace = stackTrace;
  }

  @Override
  public String toString() {
    return ":" + getLocalizedMessage();
  }
}
