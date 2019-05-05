package java.util;

public interface NavigableSet<E> extends SortedSet<E> {

  public E ceiling(E e);

  public Iterator<E> descendingIterator();

  public NavigableSet<E> descendingSet();

  public E floor(E e);

  public NavigableSet<E> headSet(E toElement, boolean inclusive);

  public E higher(E e);

  public E lower(E e);

  public E pollFirst();

  public E pollLast();

  public NavigableSet<E> subSet(E fromElement, boolean fromInclusive, E toElement, boolean toInclusive);

  public NavigableSet<E> tailSet(E fromElement, boolean inclusive);
}
