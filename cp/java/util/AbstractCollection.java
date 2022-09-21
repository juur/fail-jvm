package java.util;

public abstract class AbstractCollection<E> implements Collection<E> {

  protected AbstractCollection() {
    super();
  }

  @Override
  public boolean add(final E e) {
    throw new UnsupportedOperationException();
  }

  @Override
  public boolean addAll(final Collection<? extends E> c) {
    boolean add = false;
    for (final E o : c)
      if (this.add(o))
        add = true;
    return add;
  }

  @Override
  public void clear() {
    final Iterator<E> i = iterator();
    while (i.hasNext()) {
      i.next();
      i.remove();
    }
  }

  @Override
  public boolean contains(final Object o) {
    for (final Object obj : this) {
      if (obj.equals(o))
        return true;
    }

    return false;
  }

  @Override
  public boolean containsAll(final Collection<?> c) {
    for (final Object obj : c)
      if (!contains(obj))
        return false;
    return true;
  }

  @Override
  public boolean isEmtpy() {
    return size() == 0;
  }

  @Override
  public abstract Iterator<E> iterator();

  @Override
  public boolean remove(final Object o) {
    final Iterator<E> it = iterator();
    while (it.hasNext())
    {
      final Object tmp = it.next();
      if(tmp.equals(o)) {
        it.remove();
        return true;
      }
    }
    return false;
  }

  @Override
  public boolean removeAll(final Collection<?> c) {
    boolean rem = false;
    for (final Object o : c)
      if (remove(o))
        rem = true;
    return rem;
  }

  @Override
  public abstract int size();

  @Override
  public Object[] toArray() {
    return toArray(new Object[size()]);
  }

  @SuppressWarnings("unchecked")
  @Override
  public <T> T[] toArray(final T[] a) {
    if (a.length >= size()) {
      final Iterator<E> it = iterator();
      int               i  = 0;

      while (it.hasNext()) {
        final Object tmp = it.next();
        try {
          a[i++] = (T) tmp;
        } catch (final ClassCastException e) {
          throw new ArrayStoreException();
        }
      }

      return a;
    }

    return null;
  }

}
