package java.util;

import java.io.Serializable;

public class ArrayList<E> extends AbstractList<E> implements RandomAccess, Cloneable, Serializable {

  private static final long serialVersionUID = 1L;

  private static class ListIteratorImpl<E> implements ListIterator<E> {

    private final ArrayList<E> list;
    private int                ptr;

    private ListIteratorImpl(final ArrayList<E> arrayList, final int index) {
      this.list = arrayList;
      this.ptr = index;
    }

    @Override
    public void add(final E e) {
      list.add(e);
    }

    @Override
    public boolean hasNext() {
      return ptr < list.size();
    }

    @Override
    public boolean hasPrevious() {
      return ptr > 0;
    }

    @Override
    public E next() {
      if (ptr < list.size())
        return list.get(ptr++);
      throw new NoSuchElementException();
    }

    @Override
    public int nextIndex() {
      return Math.min(ptr + 1, list.size());
    }

    @Override
    public E previous() {
      if (ptr > 0)
        return list.get(--ptr);
      throw new NoSuchElementException();
    }

    @Override
    public int previousIndex() {
      return Math.max(0, ptr - 1);
    }

    @Override
    public void remove() {
      list.remove(ptr);
    }

    @Override
    public void set(final E e) {
      list.set(ptr, e);
    }

  }

  private Object[] array;
  private int      last;

  public ArrayList() {
    this(10);
  }

  public ArrayList(final int i) {
    this.array = new Object[i];
    last = 0;
  }

  public ArrayList(final Object[] newArray) {
    this.array = newArray;
    this.last = newArray.length - 1;
  }

  @Override
  public boolean add(final E e) {
    if (last >= array.length)
      array = Arrays.copyOf(array, array.length * 2);
    array[last++] = e;
    modCount++;
    return true;
  }

  @SuppressWarnings("unchecked")
  @Override
  public E get(final int index) {
    if (index < 0 || index >= size())
      throw new IndexOutOfBoundsException();
    return (E) array[index];
  }

  @Override
  public ListIterator<E> listIterator(final int index) {
    return new ListIteratorImpl<E>(this, index);
  }

  @Override
  public E set(final int index, final E element) {
    if (index < 0 || index >= size())
      throw new IndexOutOfBoundsException();
    array[index] = element;
    modCount++;
    return element;
  }

  @Override
  public int size() {
    return last + 1;
  }

  @Override
  public boolean remove(Object o) {
    int safe = modCount;
    int idx  = indexOf(o);
    if (idx == -1)
      return false;
    
    if (safe != modCount)
      throw new ConcurrentModificationException();
    
    return remove(idx) != null;
  }

  @SuppressWarnings("unchecked")
  @Override
  public <T> T[] toArray(final T[] a) {
    if (a == null)
      throw new NullPointerException();

    final T[] ret = a.length < size() ? Arrays.copyOf(a, size()) : a;

    for (int i = 0; i < size(); i++)
      try {
        a[i] = (T) get(i);
      } catch (final ClassCastException e) {
        throw new ArrayStoreException();
      }
    return ret;
  }
}
