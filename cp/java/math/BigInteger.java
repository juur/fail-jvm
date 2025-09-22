package java.math;

import java.nio.ByteBuffer;

public class BigInteger extends Number implements Comparable<BigInteger> {

  public final static BigInteger ONE  = BigInteger.valueOf(1);
  public final static BigInteger TEN  = BigInteger.valueOf(10);
  public final static BigInteger ZERO = BigInteger.valueOf(0);

  public static BigInteger valueOf(final long l) {
    final ByteBuffer bb = ByteBuffer.allocateDirect(8);
    bb.putLong(l);
    return new BigInteger(bb.array());
  }

  private final byte[] val;

  public BigInteger(final byte[] value) {
    val = value;
  }

  public BigInteger abs() {
    final BigInteger ret = new BigInteger(val);
    ret.val[0] &= ~0xf0;
    return ret;
  }

  @Override
  public int compareTo(final BigInteger o) {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public double doubleValue() {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public float floatValue() {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public int intValue() {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public long longValue() {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public String toString() {
    final StringBuffer sb = new StringBuffer(val.length * 4);
    sb.append("0x");
    for (final byte b : val)
      sb.append(Number.itoa(b, 16));
    return sb.toString();

  }

}
