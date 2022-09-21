package java.lang;

public abstract class Enum<E extends Enum<E>> implements Comparable<E> {

  public static <T extends Enum<T>> T valueOf(final Class<T> enumType, final String name) {
    if (name == null)
      throw new NullPointerException("name null");
    throw new IllegalArgumentException("no constant");
  }

  private final String name;

  private final int ordinal;

  protected Enum(final String newName, final int newOrdinal) {
    name = newName;
    ordinal = newOrdinal;
  }

  @Override
  protected final Object clone() throws CloneNotSupportedException {
    throw new CloneNotSupportedException();
  }

  @Override
  public final int compareTo(final E o) {
    return ordinal() - o.ordinal();
  }

  @Override
  protected final void finalize() {}

  public final Class<E> getDeclaringClass() {
    return null;
  }

  public final String name() {
    return name;
  }

  public final int ordinal() {
    return ordinal;
  }

  @Override
  public String toString() {
    return name;
  }

}
