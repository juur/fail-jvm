package java.nio;

import java.util.Arrays;

final class RWCharBuffer extends CharBuffer {

  RWCharBuffer(final CharBuffer cb) {
    super(cb.length(), true, 0, Arrays.copyOf(cb.array(), cb.capacity()));
  }

  RWCharBuffer(final int size) {
    super(size, true, 0, new char[size]);
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
    return new RWCharBuffer(this);
  }

  @Override
  public char get() {
    if (position() >= limit())
      throw new BufferUnderflowException();
    final char ret = buffer[position()];
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
    return buffer[index];
  }

  @Override
  public boolean isDirect() {
    return false;
  }

  @Override
  public boolean isReadOnly() {
    return false;
  }

  @Override
  public CharBuffer put(final char c) {
    if (!hasRemaining())
      throw new BufferOverflowException();
    buffer[position()] = c;
    position(position() + 1);
    return this;
  }

  @Override
  public CharBuffer put(final char[] src, final int offset, final int length) {
    if (isReadOnly())
      throw new ReadOnlyBufferException();
    if (length > remaining())
      throw new BufferOverflowException();
    if (offset > src.length || offset + length > src.length)
      throw new IndexOutOfBoundsException();

    System.arraycopy(src, offset, buffer, position(), length);
    position(position() + length);

    return this;
  }

  @Override
  public CharBuffer put(final int index, final char c) {
    if (index < 0 || index > limit())
      throw new IndexOutOfBoundsException();
    buffer[index] = c;
    return this;
  }

  @Override
  public CharBuffer slice() {
    throw new UnsupportedOperationException();

  }

  @Override
  public CharSequence subSequence(final int start, final int end) {
    throw new UnsupportedOperationException();

  }
}
