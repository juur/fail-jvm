package java.net;

import java.io.IOException;
import java.io.Serializable;
import java.nio.ByteOrder;

public class InetAddress implements Serializable {

  private static final long serialVersionUID = 1L;
  
  public static InetAddress[] getAllByName(final String host)
    throws UnknownHostException
  {
    return null;
  }

  public static InetAddress getByAddress(final byte[] addr)
    throws UnknownHostException
  {
    return null;
  }

  public static InetAddress getByAddress(final String host,
    final byte[] addr)
      throws UnknownHostException {
    return null;
  }

  public static InetAddress getByName(final String host)
    throws UnknownHostException {
    return null;
  }

  public static InetAddress getLocalHost()
    throws UnknownHostException
  {
    return null;
  }

  private static int htonl(final int value) {
    if (ByteOrder.nativeOrder().equals(ByteOrder.BIG_ENDIAN)) {
      return value;
    }
    return Integer.reverseBytes(value);
  }

  public byte[] getAddress() {
    return new byte[0];
  }

  public String getCanonicalHostName() {
    return "";
  }

  public String getHostAddress() {
    return "";
  }

  public String getHostName() {
    return new String(nativeGetHostName());
  }

  public boolean isAnyLocalAddress() {
    return false;
  }

  public boolean isLinkLocalAddress() {
    return false;
  }

  public boolean isLoopbackAddress() {
    return false;
  }

  public boolean isMCGlobal() {
    return false;
  }

  public boolean isMCLinkLocal() {
    return false;
  }

  public boolean isMCNodeLocal() {
    return false;
  }

  public boolean isMCOrgLocal() {
    return false;
  }

  public boolean isMCSiteLocal() {
    return false;
  }

  public boolean isMulticastAddress() {
    return false;
  }

  public boolean isReachable(final int timeout) throws IOException {
    return true;
  }

  public boolean isReachable(final NetworkInterface netif, final int ttl, final int timeout)
    throws IOException {
    return true;
  }

  public boolean isSiteLocalAddress() {
    return false;
  }

  private native byte[] nativeGetHostName();
}
