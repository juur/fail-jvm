package java.lang.reflect;

import java.lang.annotation.Annotation;

public class AccessibleObject implements AnnotatedElement {

  private boolean isAccessible;

  @Override
  public <T extends Annotation> T getAnnotation(final Class<T> annotationClass) {
    return null;
  }

  @Override
  public Annotation[] getAnnotations() {
    return new Annotation[0];
  }

  @Override
  public Annotation[] getDeclaredAnnotations() {
    return new Annotation[0];
  }

  public boolean isAccessible() {
    return isAccessible;
  }

  @Override
  public boolean isAnnotationPresent(final Class<? extends Annotation> annotationClass) {
    return false;
  }

  public void setAccessible(final boolean flag) throws SecurityException {
    isAccessible = flag;
  }
}
