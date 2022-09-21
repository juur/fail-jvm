package java.nio;

import java.io.IOException;

import spod.ROCharBuffer;

public abstract class CharBuffer extends Buffer implements Readable, Appendable, CharSequence, Comparable<CharBuffer> {

  public static CharBuffer allocate(final int capacity) {
    if (capacity < 0)
      throw new IllegalArgumentException();

    return new spod.RWCharBuffer(capacity);
  }

  public static CharBuffer wrap(final char[] array) {
    return new ROCharBuffer(array);
  }

  public static CharBuffer wrap(final char[] array, final int offset, final int length) {
    final CharBuffer ret = new ROCharBuffer(array);
    ret.limit(offset + length);
    ret.position(offset);
    return ret;
  }

  public static CharBuffer wrap(final CharSequence str) {
    return new ROCharBuffer(str);
  }

  public static CharBuffer wrap(final CharSequence str, final int start, final int end) {
    final CharBuffer ret = new ROCharBuffer(str);
    ret.limit(end);
    ret.position(start);
    return ret;
  }

  private final int      arrayOffset;
  private final boolean  hasArray;
  protected char[]       buffer;
  protected CharSequence csqBuffer;

  protected CharBuffer(final CharSequence str) {
    this(str.length(), false, 0, null);
    csqBuffer = str;
  }

  public CharBuffer(final int size, final boolean newHasArray, final int newArrayOffset, final char[] newBuffer) {
    super(size);
    hasArray = newHasArray;
    arrayOffset = newArrayOffset;
    if (newHasArray)
      buffer = newBuffer;
    else
      buffer = null;
  }

  @Override
  public CharBuffer append(final char c) throws IOException {
    return put(c);
  }

  @Override
  public CharBuffer append(final CharSequence csq) throws IOException {
    return put(csq.toString());
  }

  @Override
  public CharBuffer append(final CharSequence csq, final int start, final int end) throws IOException {
    return put(csq.subSequence(start, end).toString());
  }

  @Override
  public final char[] array() {
    if (hasArray())
      return buffer;
    throw new UnsupportedOperationException();
  }

  @Override
  public int arrayOffset() {
    if (hasArray())
      return arrayOffset;
    throw new UnsupportedOperationException();
  }

  public abstract CharBuffer asReadOnlyBuffer();

  @Override
  public char charAt(final int index) {
    return 0;
  }

  public abstract CharBuffer compact();

  @Override
  public int compareTo(final CharBuffer o) {
    if (o == null)
      return -1;
    if (o.length() != length())
      return length() - o.length();
    return 0; // FIXME
  }

  public abstract CharBuffer duplicate();

  public abstract char get();

  public CharBuffer get(final char[] dst) {
    return get(dst, 0, dst.length);
  }

  public CharBuffer get(final char[] dst, final int offset, final int length) {
    if (length() < length)
      throw new BufferUnderflowException();
    if (offset < 0 || length < 0 || offset + length > dst.length)
      throw new IndexOutOfBoundsException();

    for (int i = offset; i < offset + length; i++)
      dst[i] = get();
    return this;
  }

  public abstract char get(int index);

  @Override
  public final boolean hasArray() {
    return hasArray;
  }

  @Override
  public int length() {
    return remaining();
  }

  public abstract CharBuffer put(char c);

  public CharBuffer put(final char[] src) {
    return put(src, 0, src.length);
  }

  public CharBuffer put(final char[] src, final int offset, final int length) {
    for (int i = offset; i < offset + length; i++)
      put(src[i]);
    return this;
  }

  public CharBuffer put(final CharBuffer src) {
    while (src.hasRemaining())
      put(src.get());
    return this;
  }

  public abstract CharBuffer put(int index, char c);

  public CharBuffer put(final String src) {
    return put(src, 0, src.length());
  }

  public CharBuffer put(final String src, final int start, final int end) {
    for (int i = start; i < end; i++)
      put(src.charAt(i));
    return this;
  }

  @Override
  public int read(final CharBuffer cb) {
    if (!hasRemaining())
      return -1;

    if (cb == null)
      throw new NullPointerException();
    if (cb.isReadOnly())
      throw new ReadOnlyBufferException();

    int count = 0;
    while (hasRemaining()) {
      cb.put(get());
      count++;
    }

    return count;

  }

  public abstract CharBuffer slice();
}
