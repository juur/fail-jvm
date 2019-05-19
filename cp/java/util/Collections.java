package java.util;

public class Collections {

  static class ImmutableCollection<T> implements Collection<T> {

    Collection<? extends T> base;

    ImmutableCollection(final Collection<? extends T> collection) {
      if (collection == null)
        throw new NullPointerException();
      base = collection;
    }

    @Override
    public boolean add(final T e) {
      throw new UnsupportedOperationException();
    }

    @Override
    public boolean addAll(final Collection<? extends T> c) {
      throw new UnsupportedOperationException();
    }

    @Override
    public void clear() {
      throw new UnsupportedOperationException();
    }

    @Override
    public boolean contains(final Object o) {
      return base.contains(o);
    }

    @Override
    public boolean containsAll(final Collection<?> c) {
      return base.containsAll(c);
    }

    @Override
    public boolean isEmtpy() {
      return base.isEmtpy();
    }

    @Override
    public Iterator<T> iterator() {
      return new Iterator<T>() {

        private final Iterator<? extends T> it = base.iterator();

        @Override
        public boolean hasNext() {
          return it.hasNext();
        }

        @Override
        public T next() {
          return it.next();
        }

        @Override
        public void remove() {
          throw new UnsupportedOperationException();
        }

      };
    }

    @Override
    public boolean remove(final Object o) {
      throw new UnsupportedOperationException();
    }

    @Override
    public boolean removeAll(final Collection<?> c) {
      throw new UnsupportedOperationException();
    }

    @Override
    public boolean retainAll(final Collection<?> c) {
      throw new UnsupportedOperationException();
    }

    @Override
    public int size() {
      return base.size();
    }

    @Override
    public Object[] toArray() {
      return base.toArray();
    }

    @Override
    public <E> E[] toArray(final E[] a) {
      return base.toArray(a);
    }

  }

  static class ImmutableList<T> extends ImmutableCollection<T> implements List<T> {

    private final List<? extends T> list;

    ImmutableList(final List<? extends T> collection) {
      super(collection);
      list = collection;
    }

    @Override
    public void add(final int index, final T element) {
      throw new UnsupportedOperationException();
    }

    @Override
    public T get(final int index) {
      return list.get(index);
    }

    @Override
    public int indexOf(final Object o) {
      return list.indexOf(o);
    }

    @Override
    public int lastIndexOf(final Object o) {
      return list.lastIndexOf(o);
    }

    @Override
    public ListIterator<T> listIterator() {
      return listIterator(0);
    }

    @Override
    public ListIterator<T> listIterator(final int index) {
      return new ListIterator<T>() {

        private final ListIterator<? extends T> it = list.listIterator(index);

        @Override
        public void add(final T e) {
          throw new UnsupportedOperationException();
        }

        @Override
        public boolean hasNext() {
          return it.hasNext();
        }

        @Override
        public boolean hasPrevious() {
          return it.hasPrevious();
        }

        @Override
        public T next() {
          return it.next();
        }

        @Override
        public int nextIndex() {
          return it.nextIndex();
        }

        @Override
        public T previous() {
          return it.previous();
        }

        @Override
        public int previousIndex() {
          return it.previousIndex();
        }

        @Override
        public void remove() {
          throw new UnsupportedOperationException();
        }

        @Override
        public void set(final T e) {
          throw new UnsupportedOperationException();
        }
      };
    }

    @Override
    public T remove(final int index) {
      throw new UnsupportedOperationException();
    }

    @Override
    public T set(final int index, final T element) {
      throw new UnsupportedOperationException();
    }

    @Override
    public List<T> subList(final int fromIndex, final int toIndex) {
      return new ImmutableList<T>(list.subList(fromIndex, toIndex));
    }

  }

  static class ImmutableMap<K, V> implements Map<K, V> {

    private final Map<? extends K, ? extends V> map;

    ImmutableMap(final Map<? extends K, ? extends V> collection) {
      map = collection;
    }

    @Override
    public void clear() {
      throw new UnsupportedOperationException();
    }

    @Override
    public boolean containsKey(final Object key) {
      return map.containsKey(key);
    }

    @Override
    public boolean containsValue(final Object value) {
      return map.containsValue(value);
    }

    @Override
    public Set<Entry<K, V>> entrySet() {
      return null;
    }

    @Override
    public V get(final Object key) {
      return map.get(key);
    }

    @Override
    public boolean isEmpty() {
      return map.isEmpty();
    }

    @Override
    public Set<K> keySet() {
      return new ImmutableSet<K>(map.keySet());
    }

    @Override
    public V put(final K key, final V value) {
      throw new UnsupportedOperationException();
    }

    @Override
    public void PutAll(final Map<? extends K, ? extends V> m) {
      throw new UnsupportedOperationException();
    }

    @Override
    public V remove(final Object key) {
      throw new UnsupportedOperationException();
    }

    @Override
    public int size() {
      return map.size();
    }

    @Override
    public Collection<V> values() {
      return new ImmutableCollection<V>(map.values());
    }

  }

  static class ImmutableSet<T> extends ImmutableCollection<T> implements Set<T> {

    ImmutableSet(final Set<? extends T> collection) {
      super(collection);
    }

  }

  public static <T> ArrayList<T> list(final Enumeration<T> e) {
    final ArrayList<T> ret = new ArrayList<T>();
    while (e.hasMoreElements())
      ret.add(e.nextElement());
    return ret;
  }

  public static <T> Collection<T> unmodifiableCollection(final Collection<? extends T> collection) {
    return new ImmutableCollection<T>(collection);
  }

  public static <T> List<T> unmodifiableList(final List<? extends T> list) {
    return new ImmutableList<T>(list);
  }

  public static <K, V> Map<K, V> unmodifiableMap(final Map<? extends K, ? extends V> map) {
    return new ImmutableMap<K, V>(map);
  }

  public static <T> Set<T> unmodifiableSet(final Set<? extends T> set) {
    return new ImmutableSet<T>(set);
  }

  private Collections() {
    throw new IllegalStateException();
  }
}
