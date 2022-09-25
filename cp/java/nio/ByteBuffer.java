package java.nio;

public abstract class ByteBuffer extends Buffer implements Comparable<ByteBuffer> {

  public static ByteBuffer allocate(final int capacity) {
    if (capacity < 0)
      throw new IllegalArgumentException();

    return new RWByteBuffer(capacity);
  }

  public static ByteBuffer allocateDirect(final int capacity) {
    if (capacity < 0)
      throw new IllegalArgumentException();

    return new DirectRWByteBuffer(capacity);
  }

  public static ByteBuffer wrap(final byte[] array) {
    return new RWByteBuffer(array);
  }

  public static ByteBuffer wrap(final byte[] array, final int offset, final int length) {
    final ByteBuffer ret = new java.nio.RWByteBuffer(array);
    ret.position(offset);
    ret.limit(offset + length);
    return ret;
  }

  private final int     arrayOffset;
  private ByteOrder     byteOrder;
  private final boolean hasArray;

  protected byte[] buffer;

  protected ByteBuffer(final int size, final boolean newHasArray, final int newArrayOffset, final byte[] newBuffer) {
    super(size);
    byteOrder = ByteOrder.BIG_ENDIAN;
    hasArray = newHasArray;
    arrayOffset = newArrayOffset;
    buffer = newBuffer;
  }

  @Override
  public final byte[] array() {
    if (hasArray())
      return buffer;
    throw new UnsupportedOperationException();
  }

  @Override
  public final int arrayOffset() {
    if (hasArray())
      return arrayOffset;
    throw new UnsupportedOperationException();
  }

  public abstract CharBuffer asCharBuffer();

  public abstract DoubleBuffer asDoubleBuffer();

  public abstract FloatBuffer asFloatBuffer();

  public abstract IntBuffer asIntBuffer();

  public abstract LongBuffer asLongBuffer();

  public abstract ByteBuffer asReadOnlyBuffer();

  public abstract ShortBuffer asShortBuffer();

  public abstract ByteBuffer compact();

  @Override
  public int compareTo(final ByteBuffer o) {
    if (o == null)
      return -1;
    if (o.capacity() != capacity())
      return capacity() - o.capacity();
    return 0; // FIXME
  }

  public abstract ByteBuffer duplicate();

  public abstract byte get();

  public ByteBuffer get(final byte[] dst) {
    return get(dst, 0, dst.length);
  }

  public ByteBuffer get(final byte[] dst, final int offset, final int length) {
    if (length > remaining())
      throw new BufferUnderflowException();
    if (offset > dst.length || offset + length > dst.length)
      throw new IndexOutOfBoundsException();

    for (int i = offset; i < offset + length; i++)
      dst[i] = get();
    return this;
  }

  public abstract byte get(int index);

  public abstract char getChar();

  public abstract char getChar(int index);

  public abstract double getDouble();

  public abstract double getDouble(int index);

  public abstract float getFloat();

  public abstract float getFloat(int index);

  public abstract int getInt();

  public abstract int getInt(int index);

  public abstract long getLong();

  public abstract long getLong(int index);

  public abstract short getShort();

  public abstract short getShort(int index);

  @Override
  public final boolean hasArray() {
    return hasArray;
  }

  public final ByteOrder order() {
    return byteOrder;
  }

  public final ByteBuffer order(final ByteOrder bo) {
    byteOrder = bo;
    return this;
  }

  public abstract ByteBuffer put(byte b);

  public ByteBuffer put(final byte[] src) {
    return put(src, 0, src.length);
  }

  public ByteBuffer put(final byte[] src, final int offset, final int length) {
    if (isReadOnly())
      throw new ReadOnlyBufferException();
    if (length > remaining())
      throw new BufferOverflowException();
    if (offset > src.length || offset + length > src.length)
      throw new IndexOutOfBoundsException();

    for (int i = offset; i < offset + length; i++)
      put(src[i]);

    return this;
  }

  public ByteBuffer put(final ByteBuffer src) {
    if (src == this)
      throw new IllegalArgumentException();
    if (src.remaining() > remaining())
      throw new BufferOverflowException();
    if (isReadOnly())
      throw new ReadOnlyBufferException();

    while (src.hasRemaining())
      put(src.get());

    return this;
  }

  public abstract ByteBuffer put(int index, byte b);

  public abstract ByteBuffer putChar(char value);

  public abstract ByteBuffer putChar(int index, char value);

  public abstract ByteBuffer putDouble(double value);

  public abstract ByteBuffer putDouble(int index, double value);

  public abstract ByteBuffer putFloat(float value);

  public abstract ByteBuffer putFloat(int index, float value);

  public abstract ByteBuffer putInt(int value);

  public abstract ByteBuffer putInt(int index, int value);

  public abstract ByteBuffer putLong(long value);

  public abstract ByteBuffer putLong(long index, int value);

  public abstract ByteBuffer putShort(int index, short value);

  public abstract ByteBuffer putShort(short value);

  public abstract ByteBuffer slice();

}
