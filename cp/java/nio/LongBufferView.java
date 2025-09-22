package java.nio;

final class LongBufferView extends LongBuffer {

  private final ByteBuffer buf;
  private final int        bufpos;

  protected LongBufferView(final ByteBuffer str) {
    super(str.remaining());
    buf = str;
    limit(str.remaining());
    bufpos = str.position();
  }

  @Override
  public long get() {
    final long ret = buf.getLong(bufpos + position());
    position(position() + 8);
    return ret;
  }

  @Override
  public long get(final int index) {
    if (index < 0)
      throw new IndexOutOfBoundsException();

    return buf.getLong(index * 2);
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
  public LongBuffer put(final int index, final long c) {
    if (index < 0)
      throw new IndexOutOfBoundsException();
    if (isReadOnly())
      throw new ReadOnlyBufferException();

    buf.putLong(bufpos + index * 8, c);

    return this;
  }

  @Override
  public LongBuffer put(final long c) {
    if (isReadOnly())
      throw new ReadOnlyBufferException();
    buf.putLong(bufpos + position(), c);

    position(position() + 8);
    return this;
  }
}
