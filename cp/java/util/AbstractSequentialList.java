package java.util;

public abstract class AbstractSequentialList<E> extends AbstractList<E> {

  @Override
  public void add(final int index, final E element) {
    final ListIterator<E> it = listIterator(index);
    it.add(element);
  }

  @Override
  public E get(final int index) {
    final ListIterator<E> it = listIterator(index);
    return it.next();
  }

  @Override
  public E remove(final int index) {
    final ListIterator<E> it  = listIterator(index);
    final E               ret = it.next();
    it.remove();
    return ret;
  }

  @Override
  public E set(final int index, final E element) {
    final ListIterator<E> it  = listIterator(index);
    final E               old = it.next();
    it.set(element);
    return old;
  }
}
