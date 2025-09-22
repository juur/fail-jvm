package java.net;

public class Proxy {

  public enum Type {
    DIRECT, HTTP, SOCKS
  }

  public Proxy(final Proxy.Type type, final SocketAddress sa) {}
}
