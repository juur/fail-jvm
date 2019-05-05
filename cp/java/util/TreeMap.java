package java.util;

public class TreeMap<K, V> extends AbstractMap<K, V> implements NavigableMap<K, V> {

  private Comparator<? super K> comparator;
  private HashSet<Entry<K, V>>  map;       // wrong

  public TreeMap() {
    super();
    comparator = null;
    map = new HashSet<Map.Entry<K, V>>();
  }

  public TreeMap(final Comparator<? super K> comparator) {
    this();
    this.comparator = comparator;
  }

  public TreeMap(final Map<? extends K, ? extends V> m) {
    this();
  }

  public TreeMap(final SortedMap<K, ? extends V> m) {
    this();
  }

  @Override
  public Entry<K, V> ceilingEntry(final K key) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public K ceilingKey(final K key) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public Comparator<? super K> comparator() {
    return comparator;
  }

  @Override
  public NavigableSet<K> descendingKeySet() {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public NavigableMap<K, V> descendingMap() {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public Set<Entry<K, V>> entrySet() {
    return map;
  }

  @Override
  public Entry<K, V> firstEntry() {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public K firstKey() {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public Entry<K, V> floorEntry(final K key) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public K floorKey(final K key) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public SortedMap<K, V> headMap(final K toKey) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public NavigableMap<K, V> headMap(final K toKey, final boolean inclusive) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public Entry<K, V> higherEntry(final K key) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public K higherKey(final K key) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public Entry<K, V> lastEntry() {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public K lastKey() {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public Entry<K, V> lowerEntry(final K key) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public K lowerKey(final K key) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public NavigableMap<K, V> navigableKeySet() {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public Entry<K, V> pollFirstEntry() {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public Entry<K, V> pollLastEntry() {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public NavigableMap<K, V> subMap(final K fromKey, final boolean inclusive) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public NavigableMap<K, V> subMap(final K fromKey, final boolean fromInclusive, final K toKey, final boolean toInclusive) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public SortedMap<K, V> subMap(final K fromKey, final K toKey) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public SortedMap<K, V> tailMap(final K fromKey) {
    // TODO Auto-generated method stub
    return null;
  }
}
