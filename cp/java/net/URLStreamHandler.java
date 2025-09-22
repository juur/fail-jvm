package java.net;

import java.io.IOException;

public abstract class URLStreamHandler {

  public URLStreamHandler() {}

  protected int getDefaultPort() {
    return -1;
  }

  protected abstract URLConnection openConnection(URL u) throws IOException;

  protected URLConnection openConnection(final URL u, final Proxy p) throws IOException {
    return openConnection(u);
  }

  protected void parseURL(final URL u, final String spec, final int start, final int limit) {
    final String toParse = spec.substring(start, limit);
  }

  protected void setURL(
    final URL u,
    final String protocol,
    final String host,
    final int port,
    final String authority,
    final String userInfo,
    final String path,
    final String query,
    final String ref
    ) {

  }
}
