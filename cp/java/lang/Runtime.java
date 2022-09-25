package java.lang;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class Runtime {

  public static native Runtime getRuntime();

  private final List<Thread> shutdownList;

  private Runtime() {
    shutdownList = new ArrayList<Thread>();
  }

  public void addShutdownHook(final Thread hook) {
    synchronized (shutdownList) {
      shutdownList.add(hook);
    }
  }

  public native int availableProcessors();

  public Process exec(final String command) throws IOException {
    return exec(command, null, null);
  }

  public Process exec(final String command, final String[] envp) throws IOException {
    return exec(command, envp, null);
  }

  public Process exec(final String command,
    final String[] envp,
    final File dir)
      throws IOException {
    return exec(command.split(" \t\n\r\f"), envp, dir);
  }

  public Process exec(final String[] cmdarray) throws IOException {
    return exec(cmdarray, null, null);
  }

  public Process exec(final String[] cmdarray, final String[] envp) throws IOException {
    return exec(cmdarray, envp, null);
  }

  public Process exec(final String[] cmdarray, final String[] envp, final File dir) throws IOException {
    return null;
  }

  public void exit(final int status) {
    System.exit(status);
  }

  public native long freeMemory();

  public void gc() {
    System.gc();
  }

  public native long maxMemory();

  public void removeShutdownHook(final Thread hook) {
    synchronized (shutdownList) {
      shutdownList.remove(hook);
    }
  }

  public native long totalMemory();
}
