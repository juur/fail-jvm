package java.util;

public class HashMap<K, V> extends AbstractMap<K, V> {

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

  public HashMap() {
    this(11, 0.75f);
  }

  public HashMap(final int initialCapacity) {
    this(initialCapacity, 0.75f);
  }

  public HashMap(final int initialCapacity, final float loadFactor) {
    this.buckets = new ImplEntry<?, ?>[initialCapacity];
  }

  public HashMap(final Map<? extends K, ? extends V> t) {
    this(t.size(), 0.75f);
  }

  @Override
  public Set<Entry<K, V>> entrySet() {
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
    final int             bucket = key.hashCode() % buckets.length;
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
}
