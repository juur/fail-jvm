package java.nio;

final class IntBufferView extends IntBuffer {

  private final ByteBuffer buf;
  private final int        bufpos;

  protected IntBufferView(final ByteBuffer str) {
    super(str.remaining());
    buf = str;
    limit(str.remaining());
    bufpos = str.position();
  }

  @Override
  public int get() {
    final int ret = buf.getInt(bufpos + position());
    position(position() + 4);
    return ret;
  }

  @Override
  public int get(final int index) {
    if (index < 0)
      throw new IndexOutOfBoundsException();

    return buf.getInt(index * 4);
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
  public IntBuffer put(final int c) {
    if (isReadOnly())
      throw new ReadOnlyBufferException();
    buf.putInt(bufpos + position(), c);

    position(position() + 4);
    return this;
  }

  @Override
  public IntBuffer put(final int index, final int c) {
    if (index < 0)
      throw new IndexOutOfBoundsException();
    if (isReadOnly())
      throw new ReadOnlyBufferException();

    buf.putInt(bufpos + index * 4, c);

    return this;
  }
}
