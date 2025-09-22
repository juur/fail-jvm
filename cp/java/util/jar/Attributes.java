package java.util.jar;

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

public class Attributes implements Map<Object, Object>, Cloneable {

  public static class Name {

    public static final Attributes.Name CLASS_PATH   = new Name("Class-Path");
    public static final Attributes.Name CONTENT_TYPE = new Name("Content-Type");

    private final String name;

    public Name(final String string) {
      if (string.length() > 70)
        throw new IllegalArgumentException();

      if (!string.matches("^[0-9a-zA-Z_-]"))
        throw new IllegalArgumentException();

      name = string;
    }

    @Override
    public boolean equals(final Object obj) {
      if (!(obj instanceof Name))
        return false;
      return toString().equalsIgnoreCase(obj.toString());
    }

    @Override
    public int hashCode() {
      return name.hashCode();
    }

    @Override
    public String toString() {
      return name;
    }
  }

  protected Map<Object, Object> map;

  public Attributes() {
    map = new HashMap<>();
  }

  public Attributes(final Attributes attr) {
    map = new HashMap<>(attr);
  }

  public Attributes(final int size) {
    map = new HashMap<>(size);
  }

  @Override
  public void clear() {
    map.clear();
  }

  @Override
  public Object clone() {
    return new Attributes(this);
  }

  @Override
  public boolean containsKey(final Object key) {
    return map.containsKey(key);
  }

  @Override
  public boolean containsValue(final Object value) {
    return map.containsValue(value);
  }

  @Override
  public Set<Entry<Object, Object>> entrySet() {
    return map.entrySet();
  }

  @Override
  public boolean equals(final Object obj) {
    if (!(obj instanceof Attributes))
      return false;
    return map.equals(((Attributes) obj).map);
  }

  @Override
  public Object get(final Object key) {
    return map.get(key);
  }

  @Override
  public int hashCode() {
    return map.hashCode();
  }

  @Override
  public boolean isEmpty() {
    return map.isEmpty();
  }

  @Override
  public Set<Object> keySet() {
    return map.keySet();
  }

  @Override
  public Object put(final Object key, final Object value) {
    return map.put(key, value);
  }

  @Override
  public void PutAll(final Map<? extends Object, ? extends Object> m) {
    map.PutAll(m);
  }

  @Override
  public Object remove(final Object key) {
    return map.remove(key);
  }

  @Override
  public int size() {
    return map.size();
  }

  @Override
  public Collection<Object> values() {
    return map.values();
  }

}
