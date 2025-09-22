package fail;

import java.io.IOException;
import java.net.URL;
import java.net.URLConnection;
import java.net.URLStreamHandler;


class FailHTTPStreamHandler extends URLStreamHandler {

  private int defaultPort;

  FailHTTPStreamHandler(final String protocol) {
    switch (protocol) {
      case "http":
        defaultPort = 80;
        break;
      case "https":
        defaultPort = 443;
        break;
      default:
        throw new IllegalArgumentException();
    }
  }

  @Override
  protected int getDefaultPort() {
    return defaultPort;
  }

  @Override
  protected URLConnection openConnection(final URL u) throws IOException {
    return null;
  }

}
