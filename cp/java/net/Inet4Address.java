package java.net;


public class Inet4Address extends InetAddress {

  private final byte[] addr;
  private final String hostAddress;
  private static final long serialVersionUID = 1L;

  Inet4Address(final byte[] newAddr) {
    addr = newAddr;
    hostAddress = (int) addr[0] + "." + (int) addr[1] + "." + (int) addr[2] + "." + (int) addr[3];
  }

  @Override
  public byte[] getAddress() {
    return addr;
  }

  @Override
  public String getHostAddress() {
    return hostAddress;
  }
}
