package java.util;

import java.lang.reflect.Array;

public class Arrays {

  @SafeVarargs
  public static <T> List<T> asList(final T... a) {
    return new ArrayList<T>(a);
  }

  public static boolean[] copyOf(final boolean[] array, final int newLength) {
    if (array == null)
      throw new NullPointerException();
    if (newLength < 0)
      throw new IndexOutOfBoundsException();

    final boolean[] ret   = new boolean[newLength];
    final int       limit = array.length > newLength ? newLength : array.length;

    for (int i = 0; i < limit; i++)
      ret[i] = array[i];
    for (int i = limit; i < ret.length; i++)
      ret[i] = false;
    return ret;
  }

  public static byte[] copyOf(final byte[] array, final int newLength) {
    if (array == null)
      throw new NullPointerException();
    if (newLength < 0)
      throw new IndexOutOfBoundsException();

    final byte[] ret   = new byte[newLength];
    final int    limit = array.length > newLength ? newLength : array.length;

    for (int i = 0; i < limit; i++)
      ret[i] = array[i];
    for (int i = limit; i < ret.length; i++)
      ret[i] = 0;
    return ret;
  }

  public static char[] copyOf(final char[] array, final int newLength) {
    if (array == null)
      throw new NullPointerException();
    if (newLength < 0)
      throw new IndexOutOfBoundsException();

    final char[] ret   = new char[newLength];
    final int    limit = array.length > newLength ? newLength : array.length;

    for (int i = 0; i < limit; i++)
      ret[i] = array[i];
    for (int i = limit; i < ret.length; i++)
      ret[i] = '\u0000';
    return ret;
  }

  public static double[] copyOf(final double[] array, final int newLength) {
    if (array == null)
      throw new NullPointerException();
    if (newLength < 0)
      throw new IndexOutOfBoundsException();

    final double[] ret   = new double[newLength];
    final int      limit = array.length > newLength ? newLength : array.length;

    for (int i = 0; i < limit; i++)
      ret[i] = array[i];
    for (int i = limit; i < ret.length; i++)
      ret[i] = 0;
    return ret;
  }

  public static float[] copyOf(final float[] array, final int newLength) {
    if (array == null)
      throw new NullPointerException();
    if (newLength < 0)
      throw new IndexOutOfBoundsException();

    final float[] ret   = new float[newLength];
    final int     limit = array.length > newLength ? newLength : array.length;

    for (int i = 0; i < limit; i++)
      ret[i] = array[i];
    for (int i = limit; i < ret.length; i++)
      ret[i] = 0;
    return ret;
  }

  public static int[] copyOf(final int[] array, final int newLength) {
    if (array == null)
      throw new NullPointerException();
    if (newLength < 0)
      throw new IndexOutOfBoundsException();

    final int[] ret   = new int[newLength];
    final int   limit = array.length > newLength ? newLength : array.length;

    for (int i = 0; i < limit; i++)
      ret[i] = array[i];
    for (int i = limit; i < ret.length; i++)
      ret[i] = 0;
    return ret;
  }

  public static long[] copyOf(final long[] array, final int newLength) {
    if (array == null)
      throw new NullPointerException();
    if (newLength < 0)
      throw new IndexOutOfBoundsException();

    final long[] ret   = new long[newLength];
    final int    limit = array.length > newLength ? newLength : array.length;

    for (int i = 0; i < limit; i++)
      ret[i] = array[i];
    for (int i = limit; i < ret.length; i++)
      ret[i] = 0;
    return ret;
  }

  public static short[] copyOf(final short[] array, final int newLength) {
    if (array == null)
      throw new NullPointerException();
    if (newLength < 0)
      throw new IndexOutOfBoundsException();

    final short[] ret   = new short[newLength];
    final int     limit = array.length > newLength ? newLength : array.length;

    for (int i = 0; i < limit; i++)
      ret[i] = array[i];
    for (int i = limit; i < ret.length; i++)
      ret[i] = 0;
    return ret;
  }

  @SuppressWarnings("unchecked")
  public static <T> T[] copyOf(final T[] original, final int newLength) {
    return (T[]) copyOf(original, newLength, original.getClass());
  }

  @SuppressWarnings("unchecked")
  public static <T, U> T[] copyOf(final U[] original, final int newLength, final Class<? extends T[]> newType) {
    final T[] copy = (Object) newType == (Object) Object[].class ? (T[]) new Object[newLength]
      : (T[]) Array.newInstance(newType.getComponentType(), newLength);
    System.arraycopy(original, 0, copy, 0, Math.min(original.length, newLength));
    return copy;
  }

  public static boolean[] copyOfRange(final boolean[] original, final int from, final int to) {
    if (original == null)
      throw new NullPointerException();
    if (from < 0 || from > original.length)
      throw new ArrayIndexOutOfBoundsException();
    if (from > to)
      throw new IllegalArgumentException();

    final int       newLength = from - to;
    final boolean[] ret       = new boolean[newLength];
    final int       limit     = original.length > newLength ? newLength : original.length;

    for (int i = from; i < limit && i < to; i++)
      ret[i] = original[i];
    for (int i = limit; i < to; i++)
      ret[i] = false;

    return ret;
  }

  public static byte[] copyOfRange(final byte[] original, final int from, final int to) {
    if (original == null)
      throw new NullPointerException();
    if (from < 0 || from > original.length)
      throw new ArrayIndexOutOfBoundsException();
    if (from > to)
      throw new IllegalArgumentException();

    final int    newLength = from - to;
    final byte[] ret       = new byte[newLength];
    final int    limit     = original.length > newLength ? newLength : original.length;

    for (int i = from; i < limit && i < to; i++)
      ret[i] = original[i];
    for (int i = limit; i < to; i++)
      ret[i] = 0;

    return ret;
  }

