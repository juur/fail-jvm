package java.util;

public interface SortedSet<E> extends Set<E> {

  public Comparator<? super E> comparator();

  public E first();

  public SortedSet<E> headSet(E toElement);

  public E last();

  public SortedSet<E> subSet(E fromElement, E toElement);

  public SortedSet<E> tailSet(E fromElement);
}
