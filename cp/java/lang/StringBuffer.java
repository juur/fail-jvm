package java.lang;

public final class StringBuffer implements CharSequence {

  public StringBuffer() {
    this(16);
  }

  public StringBuffer(final CharSequence seq) {}

  public StringBuffer(final int capacity) {}

  public StringBuffer(final String str) {}

  @Override
  public char charAt(final int index) {
    return 0;
  }

  @Override
  public int length() {
    return 0;
  }

  @Override
  public CharSequence subSequence(final int start, final int end) {
    return null;
  }

}
