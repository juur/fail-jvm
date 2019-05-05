package java.lang.reflect;

public class Method extends AccessibleObject implements GenericDeclaration, Member {

  private final Class<?> declaringClass;
  private final int      modifiers;
  private final String   name;

  Method(final Class<?> declaringClass, final int modifiers, final String name) {
    this.declaringClass = declaringClass;
    this.modifiers = modifiers;
    this.name = name;
  }

  @Override
  public Class<?> getDeclaringClass() {
    return declaringClass;
  }

  @Override
  public int getModifiers() {
    return modifiers;
  }

  @Override
  public String getName() {
    return name;
  }

  @Override
  public TypeVariable<?>[] getTypeParameters() {
    // TODO Auto-generated method stub
    return null;
  }

  public native Object invoke(final Object object, final Object... args)
    throws IllegalAccessException, IllegalArgumentException, InvocationTargetException;

  @Override
  public boolean isSynthetic() {
    return false;
  }

}
