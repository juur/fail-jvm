package java.util.jar;

import java.io.IOException;
import java.io.InputStream;

public class Manifest implements Cloneable {

  public Manifest() {}

  public Manifest(final InputStream is) throws IOException {

  }

  public Manifest(final Manifest man) {

  }

  public Attributes getMainAttributes() {
    return null;
  }
}
