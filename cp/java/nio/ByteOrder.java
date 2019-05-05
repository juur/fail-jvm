package java.nio;

public final class ByteOrder {

  public static final ByteOrder BIG_ENDIAN;
  public static final ByteOrder LITTLE_ENDIAN;

  static {
    BIG_ENDIAN = new ByteOrder("BIG_ENDIAN");
    LITTLE_ENDIAN = new ByteOrder("LITTLE_ENDIAN");
  }

  public static ByteOrder nativeOrder() {
    return LITTLE_ENDIAN;
  }

  private final String name;

  private ByteOrder(final String name) {
    this.name = name;
  }

  @Override
  public int hashCode() {
    return name.hashCode();
  }

  @Override
  public String toString() {
    return name;
  };
}
