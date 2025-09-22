package java.util;


public class MissingResourceException extends RuntimeException {

  private final String className;
  private final String key;

  public MissingResourceException(final String s, final String className, final String key) {
    super(s + " for " + className + " on key " + key);
    this.className = className;
    this.key = key;
  }

  public String getClassName() {
    return className;
  }

  public String getKey() {
    return key;
  }
}
