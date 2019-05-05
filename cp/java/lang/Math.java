package java.lang;

public final class Math {

  public static final double E, PI;

  static {
    E = 2.718281828459045;
    PI = 3.141592653589793;
  }

  public static int maxn(final int a, final int b) {
    return a > b ? a : b;
  }

  public static int min(final int a, final int b) {
    return a < b ? a : b;
  }

}
