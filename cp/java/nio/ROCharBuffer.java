package java.nio;

import java.util.Arrays;

final class ROCharBuffer extends CharBuffer {

  ROCharBuffer(final char[] array) {
    super(array.length, true, 0, array);
  }

  ROCharBuffer(final CharBuffer cb) {
    super(cb.limit(), true, 0, Arrays.copyOf(cb.array(), cb.limit()));
  }

  ROCharBuffer(final CharSequence str) {
    super(str);
  }

  @Override
  public CharBuffer asReadOnlyBuffer() {
    return new ROCharBuffer(this);
  }

  @Override
  public CharBuffer compact() {
    return this;
  }

  @Override
  public CharBuffer duplicate() {
    return new ROCharBuffer(this);
  }

  @Override
  public char get() {
    char ret;

    if (hasArray())
      ret = buffer[position()];
    else
      ret = csqBuffer.charAt(position());

    position(position() + 1);
    return ret;
  }

  @Override
  public CharBuffer get(final char[] dst, final int offset, final int length) {
    if (length > remaining())
      throw new BufferUnderflowException();
    if (offset > dst.length || offset + length > dst.length)
      throw new IndexOutOfBoundsException();

    System.arraycopy(buffer, position(), dst, offset, length);
    position(position() + length);

    return this;
  }

  @Override
  public char get(final int index) {
    if (index < 0 || index > limit())
      throw new IndexOutOfBoundsException();
    if (hasArray())
      return buffer[index];
    return csqBuffer.charAt(index);
  }

  @Override
  public boolean isDirect() {
    return false;
  }

  @Override
  public boolean isReadOnly() {
    return true;
  }

  @Override
  public CharBuffer put(final char c) {
    throw new ReadOnlyBufferException();
  }

  @Override
  public CharBuffer put(final int index, final char c) {
    throw new ReadOnlyBufferException();
  }

  @Override
  public CharBuffer slice() {
    return null; // FIXME
  }

  @Override
  public CharSequence subSequence(final int start, final int end) {
    return null; // FIXME
  }
}
