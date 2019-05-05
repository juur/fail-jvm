package java.lang;

import java.io.FileDescriptor;
import java.io.FileOutputStream;
import java.io.PrintStream;

public final class System {

  public static PrintStream err;
  public static PrintStream in;
  public static PrintStream out;

  private static FileOutputStream fosErr;
  private static FileOutputStream fosIn;
  private static FileOutputStream fosOut;

  static {
    fosOut = new FileOutputStream(FileDescriptor.out);
    fosIn = new FileOutputStream(FileDescriptor.in);
    fosErr = new FileOutputStream(FileDescriptor.err);
    out = new PrintStream(fosOut);
    in = new PrintStream(fosIn);
    err = new PrintStream(fosErr);
  }

  public static void arraycopy(final Object src, final int srcPos, final Object dest, final int destPos, final int length) {
    if (dest == null || src == null)
      throw new NullPointerException();
    if (srcPos < 0 || destPos < 0 || length < 0)
      throw new IndexOutOfBoundsException();
    if (!src.getClass().isArray() || !dest.getClass().isArray())
      throw new ArrayStoreException();

    final Object[] srcArray  = (Object[]) src;
    final Object[] destArray = (Object[]) dest;

    if (srcPos + length > srcArray.length || destPos + length > destArray.length)
      throw new IndexOutOfBoundsException();

    if (srcArray.getClass().getComponentType().isPrimitive() && !destArray.getClass().getComponentType().isPrimitive())
      throw new ArrayStoreException();

    if (srcArray.getClass().getComponentType().isPrimitive()
      && !srcArray.getClass().getComponentType().equals(destArray.getClass().getComponentType()))
      throw new ArrayStoreException();

    for (int pos = 0; pos < length; pos++) {
      destArray[destPos + pos] = srcArray[srcPos + pos];
    }
  }

  public static native long currentTimeMillis();

  public static native void exit(int status);

  public static native void gc();
}
