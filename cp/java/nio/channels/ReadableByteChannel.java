package java.nio.channels;

import java.nio.ByteBuffer;

public interface ReadableByteChannel extends Channel {

  public int read(ByteBuffer dst);
}
