package java.lang;

import java.lang.annotation.Annotation;
import java.lang.reflect.AnnotatedElement;

public class Package implements AnnotatedElement {

  @Override
  public <T extends Annotation> T getAnnotation(final java.lang.Class<T> annotationClass) {
    return null;
  }

  @Override
  public Annotation[] getAnnotations() {
    return null;
  }

  @Override
  public Annotation[] getDeclaredAnnotations() {
    return null;
  }

  @Override
  public boolean isAnnotationPresent(final java.lang.Class<? extends Annotation> annotationClass) {
    return false;
  }

}
