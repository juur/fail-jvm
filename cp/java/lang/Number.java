package java.lang;

public abstract class Number {

  private static int isValidRadix(final char c, final int radix) {
    if (radix <= 0 || radix > 36)
      throw new IllegalArgumentException();
    if (c >= '0' && c <= '0' + Math.max(radix, 10))
      return c - '0';
    if (radix > 10) {
      if (c >= 'a' && c <= 'a' + Math.max(radix - 10, 26))
        return 10 + c - 'a';
      if (c >= 'A' && c <= 'A' + Math.max(radix - 10, 26))
        return 10 + c - 'A';
    }

    return -1;
  }

  protected static long atoi(final char[] str, final int radix, final long max, final long min)
    throws NumberFormatException {
    long sign = 1, base = 0;
    int  i    = 0;

    if (str == null)
      throw new NullPointerException();

    while (str[i] == ' ') {
      i++;
    }

    if (str[i] == '-' || str[i] == '+') {
      sign = 1 - 2 * (str[i++] == '-' ? 1 : 0);
    }

    int val = -1;

    while (i < str.length && (val = isValidRadix(str[i], radix)) != -1) {
      if (base > max / radix || base == max / radix && val > 7) {
        if (sign == 1)
          return max;
        return min;
      }
      base = radix * base + val;
      i++;
    }
    if (val == -1)
      throw new NumberFormatException();

    return base * sign;
  }

  protected static char[] itoa(final long value, final int radix) {
    final char[] tmp = new char[64];
    final char[] ret = new char[tmp.length+1];

    int  tp = 0, sp = 0;
    long i  = 0, v = 0;

    final boolean sign = radix == 10 && value < 0;

    //if (sign)
    //  v = -value;
    //else
      v = value;

    while (v != 0 || tp == 0) {
      i = Math.abs(v % radix);
      v /= radix;
      if (i < 10)
        tmp[tp++] = (char) (i + 48);
      else
        tmp[tp++] = (char) (i + 97 - 10);
    }

    if (sign)
      ret[sp++] = '-';

    while (tp > 0)
      ret[sp++] = tmp[--tp];

    return ret;
  }

  public byte byteValue() {
    return (byte) intValue();
  }

  public abstract double doubleValue();

  public abstract float floatValue();

  public abstract int intValue();

  public abstract long longValue();

  public short shortValue() {
    return (short) intValue();
  }
}
