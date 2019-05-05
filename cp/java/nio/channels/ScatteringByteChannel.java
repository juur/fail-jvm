package java.nio.channels;

import java.io.IOException;
import java.nio.ByteBuffer;

public interface ScatteringByteChannel extends ReadableByteChannel {

  public long read(ByteBuffer[] dsts) throws IOException;

  public long read(ByteBuffer[] dsts, int offset, int length) throws IOException;
}
