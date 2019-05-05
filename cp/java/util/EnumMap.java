package java.util;

public class EnumMap<K extends Enum<K>, V> extends AbstractMap<K, V> {

  private Set<K> keySet;
  private ArrayList<V> list;

  public EnumMap(final Class<K> keyType) {
    super();
    keySet = new HashSet<K>(Arrays.asList(keyType.getEnumConstants()));
    list = new ArrayList<V>(keySet.size());
  }

  public EnumMap(final EnumMap<K, ? extends V> m) {
  }

  public EnumMap(final Map<K, ? extends V> m) {}

  @Override
  public Set<Entry<K, V>> entrySet() {
    return null;
  }

  @Override
  public V get(final Object key) {
    return null;
  }

}
