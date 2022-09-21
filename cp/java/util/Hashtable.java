package java.util;

public class Hashtable<K, V> extends Dictionary<K, V> implements Map<K, V> {

  static class ImplEntry<KK, VV> implements Map.Entry<KK, VV> {

    private final KK          key;
    private ImplEntry<KK, VV> next;

    private VV value;

    public ImplEntry(final KK newKey, final VV newValue, final ImplEntry<KK, VV> newNext) {
      key = newKey;
      value = newValue;
      next = newNext;
    }

    @Override
    public KK getKey() {
      return key;
    }

    @Override
    public VV getValue() {
      return value;
    }

    @Override
    public VV setValue(final VV newValue) {
      value = newValue;
      return value;
    }
  }

  final private ImplEntry<?, ?>[] buckets;
  private int                     size;

  public Hashtable() {
    this(11, 0.75f);
  }

  public Hashtable(final int initialCapacity) {
    this(initialCapacity, 0.75f);
  }

  public Hashtable(final int initialCapacity, final float loadFactor) {
    this.buckets = new ImplEntry<?, ?>[initialCapacity];
  }

  public Hashtable(final Map<? extends K, ? extends V> t) {
    this(t.size(), 0.75f);
  }

  @Override
  public void clear() {
    // TODO Auto-generated method stub

  }

  @Override
  public boolean containsKey(final Object key) {
    // TODO Auto-generated method stub
    return false;
  }

  @Override
  public boolean containsValue(final Object value) {
    // TODO Auto-generated method stub
    return false;
  }

  @Override
  public Enumeration<V> elements() {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public Set<Map.Entry<K, V>> entrySet() {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public V get(final Object key) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public boolean isEmpty() {
    // TODO Auto-generated method stub
    return false;
  }

  @Override
  public boolean isEmtpy() {
    // TODO Auto-generated method stub
    return false;
  }

  @Override
  public Enumeration<K> keys() {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public Set<K> keySet() {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public V put(final K key, final V value) {
    final ImplEntry<K, V> entry  = new ImplEntry<K, V>(key, value, null);
    final int             bucket = key.hashCode() % buckets.length;
    final ImplEntry<?, ?> tab[]  = buckets;

    @SuppressWarnings("unchecked")
    ImplEntry<K, V> head = (ImplEntry<K, V>) tab[bucket];

    if (head == null) {
      tab[bucket] = entry;
      size++;
    } else {
      while (head.next != null) {
        if (head.key.equals(key)) {
          head.value = value;
          return value;
        }
        head = head.next;
      }

      if (head.key.equals(key)) {
        head.value = value;
      } else {
        head.next = entry;
        size++;
      }
    }
    return value;
  }

  @Override
  public void PutAll(final Map<? extends K, ? extends V> m) {
    for (final K k : m.keySet())
      put(k, m.get(k));
  }

  @Override
  public V remove(final Object key) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public int size() {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public Collection<V> values() {
    // TODO Auto-generated method stub
    return null;
  }

}
