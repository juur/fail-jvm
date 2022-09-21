package java.nio.channels;

import java.io.IOException;

public abstract class FileLock {

  private final FileChannel channel;
  private final long        position;
  private final boolean     shared;
  private final long        size;

  protected FileLock(final FileChannel newChannel, final long newPosition, final long newSize, final boolean isShared) {
    this.channel = newChannel;
    this.position = newPosition;
    this.size = newSize;
    this.shared = isShared;
  }

  public final FileChannel channel() {
    return channel;
  }

  public final boolean isShared() {
    return shared;
  }

  public abstract boolean isValid();

  public final boolean overlaps(final long checkPosition, final long checkSize) {
    if (checkPosition + checkSize < position() || checkPosition > position() + size())
      return false;
    return true;
  }

  public final long position() {
    return position;
  }

  public abstract void release() throws IOException;

  public final long size() {
    return size;

  }

}
