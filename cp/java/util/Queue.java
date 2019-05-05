package java.util;

public interface Queue<E> extends Collection<E> {

  public E element();

  public boolean offer(E e);

  public E peek();

  public E poll();

  public E remove();
}
