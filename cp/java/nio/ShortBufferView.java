package java.nio;

final class ShortBufferView extends ShortBuffer {

  private final ByteBuffer buf;
  private final int        bufpos;

  protected ShortBufferView(final ByteBuffer str) {
    super(str.remaining());
    buf = str;
    limit(str.remaining());
    bufpos = str.position();
  }

  @Override
  public short get() {
    final short ret = buf.getShort(bufpos + position());
    position(position() + 2);
    return ret;
  }

  @Override
  public short get(final int index) {
    if (index < 0)
      throw new IndexOutOfBoundsException();

    return buf.getShort(index * 2);
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
  public ShortBuffer put(final int index, final short c) {
    if (index < 0)
      throw new IndexOutOfBoundsException();
    if (isReadOnly())
      throw new ReadOnlyBufferException();

    buf.putShort(bufpos + index * 2, c);

    return this;
  }

  @Override
  public ShortBuffer put(final short c) {
    if (isReadOnly())
      throw new ReadOnlyBufferException();
    buf.putInt(bufpos + position(), c);

    position(position() + 2);
    return this;
  }
}
