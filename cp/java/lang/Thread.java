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
  private static int      threadCount   = 0;

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

  public Thread(final Runnable target) {
    this(null, target, "Thread" + threadCount++);
  }

  public Thread(final Runnable target, final String name) {
    this(null, target, name);
  }

  public Thread(final String name) {
    this(null, null, name);
  }

  public Thread(final ThreadGroup group, final Runnable target) {
    this(group, target, "Thread" + threadCount++);
  }

  public Thread(final ThreadGroup group, final Runnable target, final String name) {
    this.name = name;
    this.target = target;
    this.group = group;
    // state = State.NEW;
  }

  public Thread(final ThreadGroup group, final Runnable target, final String name, final long stackSize) {
    this(group, null, "Thread" + threadCount++);
  }

  public Thread(final ThreadGroup group, final String name) {
    this(group, null, "Thread" + threadCount++);
  }

  public String getName() {
    return name;
  }

  // public State getState() {
  // return state;
  // }

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

  private native void nativeStart();
}
