package java.lang.reflect;

public class Field extends AccessibleObject implements Member {

  private final boolean  synthetic;
  private final String   name;
  private final int      modifiers;
  private final Class<?> declaringClass;
  private boolean        enumConstant;
  private Class<?>       type;

  public Field(final boolean nSynthetic, final String nName, final int nModifiers, final Class<?> nDeclaringClass) {
    super();
    synthetic = nSynthetic;
    name = nName;
    modifiers = nModifiers;
    declaringClass = nDeclaringClass;
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

  public Class<?> getType() {
    return type;
  }

  public boolean isEnumConstant() {
    return enumConstant;
  }

  @Override
  public boolean isSynthetic() {
    return synthetic;
  }

}
