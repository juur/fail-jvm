package java.nio;

class CharBufferView extends CharBuffer {

  private final ByteBuffer buf;
  private final int        bufpos;

  protected CharBufferView(final ByteBuffer str) {
    super(str.remaining());
    buf = str;
    limit(str.remaining());
    bufpos = str.position();
  }

  @Override
  public CharBuffer asReadOnlyBuffer() {
    throw new UnsupportedOperationException();
  }

  @Override
  public CharBuffer compact() {
    return this;
  }

  @Override
  public CharBuffer duplicate() {
    throw new UnsupportedOperationException();
  }

  @Override
  public char get() {
    final char ret = buf.getChar(bufpos + position());
    position(position() + 2);
    return ret;
  }

  @Override
  public char get(final int index) {
    if (index < 0)
      throw new IndexOutOfBoundsException();

    return buf.getChar(index * 2);
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
  public CharBuffer put(final char c) {
    if (isReadOnly())
      throw new ReadOnlyBufferException();
    buf.putChar(bufpos + position(), c);
    position(position() + 2);
    return this;
  }

  @Override
  public CharBuffer put(final int index, final char c) {
    if (index < 0)
      throw new IndexOutOfBoundsException();
    if (isReadOnly())
      throw new ReadOnlyBufferException();
    buf.putChar(bufpos + index * 2, c);
    return this;
  }

  @Override
  public CharBuffer slice() {
    throw new UnsupportedOperationException();
  }

  @Override
  public CharSequence subSequence(final int start, final int end) {
    final byte[] tmp = new byte[(end - start) * 2];
    buf.get(tmp, start * 2, tmp.length);
    return new String(tmp);
  }

}