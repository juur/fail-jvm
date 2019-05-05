package java.lang.reflect;

public final class Array {

  public static native Object newInstance(Class<?> componentType, int... dimensions) throws NegativeArraySizeException;

  public static Object newInstance(final Class<?> componentType, final int length) {
    final int[] x = { length };
    return newInstance(componentType, x);
  }
}
