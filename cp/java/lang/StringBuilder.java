package java.lang;

import java.util.Arrays;

public final class StringBuilder {

  private char[] backing;
  private int    ptr;

  public StringBuilder() {
    backing = new char[25];
    ptr = 0;
  }

  public StringBuilder append(final boolean b) {
    return append(b ? "true" : "false");
  }

  public StringBuilder append(final char c) {
    if (ptr >= backing.length) {
      backing = Arrays.copyOf(backing, backing.length * 2);
      if (backing == null)
        throw new IllegalStateException("unable to grow array");
    }
    backing[ptr++] = c;
    return this;
  }

  public StringBuilder append(final int a) {
    return append(Integer.toString(a));
  }

  public StringBuilder append(final long a) {
    return append(Long.toString(a));
  }

  public StringBuilder append(final double a) {
	  return append(Double.toString(a));
  }

  public StringBuilder append(final Object a) {
    return append(a.toString());
  }

  public StringBuilder append(final String a) {
    if (a == null)
      return append("null");

    final char[] add = a.toCharArray();
    for (final char element : add) {
      append(element);
    }
    return this;
  }

  @Override
  public String toString() {
    return new String(Arrays.copyOf(backing, ptr));
  }
}
