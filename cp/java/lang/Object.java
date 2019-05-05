package java.lang;

public class Object {

  public Object() {}

  public boolean equals(final Object obj) {
    if (obj == null)
      return false;
    if (!obj.getClass().equals(getClass()))
      return false;
    if (obj.hashCode() == hashCode())
      return true;
    return false;
  }

  public final native Class<?> getClass();

  public native int hashCode();

  public final native void notify();

  public final native void notifyAll();

  public String toString() {
    return getClass().getSimpleName() + ":" + hashCode();
  }

  public final void wait() throws InterruptedException {
    wait(0);
  }

  public final void wait(final long timeout) throws InterruptedException {
    wait(timeout, 0);
  }

  public final native void wait(long timeout, long nanos) throws InterruptedException;

  protected Object clone() throws CloneNotSupportedException {
    throw new CloneNotSupportedException();
  }

  protected void finalize() throws Throwable {}
}
