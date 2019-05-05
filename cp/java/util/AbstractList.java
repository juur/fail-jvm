package java.util;

public abstract class AbstractList<E> extends AbstractCollection<E> implements List<E> {

  private static class IteratorImpl<E> implements Iterator<E> {

    private final AbstractList<E> list;
    private int                   pos;

    public IteratorImpl(final AbstractList<E> abstractList) {
      this.list = abstractList;
      this.pos = 0;
    }

    @Override
    public boolean hasNext() {
      return pos + 1 < list.size();
    }

    @Override
    public E next() {
      if (hasNext())
        return this.list.get(this.pos++);
      else
        return null;
    }

    @Override
    public void remove() {
      this.list.remove(this.pos);
    }

  }

  protected transient int modCount;

  public AbstractList() {
    modCount = 0;
  }

  @Override
  public boolean add(final E e) {
    add(size(), e);
    return true;
  }

  @Override
  public void add(final int index, final E element) {
    throw new UnsupportedOperationException();
  }

  @Override
  public void clear() {
    removeRange(0, size());
  }

  @Override
  public int indexOf(final Object o) {
    final ListIterator<E> li = listIterator();
    while (li.hasNext()) {
      if (li.next().equals(o))
        return li.previousIndex();
    }
    return -1;
  }

  @Override
  public Iterator<E> iterator() {
    return new IteratorImpl<E>(this);
  }

  @Override
  public int lastIndexOf(final Object o) {
    final ListIterator<E> li = listIterator(size());
    while (li.hasPrevious()) {
      if (li.previous().equals(o))
        return li.nextIndex();
    }
    return -1;
  }

  @Override
  public ListIterator<E> listIterator() {
    return listIterator(0);
  }

  @Override
  public E remove(final int index) {
    throw new UnsupportedOperationException();
  }

  @Override
  public boolean remove(final Object o) {
    return remove(indexOf(o)) != null;
  }

  @Override
  public boolean retainAll(final Collection<?> c) {
    boolean               mod = false;
    final ListIterator<E> li  = listIterator();

    while (li.hasNext()) {
      if (!c.contains(li.next())) {
        li.remove();
        mod = true;
      }
    }
    return mod;
  }

  @Override
  public E set(final int index, final E element) {
    throw new UnsupportedOperationException();
  }

  @Override
  public Object[] toArray() {
    return null;
  }

  @Override
  public <T> T[] toArray(final T[] a) {
    return null;
  }

  protected void removeRange(final int fromIndex, final int toIndex) {
    final ListIterator<E> li    = listIterator(fromIndex);
    int                   count = toIndex - fromIndex;

    while (count-- > 0) {
      li.next();
      li.remove();
    }
  }

  boolean addAll(final int index, final Collection<? extends E> c) {
    boolean added = false;

    for (final E o : c)
      if (add(o))
        added = true;

    return added;
  }

}
