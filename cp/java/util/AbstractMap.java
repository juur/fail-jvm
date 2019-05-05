package java.util;

public abstract class AbstractMap<K, V> implements Map<K, V> {

  public static class SimpleEntry<K, V> implements Map.Entry<K, V> {

    private final K key;
    private V       value;

    public SimpleEntry(final K key, final V value) {
      this.key = key;
      this.value = value;
    }

    public SimpleEntry(final Map.Entry<? extends K, ? extends V> entry) {
      this.key = entry.getKey();
      this.value = entry.getValue();
    }

    @Override
    public K getKey() {
      return key;
    }

    @Override
    public V getValue() {
      return value;
    }

    @Override
    public V setValue(final V value) {
      this.value = value;
      return value;
    }

  }

  public static class SimpleImmutableEntry<K, V> implements Map.Entry<K, V> {

    private final K key;
    private final V value;

    public SimpleImmutableEntry(final K key, final V value) {
      this.key = key;
      this.value = value;
    }

    public SimpleImmutableEntry(final Map.Entry<? extends K, ? extends V> entry) {
      this.key = entry.getKey();
      this.value = entry.getValue();
    }

    @Override
    public K getKey() {
      return key;
    }

    @Override
    public V getValue() {
      return value;
    }

    @Override
    public V setValue(final V value) {
      throw new UnsupportedOperationException();
    }

  }

  protected AbstractMap() {
    super();
  }

  @Override
  public void clear() {
    entrySet().clear();
  }

  @Override
  public boolean containsKey(final Object key) {
    for (final Entry<K, V> v : entrySet())
      if (key == null ? v.getKey() == null : key.equals(v.getKey()))
        return true;
    return false;
  }

  @Override
  public boolean containsValue(final Object value) {
    for (final Entry<K, V> v : entrySet())
      if (value == null ? v.getValue() == null : value.equals(v.getValue()))
        return true;
    return false;
  }

  @Override
  public V get(final Object key) {
    for (final Entry<K, V> v : entrySet())
      if (key == null ? v.getKey() == null : key.equals(v.getKey()))
        return v.getValue();
    return null;
  }

  @Override
  public int hashCode() {
    int hc = 0;
    for (final Entry<K, V> ent : entrySet())
      hc += ent.hashCode();
    return hc;
  }

  @Override
  public boolean isEmpty() {
    return size() == 0;
  }

  @Override
  public Set<K> keySet() {
    throw new UnsupportedOperationException("TODO");
  }

  @Override
  public V put(final K key, final V value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public void PutAll(final Map<? extends K, ? extends V> m) {

    for (final Entry<? extends K, ? extends V> ent : m.entrySet())
      put(ent.getKey(), ent.getValue());

  }

  @Override
  public V remove(final Object key) {
    final Iterator<Entry<K, V>> it = entrySet().iterator();

    while (it.hasNext()) {
      final Entry<K, V> ent = it.next();
      if (key == null ? ent.getKey() == null : key.equals(ent.getKey())) {
        it.remove();
        return ent.getValue();
      }
    }

    return null;
  }

  @Override
  public int size() {
    return entrySet().size();
  }

  @Override
  public Collection<V> values() {
    throw new UnsupportedOperationException("TODO");
  }
}
