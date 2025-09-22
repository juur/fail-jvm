package java.util;

public abstract class AbstractSet<E> extends AbstractCollection<E> implements Set<E> {

  protected AbstractSet() {
    super();
  }

  @Override
  public boolean equals(final Object obj) {
    if (obj == this)
      return true;
    if (!(obj instanceof Set))
      return false;
    final Set<?> oSet = (Set<?>) obj;
    if (oSet.size() != size())
      return false;
    return containsAll(oSet);
  }

  @Override
  public int hashCode() {
    int               hc = 0;
    final Iterator<E> it = iterator();

    while (it.hasNext()) {
      hc += it.next().hashCode();
    }

    return hc;
  }
}
