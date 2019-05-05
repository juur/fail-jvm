package spod;

import java.nio.BufferOverflowException;
import java.nio.BufferUnderflowException;
import java.nio.CharBuffer;
import java.util.Arrays;

public class RWCharBuffer extends CharBuffer {

  public RWCharBuffer(final CharBuffer cb) {
    super(cb.length(), true, 0, Arrays.copyOf(cb.array(), cb.capacity()));
  }

  public RWCharBuffer(final int size) {
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
