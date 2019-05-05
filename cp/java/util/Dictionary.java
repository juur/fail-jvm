package java.util;

public abstract class Dictionary<K, V> {

  public Dictionary() {}

  public abstract Enumeration<V> elements();

  public abstract V get(Object key);

  public abstract boolean isEmtpy();

  public abstract Enumeration<K> keys();

  public abstract V put(K key, V value);

  public abstract V remove(Object key);

  public abstract int size();
}
