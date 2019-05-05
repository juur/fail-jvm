package java.nio.channels;

import java.nio.ByteBuffer;

public interface WritableByteChannel extends Channel {

  public int write(ByteBuffer src);
}
