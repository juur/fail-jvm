package java.lang;

public class Thread implements Runnable {

  public static enum State {
    BLOCKED, NEW, RUNNABLE, TERMINATED, TIMED_WAITING, WAITING
  }

  public static interface UncaughtExceptionHandler {

    void uncaughtException(Thread t, Throwable e);
  }

  public static final int MAX_PRIORITY = 10;
  public static final int MIN_PRIORITY  = 1;
  public static final int NORM_PRIORITY = 5;

  volatile private static int threadCount = 0;
  private static final Object threadCountLock = new Object();

  public static native Thread currentThread();

  public static void sleep(final long millis) throws InterruptedException {
    sleep(millis, 0);
  }

  public static native void sleep(long millis, int nanos) throws InterruptedException;

  private final ThreadGroup group;
  // private final State state;
  private final String   name;
  private final Runnable target;

  public Thread() {
    this(null, null, "Thread" + threadCount++);
  }

  public Thread(final Runnable newTarget) {
    this(null, newTarget, "Thread" + threadCount++);
  }

  public Thread(final Runnable newTarget, final String newName) {
    this(null, newTarget, newName);
  }

  public Thread(final String newName) {
    this(null, null, newName);
  }

  public Thread(final ThreadGroup newGroup, final Runnable newTarget) {
    this(newGroup, newTarget, "Thread" + threadCount++);
  }

  public Thread(final ThreadGroup newGroup, final Runnable newTarget, final String newName) {
    name = newName;
    target = newTarget;
    group = newGroup;
    // state = State.NEW;
  }

  public Thread(final ThreadGroup newGroup, final Runnable newTarget, final String newName, final long stackSize) {
    this(newGroup, null, "Thread" + threadCount++);
  }

  public Thread(final ThreadGroup newGroup, final String newName) {
    this(newGroup, null, "Thread" + threadCount++);
  }

  public String getName() {
    return name;
  }

  // public State getState() {
  // return state;
  // }

  private native void nativeStart();

  @Override
  public void run() {
    if (target != null)
      target.run();
  }

  public void start() {
    // if (getState() != State.NEW)
    // throw new IllegalThreadStateException("Thread is not NEW");

    if (group != null)
      group.add(this);
    nativeStart();
  }
}
