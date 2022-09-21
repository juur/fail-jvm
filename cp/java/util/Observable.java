package java.util;


public class Observable {

  private final Set<Observer> set;
  private boolean changed;

  public Observable()
  {
    set = new HashSet<Observer>();
    changed = false;
  }

  public void addobserver(final Observer o)
  {
    if (set.contains(o))
      return;
    set.add(o);
  }

  public int countObservers() {
    return set.size();
  }

  public void deleteObserver(final Observer o)
  {
    set.remove(o);
  }

  public void deleteObservers()
  {
    set.clear();
  }

  public boolean hasChanged()
  {
    return changed;
  }

  public void notifyObservers()
  {
    notifyObservers(null);
  }

  public void notifyObservers(final Object o)
  {
    if (!hasChanged())
      return;

    for(final Observer observer : set)
    {
      observer.update(this, o);
    }
  }

  protected void clearChanged() {
    changed = false;
  }

  protected void setChanged()
  {
    changed = true;
  }
}
