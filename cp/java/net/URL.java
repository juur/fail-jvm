package java.net;

import java.io.Serializable;

public final class URL implements Serializable {

  private static URLStreamHandlerFactory factory;

  static {
    factory = new fail.FailURLStreamHandlerFactory();
  }

  private String authority, protocol, host, path, query, ref;

  private int    port, defaultPort;

  public URL(final String spec) {
    this(null, spec);
  }

  public URL(final String newProtocol, final String newHost, final int newPort, final String file) throws MalformedURLException {

    if (newProtocol == null || newProtocol.length() == 0)
      throw new MalformedURLException();

    protocol = newProtocol;
    host = newHost;
    final int offsetQuery = file.indexOf('?');

    if (offsetQuery == -1) {
      path = file;
    } else {
      path = file.substring(0, offsetQuery - 1);
      query = file.substring(offsetQuery);

      final int offsetFragment = query.indexOf('#');
      if (offsetFragment != -1) {
        ref = query.substring(offsetFragment);
        query = query.substring(0, offsetFragment - 1);
      }
    }

    defaultPort = -1;
    URLStreamHandler handler = null;

    try {
      handler = factory.createURLStreamHandler(newProtocol);
      defaultPort = handler.getDefaultPort();
    } catch (final IllegalArgumentException e) {}

    if (handler == null)
      throw new MalformedURLException();

    if (newPort == -1)
      port = handler.getDefaultPort();
    else
      port = newPort;
  }

  public URL(final String protocol, final String host, final int port, final String file, final URLStreamHandler handler) throws MalformedURLException {
    /* TODO ignore or not the requirement for a SecurityManager check ? */
    this(protocol, host, port, file);
  }

  public URL(final String protocol, final String host, final String file) throws MalformedURLException {
    this(protocol, host, -1, file);
  }

  public URL(final URL context, final String spec) {

  }

  public String getAuthority() {
    return authority;
  }

  public int getDefaultPort() {
    return defaultPort;
  }

  public String getHost() {
    return host;
  }

  public String getPath() {
    return path;
  }

  public int getPort() {
    return port;
  }

  public String getProtocol() {
    return protocol;
  }

  public String getQuery() {
    return query;
  }

  public String getRef() {
    return ref;
  }
}