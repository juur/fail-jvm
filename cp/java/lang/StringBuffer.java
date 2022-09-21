package java.lang;

import java.util.Arrays;

public final class StringBuffer implements CharSequence {

  private char[] buffer;
  final private Object sync = new Object();

  public StringBuffer() {
    this(16);
  }

  public StringBuffer(final CharSequence seq) {
    // buffer = seq;
  }

  public StringBuffer(final int capacity) {
    buffer = new char[capacity];
  }

  public StringBuffer(final String str) {
    synchronized(sync) {
      buffer = str.toCharArray();
    }
  }

  @Override
  public char charAt(final int index) {
    return buffer[index];
  }

  public StringBuffer append(char[] str, int offset, int len) {
    synchronized (sync) {
      final char[] newBuffer = new char[buffer.length + len];

      for (int i = 0; i < offset; i++)
        newBuffer[i] = buffer[i];
      for (int i = 0; i < str.length; i++)
        newBuffer[i + offset] = str[i];
      for (int i = 0; i < buffer.length - offset; i++)
        newBuffer[i + offset + len] = buffer[offset + i];

      buffer = newBuffer;
    }

    return this;
  }
  
  public int capactiy() {
    return buffer.length;
  }
  
  public void ensureCapacity(int minimumCapacity) {
    synchronized(sync) {
      buffer = Arrays.copyOf(buffer, Math.max(minimumCapacity, buffer.length));
    }
  }

  public StringBuffer append(CharSequence str, int offset, int len) {
    return append(this.toString().toCharArray(), offset, len);
  }

  @Override
  public int length() {
    return buffer.length;
  }

  @Override
  public CharSequence subSequence(final int start, final int end) {
    synchronized(sync) {
      return new String(Arrays.copyOfRange(buffer, start, end));
    }
  }

}
