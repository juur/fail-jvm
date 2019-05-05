package java.util;

public interface Deque<E> extends Queue<E> {

  public void addFirst(E e);

  public void addLast(E e);

  public Iterator<E> descendingIterator();

  public E getFirst();

  public E getLast();

  public boolean offerFirst(E e);

  public boolean offerLast(E e);

  public E peekFirst();

  public E peekLast();

  public E pollFirst();

  public E pollLast();

  public void push(E e);

  public E removeFirst();

  public E removeLast();

  boolean removeLastOccurrence(Object o);
}
