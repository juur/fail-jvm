package java.lang;

import java.nio.ByteBuffer;
import java.nio.charset.CharacterCodingException;
import java.nio.charset.Charset;

public final class String implements CharSequence, Comparable<String> {

  public static String valueOf(final boolean b) {
    return Boolean.toString(b);
  }

  public static String valueOf(final double b) {
    return Double.toString(b);
  }

  public static String valueOf(final float b) {
    return Float.toString(b);
  }

  public static String valueOf(final int a) {
    return Integer.toString(a);
  }

  public static String valueOf(final long b) {
    return Long.toString(b);
  }

  public static String valueOf(final Object o) {
    return o.toString();
  }

  private final char[] string;

  public String(final byte[] bytes) {
    this(bytes, Charset.defaultCharset());
  }

  public String(final byte[] bytes, final Charset charset) {
    try {
      string = charset.decode(ByteBuffer.wrap(bytes)).array();
    } catch (final CharacterCodingException e) {
      throw new Error(e);
    }
  }

  public String(final char[] s) {
    string = s;
  }

  public String(final String s) {
    this(s.toCharArray());
  }

  @Override
  public char charAt(final int index) {
    return string[index];
  }

  @Override
  public int compareTo(final String o) {
    if (length() != o.length())
      return length() - o.length();
    for (int i = 0; i < length(); i++)
      if (string[i] != o.string[i])
        return -1;
    return 0;
  }

  public boolean equalsIgnoreCase(final String s) {
    if (s.length() != length())
      return false;
    if (length() == 0)
      return true;
    for (int i = 0; i < s.length(); i++)
      if (Character.toLowerCase(string[i]) != Character.toLowerCase(s.string[i]))
        return false;
    return true;
  }

  public byte[] getBytes() {
    final byte[] ret = new byte[length()];
    for (int i = 0; i < length(); i++)
      ret[i] = (byte) string[i];
    return ret;
  }

  @Override
  public int hashCode() {
    int hc = 31;
    for (final char c : string)
      hc += c;
    return hc;
  }

  public int indexOf(final int ch) {
    return indexOf(ch, 0);
  }

  public int indexOf(final int ch, final int fromIndex) {
    for (int i = fromIndex; i < length(); i++)
      if (string[i] == ch)
        return i;
    return -1;
  }

  public boolean isEmpty() {
    return length() == 0;
  }

  @Override
  public int length() {
    return string.length;
  }

  public boolean startsWith(final String testString) {
    return startsWith(testString, testString.length());
  }

  public boolean startsWith(final String other, final int len) {
    int length = len;
    if (length > other.length())
      length = other.length();
    if (length == 0)
      return true;
    if (length > length())
      return false;

    for (int i = 0; i < length; i++)
      if (string[i] != other.string[i])
        return false;
    return true;
  }

  @Override
  public CharSequence subSequence(final int start, final int end) {
    return substring(start, end);
  }

  public String substring(final int start)
  {
    return substring(start, length());
  }

  public String substring(final int start, final int end) {
    char[] ret = new char[end-start];
    int i = 0;
    for (int j = start; j < end; j++)
      ret[i++] = string[j];
    return new String(ret);
  }

  public char[] toCharArray() {
    return string;
  }

  @Override
  public String toString() {
    return this;
  }
};
