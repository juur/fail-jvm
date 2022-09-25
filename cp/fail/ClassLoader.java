package fail;

public class ClassLoader extends java.lang.ClassLoader {

  public ClassLoader()
  {
  }

  @Override
  public String toString() {
    return "SpodClassLoader:" + hashCode();
  }

  protected Class<?> loadClass(final String name, final boolean resolve) throws ClassNotFoundException {
  Class<?> ret;

  if ((ret = findLoadedClass(name)) != null)
    ;
  else
    ret = findClass(name);

  if (ret == null)
    return null;
  if (resolve)
    resolveClass(ret);
  return ret;
}
}
