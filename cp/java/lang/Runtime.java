package java.lang;

import java.util.ArrayList;
import java.util.List;

public class Runtime {

  public static native Runtime getRuntime();

  private int                availableProcessors;
  private final List<Thread> shutdownList;

  private Runtime() {
    shutdownList = new ArrayList<Thread>();
  }

  public void addShutdownHook(final Thread hook) {
    shutdownList.add(hook);
  }

  public int availableProcessors() {
    return availableProcessors;
  }

  public void exit(final int status) {
    System.exit(status);
  }

  public void gc() {
    System.gc();
  }

  public void removeShutdownHook(final Thread hook) {
    shutdownList.remove(hook);
  }
}
