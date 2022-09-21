package java.lang;

import java.io.IOException;
import java.util.Arrays;

public final class StringBuilder implements Appendable, CharSequence {

  private char[] backing;
  private int    ptr;

  public StringBuilder() {
    backing = new char[25];
    ptr = 0;
  }

  public StringBuilder append(final boolean b) {
    return append(b ? "true" : "false");
  }

  @Override
  public StringBuilder append(final char c) {
    if (ptr >= backing.length) {
      backing = Arrays.copyOf(backing, backing.length * 2);
      if (backing == null)
        throw new IllegalStateException("unable to grow array");
    }
    backing[ptr++] = c;
    return this;
  }

  @Override
  public StringBuilder append(final CharSequence csq) throws IOException {
    if (csq == null)
      return append("null");

    for (int i = 0; i < csq.length(); i++) {
      append(csq.charAt(i));
    }
    return this;
  }

  @Override
  public StringBuilder append(final CharSequence csq, final int start, final int end)
    throws IOException {
    if (csq == null)
      return append("null");

    for (int i = start; i < end; i++) {
      append(csq.charAt(i));
    }
    return this;
  }

  public StringBuilder append(final double a) {
    return append(Double.toString(a));
  }

  public StringBuilder append(final int a) {
    return append(Integer.toString(a));
  }

  public StringBuilder append(final long a) {
    return append(Long.toString(a));
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
  public char charAt(final int index) {
    return backing[index];
  }


  @Override
  public int length() {
    return ptr;
  }


  @Override
  public CharSequence subSequence(final int start, final int end) {
    return null;
  }


  @Override
  public String toString() {
    return new String(Arrays.copyOf(backing, ptr));
  }
}
