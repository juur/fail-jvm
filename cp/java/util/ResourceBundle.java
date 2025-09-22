package java.util;

public abstract class ResourceBundle {

  public static class Control {

    static List<String> FORMAT_CLASS      = Arrays.asList("java.class");
    static List<String> FORMAT_DEFAULT    = Arrays.asList("java.class", "java.properties");
    static List<String> FORMAT_PROPERTIES = Arrays.asList("java.properties");

    static long TTL_DONT_CACHE            = -1L;
    static long TTL_NO_EXPIRATION_CONTROL = -2L;

    protected Control() {

    }
  }

  public static ResourceBundle getBundle(final String baseName) {
    try {
      /* TODO somehow get the caller getClassLoader()? */
      return getBundle(baseName, Locale.getDefault(), ResourceBundle.class.getClassLoader());
    } catch (final Exception e) {
      throw new MissingResourceException("getBundle", "", baseName);
    }
  }

  public static ResourceBundle getBundle(final String baseName, final Locale locale) {
    try {
      /* TODO somehow get the caller getClassLoader()? */
      return getBundle(baseName, locale, ResourceBundle.class.getClassLoader());
    } catch (final Exception e) {
      throw new MissingResourceException("getBundle", "", baseName);
    }
  }

  public static ResourceBundle getBundle(final String baseName, final Locale locale, final ClassLoader loader) {
    final ResourceBundle ret = null;

    return ret;
  }

  public static ResourceBundle getBundle(
    final String baseName,
    final Locale locale,
    final ClassLoader loader,
    final Control control
    ) {
    final ResourceBundle ret = null;

    return ret;
  }

  protected ResourceBundle parent;

  private Locale locale;

  public ResourceBundle() {
    parent = null;
  }

  public final Object getObject(final String key) {
    Object ret;

    if (key == null)
      throw new NullPointerException();

    if ((ret = handleGetObject(key)) != null)
      return ret;

    if (parent != null && (ret = parent.getObject(key)) != null)
      return ret;

    throw new MissingResourceException("getObject", getClass().getSimpleName(), key);
  }

  public final String getString(final String key) {
    return (String) getObject(key);
  }

  protected abstract Object handleGetObject(String key);

  protected void setParent(final ResourceBundle newParent) {
    parent = newParent;
  }
}
