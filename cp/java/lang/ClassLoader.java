package java.lang;

import java.net.URL;

public abstract class ClassLoader {

  final static private ClassLoader systemClassLoader;

  static {
    systemClassLoader = new fail.ClassLoader();
  }

  public static ClassLoader getSystemClassLoader() {
    return systemClassLoader;
  }

  public static URL getSystemResource(final String name) {
    return getSystemClassLoader().getResource(name);
  }

  private final ClassLoader parent;

  protected ClassLoader() {
    this(getSystemClassLoader());
  }

  protected ClassLoader(final ClassLoader newParent) {
    parent = newParent;
  }

  public ClassLoader getParent() {
    return parent;
  }

  public URL getResource(final String name) {
    URL ret;

    if (getParent() != null) {
      if ((ret = getParent().getResource(name)) != null)
        return ret;
    } else if (this != getSystemClassLoader()) {
      if ((ret = getSystemClassLoader().getResource(name)) != null)
        return ret;
    }
    return findResource(name);
  }

  protected native Class<?> findClass(String name) throws ClassNotFoundException;

  protected native String findLibrary(String libname);

  protected final native Class<?> findLoadedClass(String name);

  protected URL findResource(final String name) {
    return null;
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

  void clearAssertionStatus() {
    // TODO
  }
}
