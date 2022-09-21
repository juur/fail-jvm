package java.lang;

public class IllegalArgumentException extends RuntimeException {

  public IllegalArgumentException() {
    super();
  }

  public IllegalArgumentException(final String message) {
    super(message);
  }
  
  public IllegalArgumentException(Throwable cause) {
    super(cause);
  }
}