  public static char[] copyOfRange(final char[] original, final int from, final int to) {
    if (original == null)
      throw new NullPointerException();
    if (from < 0 || from > original.length)
      throw new ArrayIndexOutOfBoundsException();
    if (from > to)
      throw new IllegalArgumentException();

    final int    newLength = from - to;
    final char[] ret       = new char[newLength];
    final int    limit     = original.length > newLength ? newLength : original.length;

    for (int i = from; i < limit && i < to; i++)
      ret[i] = original[i];
    for (int i = limit; i < to; i++)
      ret[i] = '\u0000';

    return ret;
  }

  public static double[] copyOfRange(final double[] original, final int from, final int to) {
    if (original == null)
      throw new NullPointerException();
    if (from < 0 || from > original.length)
      throw new ArrayIndexOutOfBoundsException();
    if (from > to)
      throw new IllegalArgumentException();

    final int      newLength = from - to;
    final double[] ret       = new double[newLength];
    final int      limit     = original.length > newLength ? newLength : original.length;

    for (int i = from; i < limit && i < to; i++)
      ret[i] = original[i];
    for (int i = limit; i < to; i++)
      ret[i] = 0;

    return ret;
  }

  public static float[] copyOfRange(final float[] original, final int from, final int to) {
    if (original == null)
      throw new NullPointerException();
    if (from < 0 || from > original.length)
      throw new ArrayIndexOutOfBoundsException();
    if (from > to)
      throw new IllegalArgumentException();

    final int     newLength = from - to;
    final float[] ret       = new float[newLength];
    final int     limit     = original.length > newLength ? newLength : original.length;

    for (int i = from; i < limit && i < to; i++)
      ret[i] = original[i];
    for (int i = limit; i < to; i++)
      ret[i] = 0;

    return ret;
  }

  public static int[] copyOfRange(final int[] original, final int from, final int to) {
    if (original == null)
      throw new NullPointerException();
    if (from < 0 || from > original.length)
      throw new ArrayIndexOutOfBoundsException();
    if (from > to)
      throw new IllegalArgumentException();

    final int   newLength = from - to;
    final int[] ret       = new int[newLength];
    final int   limit     = original.length > newLength ? newLength : original.length;

    for (int i = from; i < limit && i < to; i++)
      ret[i] = original[i];
    for (int i = limit; i < to; i++)
      ret[i] = 0;

    return ret;
  }

  public static long[] copyOfRange(final long[] original, final int from, final int to) {
    if (original == null)
      throw new NullPointerException();
    if (from < 0 || from > original.length)
      throw new ArrayIndexOutOfBoundsException();
    if (from > to)
      throw new IllegalArgumentException();

    final int    newLength = from - to;
    final long[] ret       = new long[newLength];
    final int    limit     = original.length > newLength ? newLength : original.length;

    for (int i = from; i < limit && i < to; i++)
      ret[i] = original[i];
    for (int i = limit; i < to; i++)
      ret[i] = 0;

    return ret;
  }

  public static short[] copyOfRange(final short[] original, final int from, final int to) {
    if (original == null)
      throw new NullPointerException();
    if (from < 0 || from > original.length)
      throw new ArrayIndexOutOfBoundsException();
    if (from > to)
      throw new IllegalArgumentException();

    final int     newLength = from - to;
    final short[] ret       = new short[newLength];
    final int     limit     = original.length > newLength ? newLength : original.length;

    for (int i = from; i < limit && i < to; i++)
      ret[i] = original[i];
    for (int i = limit; i < to; i++)
      ret[i] = 0;

    return ret;
  }

  public static boolean equals(final byte[] a, final byte[] a2) {
    if (a == null && a2 == null)
      return true;
    if (a == null || a2 == null)
      return false;
    if (a.length != a2.length)
      return false;

    for (int i = 0; i < a.length; i++)
      if (a[i] != a2[i])
        return false;

    return true;
  }

  public static boolean equals(final char[] a, final char[] a2) {
    if (a == null && a2 == null)
      return true;
    if (a == null || a2 == null)
      return false;
    if (a.length != a2.length)
      return false;

    for (int i = 0; i < a.length; i++)
      if (a[i] != a2[i])
        return false;

    return true;
  }

  public static int hashCode(final boolean[] a) {
    int rc = 31;
    for (final boolean b : a)
      rc += b ? 1 : 0;
    return rc;
  }

  public static int hashCode(final byte[] a) {
    int rc = 31;
    for (final byte b : a)
      rc += b;
    return rc;
  }

  public static int hashCode(final char[] a) {
    int rc = 31;
    for (final char b : a)
      rc += b;
    return rc;
  }

  public static int hashCode(final double[] a) {
    int rc = 31;
    for (final double b : a)
      rc += b;
    return rc;
  }

  public static int hashCode(final float[] a) {
    int rc = 31;
    for (final float b : a)
      rc += b;
    return rc;
  }

  public static int hashCode(final int[] a) {
    int rc = 31;
    for (final int b : a)
      rc += b;
    return rc;
  }

  public static int hashCode(final long[] a) {
    int rc = 31;
    for (final long b : a)
      rc += b;
    return rc;
  }

  public static int hashCode(final Object[] a) {
    if (a == null)
      return 0;

    int rc = 31;
    for (final Object b : a)
      rc += b == null ? 0 : b.hashCode();

    return rc;
  }

  public static int hashCode(final short[] a) {
    int rc = 31;
    for (final short b : a)
      rc += b;
    return rc;
  }

  public static String toString(final int a) {
    return String.valueOf(a);
  }

  private Arrays() {}
}
