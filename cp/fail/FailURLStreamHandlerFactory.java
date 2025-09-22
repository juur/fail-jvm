package fail;

import java.net.URLStreamHandler;
import java.net.URLStreamHandlerFactory;


public class FailURLStreamHandlerFactory implements URLStreamHandlerFactory {

  @Override
  public URLStreamHandler createURLStreamHandler(final String protocol) {
    switch (protocol) {
      case "http":
      case "https":
        return new FailHTTPStreamHandler(protocol);

      default:
        return null;
    }

  }

}
