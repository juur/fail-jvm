package java.net;

import java.io.FileDescriptor;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public abstract class SocketImpl implements SocketOptions {

  protected InetAddress    address;
  protected int            port;
  protected FileDescriptor fd;

  protected int localport;

  public SocketImpl() {

  }

  protected abstract void accept(SocketImpl s) throws IOException;

  protected abstract int available() throws IOException;

  protected abstract void bind(InetAddress host, int newPort) throws IOException;

  protected abstract void close() throws IOException;

  protected abstract void connect(InetAddress newAddress, int newPort) throws IOException;

  protected abstract void connect(SocketAddress newAddress, int timeout) throws IOException;

  protected abstract void connect(String host, int newPort) throws IOException;

  protected abstract void create(boolean stream) throws IOException;

  protected FileDescriptor getFileDescriptor() {
    return fd;
  }

  protected InetAddress getInetAddress() {
    return address;
  }

  protected abstract InputStream getInputStream() throws IOException;

  protected int getLocalPort() {
    return localport;
  }

  protected abstract OutputStream getOutputStream() throws IOException;

  protected int getPort() {
    return port;
  }

  protected abstract void listen(int backlog) throws IOException;

  protected abstract void sendUrgentData(int data) throws IOException;

  protected void setPerformancePreferences(
    final int connectionTime,
    final int latency,
    final int bandwidth
    ) {

  }

  protected void shutdownInput() throws IOException {

  }

  protected void shutdownOutput() throws IOException {

  }

  protected boolean supportsUrgentData() {
    return false;
  }
}
