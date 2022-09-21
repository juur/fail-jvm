package java.net;

import java.io.FileDescriptor;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class SimpleSocketImpl extends SocketImpl {

  @Override
  protected void accept(final SocketImpl s) throws IOException {
    nativeAccept(s.getFileDescriptor());
  }

  @Override
  protected int available() throws IOException {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  protected void bind(final InetAddress host, final int newPort) throws IOException {

  }

  @Override
  protected void close() throws IOException {
    nativeClose(fd);
  }

  @Override
  protected void connect(final InetAddress newAddress, final int newPort) throws IOException {
    // TODO Auto-generated method stub

  }

  @Override
  protected void connect(final SocketAddress newAddress, final int timeout) throws IOException {
    // TODO Auto-generated method stub

  }

  @Override
  protected void connect(final String host, final int newPort) throws IOException {
    nativeConnect(fd, host.toCharArray(), newPort);
  }

  @Override
  protected void create(final boolean stream) throws IOException {
    nativeCreate(stream);
  }

  @Override
  protected InputStream getInputStream() throws IOException {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public Object getOption(final int optID) throws SocketException {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  protected OutputStream getOutputStream() throws IOException {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  protected void listen(final int backlog) throws IOException {
    nativeListen(fd, backlog);
  }

  private native void nativeAccept(FileDescriptor fdObj) throws IOException;

  private native void nativeClose(FileDescriptor fdObj) throws IOException;

  private native void nativeConnect(FileDescriptor fdObj, char[] host, int newPort)
    throws IOException;

  private native void nativeCreate(boolean stream) throws IOException;

  private native void nativeListen(FileDescriptor fdObj, int backlog) throws IOException;

  @Override
  protected void sendUrgentData(final int data) throws IOException {
    // TODO Auto-generated method stub

  }

  @Override
  public void setOption(final int optID, final Object value) throws SocketException {
    // TODO Auto-generated method stub

  }

}
