package java.nio.channels;

import java.io.IOException;

public interface InterruptibleChannel extends Channel {

  @Override
  public void close() throws IOException;
}
