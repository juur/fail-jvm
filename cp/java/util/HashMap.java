package java.util;

public class HashMap<K, V> extends AbstractMap<K, V> {

  final private ImplEntry<?, ?>[] buckets;
  private int                     size;

  public HashMap() {
    this(11, 0.75f);
  }

  public HashMap(final int initialCapacity) {
    this(initialCapacity, 0.75f);
  }

  public HashMap(final int initialCapacity, final float loadFactor) {
    buckets = new ImplEntry<?, ?>[Math.max(11, Math.min(20, initialCapacity))];
  }

  public HashMap(final Map<? extends K, ? extends V> t) {
    this(t.size(), 0.75f);
    for (final K o : t.keySet())
      put(o, t.get(o));
  }

  @Override
  public void clear() {
    for (int i = 0; i < buckets.length; i++)
      buckets[i] = null;
  }

  @Override
  public boolean containsKey(final Object key) {
    return keySet().contains(key);
  }

  @Override
  public boolean containsValue(final Object value) {
    return get(value) != null;
  }

  @Override
  public Set<Map.Entry<K, V>> entrySet() {
    final HashSet<Entry<K, V>> hs    = new HashSet<Entry<K, V>>();
    final ImplEntry<?, ?>      tab[] = buckets;

    for (final ImplEntry<?, ?> b : tab) {
      @SuppressWarnings("unchecked")
      ImplEntry<K, V> head = (ImplEntry<K, V>) b;
      while (head != null) {
        hs.add(head);
        head = head.next;
      }
    }

    return hs;
  }

  @Override
  public V get(final Object key) {
    final int             bucket = Math.abs(key.hashCode()) % buckets.length;
    final ImplEntry<?, ?> tab[]  = buckets;

    @SuppressWarnings("unchecked")
    ImplEntry<K, V> head = (ImplEntry<K, V>) tab[bucket];

    if (head == null)
      return null;
    while (head != null) {
      if (head.key.equals(key))
        return head.value;
      head = head.next;
    }
    return null;
  }

  @Override
  public boolean isEmpty() {
    for (final ImplEntry<?, ?> bucket : buckets)
      if (bucket != null)
        return false;
    return true;
  }

  @Override
  public Set<K> keySet() {
    final HashSet<K>      hs    = new HashSet<K>();
    final ImplEntry<?, ?> tab[] = buckets;
    for (final ImplEntry<?, ?> b : tab) {
      @SuppressWarnings("unchecked")
      ImplEntry<K, V> head = (ImplEntry<K, V>) b;
      while (head != null) {
        hs.add(head.key);
        head = head.next;
      }
    }

    return hs;
  }

  @Override
  public V put(final K key, final V value) {
    if (key == null)
      throw new NullPointerException("key is null");

    final ImplEntry<K, V> entry = new ImplEntry<K, V>(key, value, null);

    if (buckets == null)
      throw new NullPointerException("buckets is null");

    final ImplEntry<?, ?> tab[]  = buckets;
    final int             bucket = Math.abs(key.hashCode()) % buckets.length;

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
  public V remove(final Object key) {
    final ImplEntry<?, ?> tab[]  = buckets;
    final int             bucket = Math.abs(key.hashCode()) % buckets.length;

    @SuppressWarnings("unchecked")
    ImplEntry<K, V> head = (ImplEntry<K, V>) tab[bucket];
    ImplEntry<K, V> prev = null;

    if (head == null)
      return null;

    while (head != null) {
      if (head.getKey().equals(key)) {
        if (prev == null)
          tab[bucket] = head.next;
        else
          prev.next = head.next;
        return head.getValue();
      }
      prev = head;
      head = head.next;
    }

    return null;
  }

  @Override
  public int size() {
    int ret = 0;
    for (final ImplEntry<?, ?> bucket : buckets) {
      @SuppressWarnings("unchecked")
      ImplEntry<K, V> head = (ImplEntry<K, V>) bucket;

      while (head != null) {
        ret++;
        head = head.next;
      }
    }

    return ret;
  }

  @Override
  protected Object clone() throws CloneNotSupportedException {
    return new HashMap<K, V>(this);
  }
}
