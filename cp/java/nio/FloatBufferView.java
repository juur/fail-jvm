package java.nio;

final class FloatBufferView extends FloatBuffer {

  private final ByteBuffer buf;
  private final int        bufpos;

  protected FloatBufferView(final ByteBuffer str) {
    super(str.remaining());
    buf = str;
    limit(str.remaining());
    bufpos = str.position();
  }

  @Override
  public float get() {
    final float ret = buf.getFloat(bufpos + position());
    position(position() + 4);
    return ret;
  }

  @Override
  public float get(final int index) {
    if (index < 0)
      throw new IndexOutOfBoundsException();

    return buf.getFloat(index * 4);
  }

  @Override
  public boolean isDirect() {
    return buf.isDirect();
  }

  @Override
  public boolean isReadOnly() {
    return buf.isReadOnly();
  }

  @Override
  public FloatBuffer put(final float c) {
    if (isReadOnly())
      throw new ReadOnlyBufferException();
    buf.putFloat(bufpos + position(), c);

    position(position() + 4);
    return this;
  }

  @Override
  public FloatBuffer put(final int index, final float c) {
    if (index < 0)
      throw new IndexOutOfBoundsException();
    if (isReadOnly())
      throw new ReadOnlyBufferException();

    buf.putFloat(bufpos + index * 4, c);

    return this;
  }

}
