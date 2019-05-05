package java.util;

public interface Map<K, V> {

  public static interface Entry<K, V> {

    public K getKey();

    public V getValue();

    public V setValue(V value);
  }

  public void clear();

  public boolean containsKey(Object key);

  public boolean containsValue(Object value);

  public Set<Entry<K, V>> entrySet();

  public V get(Object key);

  public boolean isEmpty();

  public Set<K> keySet();

  public V put(K key, V value);

  public void PutAll(Map<? extends K, ? extends V> m);

  public V remove(Object key);

  public int size();

  public Collection<V> values();

}
