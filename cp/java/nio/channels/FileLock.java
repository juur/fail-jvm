package java.nio.channels;

import java.io.IOException;

public abstract class FileLock {

  private final FileChannel channel;
  private final long        position;
  private final boolean     shared;
  private final long        size;

  protected FileLock(final FileChannel channel, final long position, final long size, final boolean shared) {
    this.channel = channel;
    this.position = position;
    this.size = size;
    this.shared = shared;
  }

  public final FileChannel channel() {
    return channel;
  }

  public final boolean isShared() {
    return shared;
  }

  public abstract boolean isValid();

  public final boolean overlaps(final long position, final long size) {
    if (position + size < position() || position > position() + size())
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
