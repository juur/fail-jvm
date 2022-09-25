package java.lang;

import java.io.Serializable;
import java.nio.ByteBuffer;
import java.nio.charset.CharacterCodingException;
import java.nio.charset.Charset;
import java.util.Arrays;
import java.util.Comparator;
import java.util.Locale;
import java.util.regex.Pattern;

import fail.CaseInsensitiveOrder;

public final class String implements Serializable, CharSequence, Comparable<String> {

  public static final Comparator<String> CASE_INSENSITIVE_ORDER;

  static {
    CASE_INSENSITIVE_ORDER = new CaseInsensitiveOrder();
  }

  public static String copyValueOf(final char data[]) {
    return valueOf(Arrays.copyOf(data, data.length));
  }

  public static String copyValueOf(final char data[], final int offset, final int count) {
    return valueOf(data, offset, count);
  }

  public static String valueOf(final boolean b) {
    return Boolean.toString(b);
  }

  public static String valueOf(final char c) {
    return Character.toString(c);
  }

  public static String valueOf(final char[] data) {
    return new String(data);
  }

  public static String valueOf(final char[] data, final int offset, final int count) {
    return new String(Arrays.copyOfRange(data, offset, count));
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

  public int codePointAt(final int i) {
    // FIXME
    return charAt(i);
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

  public boolean endsWith(final String suffix) {
    if (suffix.length() > length())
      return false;
    if (suffix.length() == length())
      return equals(suffix);

    final int offset = length() - suffix.length();

    for (int i = 0; i < suffix.length(); i++)
      if (charAt(offset + i) != suffix.charAt(i))
        return false;

    return true;
  }

  @Override
  public boolean equals(final Object obj) {
    if (this == obj) {
      return true;
    }
    if (!(obj instanceof String)) {
      return false;
    }
    final String other = (String) obj;
    if (!Arrays.equals(string, other.string)) {
      return false;
    }
    return true;
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

  public String intern() {
    /* FIXME */
    return this;
  }

  public boolean isEmpty() {
    return length() == 0;
  }

  public int lastIndexOf(final int ch) {
    return lastIndexOf(ch, string.length - 1);
  }

  public int lastIndexOf(final int ch, final int fromIndex) {
    for (int i = fromIndex; i >= 0; i--)
      if (string[i] == ch)
        return i;
    return -1;
  }

  @Override
  public int length() {
    return string.length;
  }

  public boolean matches(final String regex) {
    return Pattern.matches(regex, this);
  }

  public String replace(final char oldChar, final char newChar) {
    final char newstr[] = Arrays.copyOf(string, string.length);
    for (int i = 0; i < newstr.length; i++)
      if (newstr[i] == oldChar)
        newstr[i] = newChar;
    return new String(newstr);
  }

  public String replaceAll(final String regex, final String replacement) {
    return Pattern.compile(regex).matcher(this).replaceAll(replacement);
  }

  public String replaceFirst(final String regex, final String replacement) {
    return Pattern.compile(regex).matcher(this).replaceFirst(replacement);
  }

  public String[] split(final String regex) {
    return Pattern.compile(regex).split(this);
  }

  public String[] split(final String regex, final int limit) {
    return Pattern.compile(regex).split(this, limit);
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

  public String substring(final int start) {
    return substring(start, length());
  }

  public String substring(final int start, final int end) {
    final char[] ret = new char[end - start];
    int          i   = 0;
    for (int j = start; j < end; j++)
      ret[i++] = string[j];
    return new String(ret);
  }

  public char[] toCharArray() {
    return Arrays.copyOf(string, string.length);
  }

  public String toLowerCase() {
    return toLowerCase(Locale.getDefault());
  }

  public String toLowerCase(final Locale locale) {
    final StringBuilder sb = new StringBuilder(length());
    for (int i = 0; i < length(); i++)
      sb.append(Character.toLowerCase(codePointAt(i)));
    return sb.toString();
  }

  @Override
  public String toString() {
    return this;
  }

  public String toUpperCase() {
    return toUpperCase(Locale.getDefault());
  }

  public String toUpperCase(final Locale locale) {
    final StringBuilder sb = new StringBuilder(length());
    for (int i = 0; i < length(); i++)
      sb.append(Character.toUpperCase(codePointAt(i)));
    return sb.toString();
  }
};
