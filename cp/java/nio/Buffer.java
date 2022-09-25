package java.nio;

public abstract class Buffer {

  private final int capacity;
  private int       limit;
  private int       mark;
  private int       position;

  Buffer(final int size) {
    capacity = size;
    position = 0;
    limit = capacity;
    mark = -1;
  }

  public abstract Object array();

  public abstract int arrayOffset();

  public final int capacity() {
    return capacity;
  }

  public final Buffer clear() {
    limit = capacity;
    position = 0;
    mark = -1;
    return this;
  }

  public final Buffer flip() {
    limit = position;
    position = 0;
    mark = -1;
    return this;
  }

  public abstract boolean hasArray();

  public final boolean hasRemaining() {
    return remaining() > 0;
  }

  public abstract boolean isDirect();

  public abstract boolean isReadOnly();

  public final int limit() {
    return limit;
  }

  public final Buffer limit(final int newLimit) {
    limit = newLimit;
    if (position > limit)
      position = limit;
    if (mark > limit)
      mark = -1;
    return this;
  }

  public final Buffer mark() {
    mark = position;
    return this;
  }

  public final int position() {
    return position;
  }

  public final Buffer position(final int newPosition) {
    if (newPosition < 0 || newPosition > limit())
      throw new IllegalArgumentException();
    if (mark > newPosition)
      mark = -1;
    position = newPosition;
    return this;
  }

  public final int remaining() {
    return limit() - position();
  }

  public final Buffer reset() {
    position = mark;
    return this;
  }

  public final Buffer rewind() {
    position = 0;
    mark = -1;
    return this;
  }

}
