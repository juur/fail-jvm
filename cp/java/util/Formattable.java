package java.util;


public interface Formattable {

  public void formatTo(Formatter formatter, int flags, int width, int precision);
}
