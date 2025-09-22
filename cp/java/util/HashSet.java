package java.util;

public class HashSet<E> extends AbstractSet<E> {

  private final HashMap<E, E> hash;

  public HashSet() {
    hash = new HashMap<E, E>();
  }

  public HashSet(final Collection<? extends E> c) {
    hash = new HashMap<E, E>(c.size());
    final Iterator<? extends E> it = c.iterator();
    while (it.hasNext()) {
      final E val = it.next();
      hash.put(val, val);
    }
  }

  public HashSet(final int initial_size) {
    hash = new HashMap<E, E>(initial_size);
  }

  @Override
  public boolean add(final E e) {
    hash.put(e, e);
    return true;
  }

  @Override
  public boolean isEmpty() {
    return hash.isEmpty();
  }

  @Override
  public Iterator<E> iterator() {
    return new Iterator<E>() {

      @Override
      public boolean hasNext() {
        // TODO Auto-generated method stub
        return false;
      }

      @Override
      public E next() {
        // TODO Auto-generated method stub
        return null;
      }

      @Override
      public void remove() {
        // TODO Auto-generated method stub

      }
    };
  }

  @Override
  public boolean remove(final Object o) {
    return hash.remove(o) != null;
  }

  @Override
  public boolean retainAll(final Collection<?> c) {
    boolean changed = false;
    for (final Object o : hash.keySet())
      if (!c.contains(o))
        if (hash.remove(o) != null)
          changed = true;
    return changed;
  }

  @Override
  public int size() {
    return hash.size();
  }

  @Override
  public Object[] toArray() {
    return hash.keySet().toArray();
  }

  @Override
  public <T> T[] toArray(final T[] a) {
    // TODO Auto-generated method stub
    return null;
  }

}
