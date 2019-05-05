package java.util;

public interface NavigableMap<K, V> extends SortedMap<K, V> {

  public Map.Entry<K, V> ceilingEntry(K key);

  public K ceilingKey(K key);

  public NavigableSet<K> descendingKeySet();

  public NavigableMap<K, V> descendingMap();

  public Map.Entry<K, V> firstEntry();

  public Map.Entry<K, V> floorEntry(K key);

  public K floorKey(K key);

  public NavigableMap<K, V> headMap(K toKey, boolean inclusive);

  public Map.Entry<K, V> higherEntry(K key);

  public K higherKey(K key);

  public Map.Entry<K, V> lastEntry();

  public Map.Entry<K, V> lowerEntry(K key);

  public K lowerKey(K key);

  public NavigableMap<K, V> navigableKeySet();

  public Map.Entry<K, V> pollFirstEntry();

  public Map.Entry<K, V> pollLastEntry();

  public NavigableMap<K, V> subMap(K fromKey, boolean inclusive);

  public NavigableMap<K, V> subMap(K fromKey, boolean fromInclusive, K toKey, boolean toInclusive);
}
