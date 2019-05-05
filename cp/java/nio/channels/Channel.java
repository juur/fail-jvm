package java.nio.channels;

import java.io.Closeable;

public interface Channel extends Closeable {

  public boolean isOpen();
}
