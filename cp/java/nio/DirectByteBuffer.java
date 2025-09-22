package java.nio;

final class DirectByteBuffer extends ByteBuffer {

  private final boolean isReadOnly;

  DirectByteBuffer(final byte[] b, final boolean readOnly) {
    this(b.length, readOnly);
    put(b);
  }

  DirectByteBuffer(final int size, final boolean readOnly) {
    super(size, false, 0, null);
    isReadOnly = readOnly;
    nativeInit(size, readOnly);
  }

  @Override
  public CharBuffer asCharBuffer() {
    return new CharBufferView(this);
  }

  @Override
  public DoubleBuffer asDoubleBuffer() {
    return new DoubleBufferView(this);
  }

  @Override
  public FloatBuffer asFloatBuffer() {
    return new FloatBufferView(this);
  }

  @Override
  public IntBuffer asIntBuffer() {
    return new IntBufferView(this);
  }

  @Override
  public LongBuffer asLongBuffer() {
    return new LongBufferView(this);
  }

  @Override
  public ByteBuffer asReadOnlyBuffer() {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public ShortBuffer asShortBuffer() {
    return new ShortBufferView(this);
  }

  @Override
  public ByteBuffer compact() {
    return this;
  }

  @Override
  public ByteBuffer duplicate() {
    return new DirectByteBuffer(getBuffer(), isReadOnly());
  }

  @Override
  public native byte get();

  @Override
  public native byte get(final int index);

  @Override
  public native char getChar();

  @Override
  public native char getChar(final int index);

  @Override
  public native double getDouble();

  @Override
  public native double getDouble(final int index);

  @Override
  public native float getFloat();

  @Override
  public native float getFloat(final int index);

  @Override
  public native int getInt();

  @Override
  public native int getInt(final int index);

  @Override
  public native long getLong();

  @Override
  public native long getLong(final int index);

  @Override
  public native short getShort();

  @Override
  public native short getShort(final int index);

  @Override
  public boolean isDirect() {
    return true;
  }

  @Override
  public boolean isReadOnly() {
    return isReadOnly;
  }

  @Override
  public native ByteBuffer put(final byte b);

  @Override
  public native ByteBuffer put(final byte[] src);

  @Override
  public native ByteBuffer put(final int index, final byte b);

  @Override
  public native ByteBuffer putChar(final char value);

  @Override
  public native ByteBuffer putChar(final int index, final char value);

  @Override
  public native ByteBuffer putDouble(final double value);

  @Override
  public native ByteBuffer putDouble(final int index, final double value);

  @Override
  public native ByteBuffer putFloat(final float value);

  @Override
  public native ByteBuffer putFloat(final int index, final float value);

  @Override
  public native ByteBuffer putInt(final int value);

  @Override
  public native ByteBuffer putInt(final int index, final int value);

  @Override
  public native ByteBuffer putLong(final int index, final long value);

  @Override
  public native ByteBuffer putLong(final long value);

  @Override
  public native ByteBuffer putShort(final int index, final short value);

  @Override
  public native ByteBuffer putShort(final short value);

  @Override
  public ByteBuffer slice() {
    // TODO Auto-generated method stub
    return null;
  }

  private native void nativeInit(int size, boolean readOnly);

}
