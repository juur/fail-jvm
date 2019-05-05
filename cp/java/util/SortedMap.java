package java.util;

public interface SortedMap<K, V> extends Map<K, V> {

  public Comparator<? super K> comparator();

  public K firstKey();

  public SortedMap<K, V> headMap(K toKey);

  public K lastKey();

  public SortedMap<K, V> subMap(K fromKey, K toKey);

  public SortedMap<K, V> tailMap(K fromKey);

  @Override
  public Collection<V> values();
}
