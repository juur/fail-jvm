package java.lang;

import java.util.Arrays;

public class ThreadGroup implements Thread.UncaughtExceptionHandler {

  private int      nthreads;
  private Thread[] threads;

  public ThreadGroup() {
    threads = new Thread[5];
    nthreads = 0;
  }

  @Override
  public void uncaughtException(final Thread t, final Throwable e) {}

  void add(final Thread thread) {
    synchronized (this) {
      if (nthreads == threads.length) {
        threads = Arrays.copyOf(threads, nthreads * 2);
      }
      threads[nthreads++] = thread;
    }
  }

}
