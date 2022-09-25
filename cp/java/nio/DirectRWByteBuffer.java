package java.nio;

final class DirectRWByteBuffer extends ByteBuffer {

  DirectRWByteBuffer(final byte[] b) {
    this(b.length);
    put(b);
  }

  DirectRWByteBuffer(final int size) {
    super(size, false, 0, null);
    nativeInit(size);
  }

  @Override
  public CharBuffer asCharBuffer() {
    return new CharBufferView(this);
  }

  @Override
  public DoubleBuffer asDoubleBuffer() {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public FloatBuffer asFloatBuffer() {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public IntBuffer asIntBuffer() {
    return new IntBufferView(this);
  }

  @Override
  public LongBuffer asLongBuffer() {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public ByteBuffer asReadOnlyBuffer() {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public ShortBuffer asShortBuffer() {
    // TODO Auto-generated method stub
    return null;
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
  public native byte get();

  @Override
  public native byte get(final int index);

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
  public long getLong() {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public long getLong(final int index) {
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
    return true;
  }

  @Override
  public boolean isReadOnly() {
    return false;
  }

  @Override
  public ByteBuffer put(final byte b) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public native ByteBuffer put(byte[] src);

  @Override
  public native ByteBuffer put(final int index, final byte b);

  @Override
  public ByteBuffer putChar(final char value) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public ByteBuffer putChar(final int index, final char value) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public ByteBuffer putDouble(final double value) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public ByteBuffer putDouble(final int index, final double value) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public ByteBuffer putFloat(final float value) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public ByteBuffer putFloat(final int index, final float value) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public ByteBuffer putInt(final int value) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public ByteBuffer putInt(final int index, final int value) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public ByteBuffer putLong(final long value) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public ByteBuffer putLong(final long index, final int value) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public ByteBuffer putShort(final int index, final short value) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public ByteBuffer putShort(final short value) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public ByteBuffer slice() {
    // TODO Auto-generated method stub
    return null;
  }

  private native void nativeInit(int size);

}
