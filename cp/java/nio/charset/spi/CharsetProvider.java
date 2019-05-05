package java.nio.charset.spi;

import java.nio.charset.Charset;
import java.util.Iterator;

public abstract class CharsetProvider {

  protected CharsetProvider() {}

  public abstract Charset charsetForName(String charsetName);

  public abstract Iterator<Charset> charsets();
}
