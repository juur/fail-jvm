package java.util;

public class ArrayList<E> extends AbstractList<E> implements RandomAccess {

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
      return list.get(ptr++);
    }

    @Override
    public int nextIndex() {
      return ptr + 1;
    }

    @Override
    public E previous() {
      return list.get(--ptr);
    }

    @Override
    public int previousIndex() {
      return ptr - 1;
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

  public ArrayList(final Object[] array) {
    this.array = array;
    this.last = array.length - 1;
  }

  @Override
  public boolean add(final E e) {
    if (last >= array.length)
      array = Arrays.copyOf(array, array.length * 2);
    array[last++] = e;
    return true;
  }

  @SuppressWarnings("unchecked")
  @Override
  public E get(final int index) {
    if (index < 0 || index > size())
      throw new IndexOutOfBoundsException();
    return (E) array[index];
  }

  @Override
  public ListIterator<E> listIterator(final int index) {
    return new ListIteratorImpl<E>(this, index);
  }

  @Override
  public E set(final int index, final E element) {
    if (index < 0 || index > size())
      throw new IndexOutOfBoundsException();
    array[index] = element;
    return element;
  }

  @Override
  public int size() {
    return array.length;
  }

  @Override
  public List<E> subList(final int fromIndex, final int toIndex) {
    return null;
  }

}
