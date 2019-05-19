package java.lang;

public final class Math {

  public static final double E, PI;

  static {
    E = 2.718281828459045;
    PI = 3.141592653589793;
  }

  public static int max(final int a, final int b) {
    return a > b ? a : b;
  }

  public static int min(final int a, final int b) {
    return a < b ? a : b;
  }

  public static long max(final long a, final long b) {
    return a > b ? a : b;
  }

  public static long min(final long a, final long b) {
    return a < b ? a : b;
  }

  public static double max(final double a, final double b) {
    return a > b ? a : b;
  }

  public static double min(final double a, final double b) {
    return a < b ? a : b;
  }

  public static float max(final float a, final float b) {
    return a > b ? a : b;
  }

  public static float min(final float a, final float b) {
    return a < b ? a : b;
  }

  public static double abs(double a)
  {
	  if (a < 0) return -a;
	  return a;
  }

  public static float abs(float a)
  {
	  if (a < 0) return -a;
	  return a;
  }

  public static int abs(int a)
  {
	  if (a < 0) return -a;
	  return a;
  }

  public static long abs(long a)
  {
	  if (a < 0) return -a;
	  return a;
  }
  
  public static double expm(double x)
  {
	  return exp(x) - 1;
  }

  public static native double atan2(double y, double x);
  public static native double hypot(double y, double x);
  public static native double acos(double a);
  public static native double asin(double a);
  public static native double atan(double a);
  public static native double atan2(double a);
  public static native double cos(double a);
  public static native double cosh(double a);
  public static native double sin(double a);
  public static native double sinh(double a);
  public static native double exp(double a);
  public static native double log(double a);
  public static native double log10(double a);
  public static native double log1p(double x);
}
