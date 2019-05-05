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
  public boolean removeAll(final Collection<?> c) {
    boolean rem = false;
    for (final Object o : c)
      if (remove(o))
        rem = true;
    return rem;
  }
}
