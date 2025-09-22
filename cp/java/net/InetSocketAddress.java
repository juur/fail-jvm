package java.net;

public class InetSocketAddress extends SocketAddress {

  private final int         port;
  private final InetAddress addr;

  public InetSocketAddress(final InetAddress addr, final int port) {
    this.addr = addr;
    this.port = port;
  }
}
