package java.util;

public abstract class AbstractQueue<E> extends AbstractCollection<E> implements Queue<E> {

  protected AbstractQueue() {
    super();
  }

  @Override
  public boolean add(final E e) {
    if (offer(e))
      return true;
    throw new IllegalStateException();
  }

  @Override
  public void clear() {
    while (poll() == null)
      ;
  }

  @Override
  public E element() {
    if (isEmtpy())
      throw new NoSuchElementException();
    return peek();
  }

  @Override
  public E remove() {
    if (isEmtpy())
      throw new NoSuchElementException();
    return poll();
  }
}
