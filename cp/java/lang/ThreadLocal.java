package java.lang;

import java.util.HashMap;
import java.util.Map;

public class ThreadLocal<T> {

  private final Map<Long, T> map = new HashMap<Long, T>();

  public ThreadLocal() {}

  public T get() {
    synchronized (map) {
      return map.get(Thread.currentThread().getId());
    }
  }

  protected T initialValue() {
    return null;
  }

  void remove() {
    synchronized (map) {
      map.remove(Thread.currentThread().getId());
    }
  }

  void set(final T value) {
    synchronized (map) {
      map.put(Thread.currentThread().getId(), value);
    }
  }
}
