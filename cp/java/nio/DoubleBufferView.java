package java.nio;

final class DoubleBufferView extends DoubleBuffer {

  private final ByteBuffer buf;
  private final int        bufpos;

  protected DoubleBufferView(final ByteBuffer str) {
    super(str.remaining());
    buf = str;
    limit(str.remaining());
    bufpos = str.position();
  }

  @Override
  public double get() {
    final double ret = buf.getDouble(bufpos + position());
    position(position() + 8);
    return ret;
  }

  @Override
  public double get(final int index) {
    if (index < 0)
      throw new IndexOutOfBoundsException();

    return buf.getDouble(index * 8);
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
  public DoubleBuffer put(final double c) {
    if (isReadOnly())
      throw new ReadOnlyBufferException();
    buf.putDouble(bufpos + position(), c);

    position(position() + 8);
    return this;
  }

  @Override
  public DoubleBuffer put(final int index, final double c) {
    if (index < 0)
      throw new IndexOutOfBoundsException();
    if (isReadOnly())
      throw new ReadOnlyBufferException();

    buf.putDouble(bufpos + index * 4, c);

    return this;
  }

}
