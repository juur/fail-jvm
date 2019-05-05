package java.util;

public interface Collection<E> extends Iterable<E> {

  public boolean add(E e);

  public boolean addAll(Collection<? extends E> c);

  public void clear();

  public boolean contains(Object o);

  public boolean containsAll(Collection<?> c);

  public boolean isEmtpy();

  public boolean remove(Object o);

  public boolean removeAll(Collection<?> c);

  public boolean retainAll(Collection<?> c);

  public int size();

  public Object[] toArray();

  public <T> T[] toArray(T[] a);
}
