package java.nio.channels;

import java.io.IOException;

public interface InterruptibleChannel extends Channel {

  @Override
  void close() throws IOException;
}
