package java.util;

public interface List<E> extends Collection<E> {

  public void add(int index, E element);

  public E get(int index);

  public int indexOf(Object o);

  public int lastIndexOf(Object o);

  public ListIterator<E> listIterator();

  public ListIterator<E> listIterator(int index);

  public E remove(int index);

  public E set(int index, E element);

  List<E> subList(int fromIndex, int toIndex);
}
