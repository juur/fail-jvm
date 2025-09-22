package java.net;

import java.io.IOException;

public class Socket {

  private final InetAddress remote     = null;
  int                       remotePort = 0;
  private final InetAddress local      = null;
  private int               localPort;

  public Socket() {

  }

  public Socket(final InetAddress address, final int port) throws IOException {

  }

  public Socket(final InetAddress address, final int port, final InetAddress localAddr, final int localPort) {
    this(new InetSocketAddress(address, port), new InetSocketAddress(localAddr, localPort));
  }

  public Socket(final InetSocketAddress inetSocketAddress, final InetSocketAddress inetSocketAddress2) {
    // TODO Auto-generated constructor stub
  }

  public Socket(final String host, final int port, final InetAddress localAddr, final int localPort)
    throws IOException {
    this(InetAddress.getByName(host), port, localAddr, localPort);
  }

  protected Socket(final SocketImpl impl) throws SocketException {

  }

  public void bind(final SocketAddress bindpoint) throws IOException {

  }

  public void connect(final SocketAddress endpoint) throws IOException {

  }

  public InetAddress getInetAddress() {
    return remote;
  }

  public InetAddress getLocalAddress() {
    return local;
  }

  public int getLocalPort() {
    return localPort;
  }

  public int getPort() {
    return remotePort;
  }
}
