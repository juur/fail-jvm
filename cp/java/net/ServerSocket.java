package java.net;

import java.io.IOException;
import java.nio.channels.ServerSocketChannel;

public class ServerSocket {

  private static SocketImplFactory fac;

  static {
    // TODO set fac
  }

  public static void setSocketFactory(final SocketImplFactory newFac) throws IOException {
    fac = newFac;
  }

  public ServerSocket() throws IOException {

  }

  public ServerSocket(final int port) throws IOException {
    this(port, 50);
  }

  public ServerSocket(final int port, final int backlog) throws IOException {
    this(port, backlog, null);
  }

  public ServerSocket(final int port, final int backlog, final InetAddress bindAddr)
    throws IOException {
    throw new IOException();
  }

  public Socket accept() throws IOException {
    throw new IOException();
  }

  public void bind(final SocketAddress endpoint) throws IOException {
    throw new IOException();
  }

  public void bind(final SocketAddress endpoint, final int backlog) {

  }

  public void close() throws IOException {
    throw new IOException();
  }

  public ServerSocketChannel getChannel() {
    return null;
  }

  public InetAddress getInetAddress() {
    return null;
  }

  public int getLocalPort() {
    return -1;
  }

  public SocketAddress getLocalSocketAddress() {
    return null;
  }

  public boolean getReuseAddress() throws SocketException {
    return false;
  }

  public int getSoTimeout() throws IOException {
    return 0;
  }

  protected final void implAccept(final Socket s) throws IOException {

  }

  public boolean isBound() {
    return false;
  }

  public boolean isClosed() {
    return false;
  }

  public void setReuseAddress(final boolean on) throws SocketException {
    throw new SocketException();
  }

  public void setSoTimeout(final int timeout) throws SocketException {
    throw new SocketException();
  }
}
