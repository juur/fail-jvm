package java.lang;

public abstract class ClassLoader {

  final static private ClassLoader systemClassLoader;

  static {
    systemClassLoader = new spod.ClassLoader();
  }

  public static ClassLoader getSystemClassLoader() {
    return systemClassLoader;
  }

  private final ClassLoader parent;

  protected ClassLoader() {
    this(getSystemClassLoader());
  }

  protected ClassLoader(final ClassLoader newParent) {
    parent = newParent;
  }

  void clearAssertionStatus() {
    // TODO
  }

  protected native Class<?> findClass(String name) throws ClassNotFoundException;

  protected native String findLibrary(String libname);

  protected final native Class<?> findLoadedClass(String name);

  public ClassLoader getParent() {
    return parent;
  }

  protected Class<?> loadClass(final String name) throws ClassNotFoundException {
    return loadClass(name, false);
  }

  protected Class<?> loadClass(final String name, final boolean resolve) throws ClassNotFoundException {
    Class<?> ret;

    if ((ret = findLoadedClass(name)) != null)
      ;
    else if (getParent() != null && (ret = getParent().loadClass(name, resolve)) != null)
      ;
    else if ((ret = getSystemClassLoader().loadClass(name, resolve)) != null)
      ;
    else
      ret = findClass(name);

    if (ret == null)
      return null;
    if (resolve)
      resolveClass(ret);
    return ret;
  }

  protected final native void resolveClass(Class<?> c);
}
