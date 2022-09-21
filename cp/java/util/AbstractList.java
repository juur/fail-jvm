package java.util;

public abstract class AbstractList<E> extends AbstractCollection<E> implements List<E> {

  private static class ListViewImpl<E> extends AbstractList<E> {

    private List<E> list;
    private int from;
    private int to;

    ListViewImpl(List<E> nList, int nFrom, int nTo) {
      list = nList;
      from = nFrom;
      to = nTo;
    }
    
    @Override
    public E get(int index) {
      if (index > to || index < 0)
        throw new IndexOutOfBoundsException();
      return list.get(index + from);
    }
    
    @Override
    public E set(int index, E element) {
      if (index > to || index < 0)
        throw new IndexOutOfBoundsException();
      return list.set(index + from, element);
    }

    @Override
    public int size() {
      return from - to;
    }
    
  }
  
  private static class ListIteratorImpl<E> extends IteratorImpl<E> implements ListIterator<E> {

    ListIteratorImpl(AbstractList<E> abstractList, int index) {
      super(abstractList, index);
    }

    @Override
    public void add(E e) {
      list.add(e);
    }

    @Override
    public boolean hasPrevious() {
      return (pos > 0);
    }

    @Override
    public int nextIndex() {
      return Math.min(pos, list.size());
    }

    @Override
    public E previous() {
      if (hasPrevious())
        return this.list.get(this.pos--);
      return null;
    }

    @Override
    public int previousIndex() {
      return pos - 1;
    }

    @Override
    public void set(E e) {
      list.set(pos, e);
    }
    
  }
  
  private static class IteratorImpl<E> implements Iterator<E> {

    protected final AbstractList<E> list;
    protected int                   pos;
    
    IteratorImpl(final AbstractList<E> abstractList, int index) {
      this.list = abstractList;
      this.pos = index;
    }

    @Override
    public boolean hasNext() {
      return pos + 1 < list.size();
    }

    @Override
    public E next() {
      if (hasNext())
        return this.list.get(this.pos++);
      return null;
    }

    @Override
    public void remove() {
      this.list.remove(this.pos);
    }

  }

  protected transient int modCount;

  protected AbstractList() {
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
  public boolean addAll(int index, Collection<? extends E> c) {
    Iterator<? extends E> it = c.iterator();
    if (!it.hasNext())
      return false;
    
    int safe = modCount;
    int pos = index;
    
    while (it.hasNext())
    {
      if (safe != modCount)
        throw new ConcurrentModificationException();
      add(pos++, it.next());
      safe = modCount;
    }
    return true;
  }

  @Override
  public void clear() {
    removeRange(0, size());
  }

  @Override
  public int indexOf(final Object o) {
    final ListIterator<E> li = listIterator();
    int safe = modCount;
    while (li.hasNext()) {
      if (safe != modCount)
        throw new ConcurrentModificationException();
      if (li.next().equals(o))
        return li.previousIndex();
    }
    return -1;
  }

  @Override
  public Iterator<E> iterator() {
    return new IteratorImpl<E>(this, 0);
  }

  @Override
  public int lastIndexOf(final Object o) {
    final ListIterator<E> li = listIterator(size());
    int safe = modCount;
    while (li.hasPrevious()) {
      if (safe != modCount)
        throw new ConcurrentModificationException();
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
  public ListIterator<E> listIterator(int index) {
    return new ListIteratorImpl<E>(this, index);
  }

  @Override
  public E remove(final int index) {
    throw new UnsupportedOperationException();
  }

  protected void removeRange(final int fromIndex, final int toIndex) {
    final ListIterator<E> li    = listIterator(fromIndex);
    int                   count = toIndex - fromIndex;
    int save = modCount;

    while (count-- > 0) {
      if (save != modCount)
        throw new ConcurrentModificationException();
      li.next();
      li.remove();
    }
  }

  @Override
  public boolean retainAll(final Collection<?> c) {
    boolean               mod = false;
    final ListIterator<E> li  = listIterator();
    int save = modCount;

    while (li.hasNext()) {
      if (!c.contains(li.next())) {
        if (modCount != save)
          throw new ConcurrentModificationException();
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
  public List<E> subList(final int fromIndex, final int toIndex) {
   return new ListViewImpl<E>(this, fromIndex, toIndex);
  }
}
