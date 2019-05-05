package java.io;

public class File {

  public static String pathSeparator;
  public static char   pathSeparatorChar;
  public static String separator;
  public static char   separatorChar;
  private final int    fd;

  public File(final File parent, final String child) {
    this();
  }

  public File(final String pathname) {
    this();
  }

  public File(final String parent, final String child) {
    this();
  }

  private File() {
    fd = -1;
  }

  public boolean canWrite() {
    return false;
  }

  int getFD() {
    return fd;
  }
}
