package java.util;

import java.io.Serializable;

public class Random implements Serializable {

  private static final long serialVersionUID = 1L;
  private long              seed;
  private boolean           haveNextNextGaussian;

  private double nextNextGaussian;

  public Random() {
    this(System.currentTimeMillis());
  }

  public Random(final long nSeed) {
    setSeed(nSeed);
  }

  public boolean nextBoolean() {
    return next(1) != 0;
  }

  public void nextBytes(final byte[] bytes) {
    for (int i = 0; i < bytes.length;)
      for (int rnd = nextInt(), n = Math.min(bytes.length - i, 4); n-- > 0; rnd >>= 8)
        bytes[i++] = (byte) rnd;
  }

  public double nextDouble() {
    return (((long) next(26) << 27) + next(27)) / (double) (1L << 53);
  }

  public float nextFloat() {
    return next(24) / (float) (1 << 24);
  }

  public double nextGaussian() {
    if (haveNextNextGaussian) {
      haveNextNextGaussian = false;
      return nextNextGaussian;
    }
    double v1, v2, s;
    do {
      v1 = 2 * nextDouble() - 1;
      v2 = 2 * nextDouble() - 1;
      s = v1 * v1 + v2 * v2;
    } while (s >= 1 || s == 0);
    final double multiplier = Math.sqrt(-2 * Math.log(s) / s);
    nextNextGaussian = v2 * multiplier;
    haveNextNextGaussian = true;
    return v1 * multiplier;
  }

  public int nextInt() {
    return next(32);
  }

  public int nextInt(final int n) {
    if (n <= 0)
      throw new IllegalArgumentException();

    if ((n & -n) == n)
      return (int) (n * (long) next(31) >> 31);

    int bits, val;
    do {
      bits = next(31);
      val = bits % n;
    } while (bits - val + n - 1 < 0);
    return val;
  }

  public long nextLong() {
    return ((long) next(32) << 32) + next(32);
  }

  public void setSeed(final long newSeed) {
    seed = (newSeed ^ 0x5DEECE66DL) & (1L << 48) - 1;
    haveNextNextGaussian = false;
  }

  protected int next(final int bits) {
    seed = seed * 0x5DEECE66DL + 0xBL & (1L << 48) - 1;
    return (int) (seed >>> 48 - bits);
  }

}
