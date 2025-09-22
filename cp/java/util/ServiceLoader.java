package java.util;

public final class ServiceLoader<S> implements Iterable<S> {

  private final class ServiceLoaderIterator<T> implements Iterator<S> {

    private final Iterator<S> iterator;

    private ServiceLoaderIterator(final Iterator<S> iterator) {
      this.iterator = iterator;
    }

    @Override
    public boolean hasNext() {
      return iterator.hasNext();
    }

    @Override
    public S next() {
      return iterator.next();
    }

    @Override
    public void remove() {
      throw new UnsupportedOperationException();
    }

  }

  public static <S> ServiceLoader<S> load(final Class<S> service) {
    return load(service, Thread.currentThread().getContextClassLoader());
  }

  public static <S> ServiceLoader<S> load(final Class<S> service, final ClassLoader loader) {
    final ServiceLoader<S> ret = new ServiceLoader<S>(service);
    return ret;
  }

  public static <S> ServiceLoader<S> loadInstalled(final Class<S> service) {
    /* TODO discover ClassLoader ? */
    return load(service);
  }

  private final Set<S> providerCache;
  private final Class<S> serviceClass;

  private ServiceLoader(final Class<S> service)
  {
    serviceClass = service;
    providerCache = new HashSet<S>();
  }

  @Override
  public Iterator<S> iterator() {
    return new ServiceLoaderIterator<S>(providerCache.iterator());
  }

  public void reload() {
    providerCache.clear();
  }

}
