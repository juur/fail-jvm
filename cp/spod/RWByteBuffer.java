package spod;

import java.nio.*;

public class RWByteBuffer extends ByteBuffer {

  public RWByteBuffer(final byte[] buffer) {
    super(buffer.length, true, 0, buffer);
  }

  public RWByteBuffer(final int size) {
    super(size, true, 0, new byte[size]);
  }

  @Override
  public CharBuffer asCharBuffer() {
    throw new UnsupportedOperationException();
  }

  @Override
  public DoubleBuffer asDoubleBuffer() {
    throw new UnsupportedOperationException();
  }

  @Override
  public FloatBuffer asFloatBuffer() {
    throw new UnsupportedOperationException();

  }

  @Override
  public IntBuffer asIntBuffer() {
    throw new UnsupportedOperationException();
  }

  @Override
  public LongBuffer asLongBuffer() {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuffer asReadOnlyBuffer() {
    return new ROByteBuffer(buffer);
  }

  @Override
  public ShortBuffer asShortBuffer() {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuffer compact() {
    return this;
  }

  @Override
  public ByteBuffer duplicate() {
    return new RWByteBuffer(buffer);
  }

  @Override
  public byte get() {
    final byte ret = buffer[position()];
    position(position() + 1);
    return ret;
  }

  @Override
  public byte get(final int index) {
    if (position() >= limit())
      throw new BufferUnderflowException();
    return buffer[index];
  }

  @Override
  public char getChar() {
    throw new UnsupportedOperationException();
  }

  @Override
  public char getChar(final int index) {
    throw new UnsupportedOperationException();
  }

  @Override
  public double getDouble() {
    throw new UnsupportedOperationException();
  }

  @Override
  public double getDouble(final int index) {
    throw new UnsupportedOperationException();
  }

  @Override
  public float getFloat() {
    throw new UnsupportedOperationException();
  }

  @Override
  public float getFloat(final int index) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int getInt() {
    throw new UnsupportedOperationException();
  }

  @Override
  public int getInt(final int index) {
    throw new UnsupportedOperationException();
  }

  @Override
  public short getShort() {
    throw new UnsupportedOperationException();
  }

  @Override
  public short getShort(final int index) {
    throw new UnsupportedOperationException();
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
  public ByteBuffer put(final byte b) {
    if (!hasRemaining())
      throw new BufferOverflowException();
    buffer[position()] = b;
    position(position() + 1);
    return this;
  }

  @Override
  public ByteBuffer put(final int index, final byte b) {
    if (index < 0 || index > limit())
      throw new IndexOutOfBoundsException();
    buffer[index] = b;
    return this;
  }

  @Override
  public ByteBuffer putChar(final char value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuffer putChar(final int index, final char value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuffer putDouble(final double value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuffer putDouble(final int index, final double value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuffer putFloat(final float value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuffer putFloat(final int index, final float value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuffer putInt(final int value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuffer putInt(final int index, final int value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuffer putShort(final int index, final short value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuffer putShort(final short value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuffer slice() {
    throw new UnsupportedOperationException();
  }

}
