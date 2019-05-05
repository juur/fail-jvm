package java.lang;

public final class Void {

  public static final Class<Void> TYPE;

  static {
    TYPE = new Class<Void>();
    TYPE.isPrimitive = true;
    TYPE.canonicalName = "void";
    TYPE.simpleName = "void";
  }
}
