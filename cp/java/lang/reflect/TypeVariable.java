package java.lang.reflect;

public interface TypeVariable<D extends GenericDeclaration> {

  Type[] getBounds();

  D getGenericDeclaration();

  String getName();
}
