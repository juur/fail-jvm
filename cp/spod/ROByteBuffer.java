package spod;

import java.nio.*;

public class ROByteBuffer extends ByteBuffer {

  public ROByteBuffer(final int size) {
    super(size, true, 0, new byte[size]);
  }

  ROByteBuffer(final byte[] buffer) {
    super(buffer.length, true, 0, buffer);
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
    // TODO Auto-generated method stub
    return null;
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
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public char getChar(final int index) {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public double getDouble() {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public double getDouble(final int index) {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public float getFloat() {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public float getFloat(final int index) {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public int getInt() {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public int getInt(final int index) {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public short getShort() {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public short getShort(final int index) {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public boolean isDirect() {
    // TODO Auto-generated method stub
    return false;
  }

  @Override
  public boolean isReadOnly() {
    return true;
  }

  @Override
  public ByteBuffer put(final byte b) {
    throw new ReadOnlyBufferException();
  }

  @Override
  public ByteBuffer put(final int index, final byte b) {
    throw new ReadOnlyBufferException();
  }

  @Override
  public ByteBuffer putChar(final char value) {
    throw new ReadOnlyBufferException();

  }

  @Override
  public ByteBuffer putChar(final int index, final char value) {
    throw new ReadOnlyBufferException();

  }

  @Override
  public ByteBuffer putDouble(final double value) {
    throw new ReadOnlyBufferException();

  }

  @Override
  public ByteBuffer putDouble(final int index, final double value) {
    throw new ReadOnlyBufferException();

  }

  @Override
  public ByteBuffer putFloat(final float value) {
    throw new ReadOnlyBufferException();

  }

  @Override
  public ByteBuffer putFloat(final int index, final float value) {
    throw new ReadOnlyBufferException();

  }

  @Override
  public ByteBuffer putInt(final int value) {
    throw new ReadOnlyBufferException();

  }

  @Override
  public ByteBuffer putInt(final int index, final int value) {
    throw new ReadOnlyBufferException();

  }

  @Override
  public ByteBuffer putShort(final int index, final short value) {
    throw new ReadOnlyBufferException();

  }

  @Override
  public ByteBuffer putShort(final short value) {
    throw new ReadOnlyBufferException();

  }

  @Override
  public ByteBuffer slice() {
    // TODO Auto-generated method stub
    return null;
  }

}
