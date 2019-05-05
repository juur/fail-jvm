package java.lang;

import java.lang.annotation.Annotation;
import java.lang.reflect.GenericDeclaration;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Type;
import java.lang.reflect.TypeVariable;
import java.util.Map;

public final class Class<T> implements GenericDeclaration, Type, AnnotatedElement {

  private final Annotation[] annotations;
  private final Class<?>     componentType;
  private final Annotation[] declaredAnnotations;
  private final int          modifiers;
  private Class<? super T>   superClass;
  boolean                    isArray, isPrimitive, isInterface;
  String                     simpleName, canonicalName, name;
  private final ClassLoader  classLoader;
  private T[]                enumConstants;

  private Map<String, Method> methodMap;

  Class() {
    isArray = false;
    isPrimitive = false;
    annotations = null;
    declaredAnnotations = null;
    componentType = null;
    isInterface = false;
    modifiers = 0;
    classLoader = ClassLoader.getSystemClassLoader();
  }

  @Override
  public boolean equals(final Object obj) {
    if (!(obj instanceof Class))
      return false;
    if (obj.hashCode() == hashCode())
      return true;
    return false;
  }

  @SuppressWarnings("unchecked")
  @Override
  public <TT extends Annotation> TT getAnnotation(final Class<TT> annotationClass) {
    if (annotations == null)
      return null;
    for (final Annotation a : annotations)
      if (a.getClass().isInstance(annotationClass))
        return (TT) a;
    return null;
  }

  @Override
  public Annotation[] getAnnotations() {
    return annotations == null ? new Annotation[0] : annotations;
  }

  public String getCanonicalname() {
    return canonicalName;
  }

  public ClassLoader getClassLoader() {
    return classLoader;
  }

  public Class<?> getComponentType() {
    return componentType;
  }

  @Override
  public Annotation[] getDeclaredAnnotations() {
    return declaredAnnotations == null ? new Annotation[0] : declaredAnnotations;
  }

  @SuppressWarnings("unchecked")
  public T[] getEnumConstants() {
    if (!isEnum())
      return null;

    if (enumConstants == null) {
      final Method values = getMethod("values");
      try {
        enumConstants = (T[]) values.invoke(null);
        return enumConstants;
      } catch (final IllegalArgumentException e) {
        return null;
      } catch (final IllegalAccessException e) {
        return null;
      } catch (final InvocationTargetException e) {
        return null;
      }
    } else
      return enumConstants;
  }

  public Method getMethod(final String string) {
    return methodMap.get(string);
  }

  public int getModifiers() {
    return modifiers;
  }

  public String getName() {
    return name;
  }

  public Package getPackager() {
    return null;
  }

  public Object[] getSigners() {
    return null;
  }

  public String getSimpleName() {
    return simpleName;
  }

  public Class<? super T> getSuperClass() {
    return this.superClass;
  }

  @Override
  public TypeVariable<?>[] getTypeParameters() {
    return null;
  }

  @Override
  public boolean isAnnotationPresent(final Class<? extends Annotation> annotationClass) {
    return getAnnotation(annotationClass) != null;
  }

  public boolean isArray() {
    return isArray;
  }

  public boolean isAssignableFrom(final Class<?> cls) {
    if (cls.equals(this))
      return true;
    if (getSuperClass() != null)
      return getSuperClass().isAssignableFrom(cls);
    return false;
  }

  public boolean isEnum() {
    return Enum.class.isAssignableFrom(this);
  }

  public boolean isInstance(final Class<?> other) {
    return equals(other.getClass());
  }

  public boolean isInterface() {
    return isInterface;
  }

  public boolean isPrimitive() {
    return isPrimitive;
  }

  @Override
  public String toString() {
    return (isPrimitive() ? "" : isInterface() ? "interface " : "class ") + getName();
  }

}
