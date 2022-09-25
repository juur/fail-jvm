package java.io;

public class File implements Serializable, Comparable<File> {

  public static String pathSeparator;
  public static char   pathSeparatorChar;
  public static String separator;
  public static char   separatorChar;

  private static final long serialVersionUID = 1L;

  private final String pathname;
  private boolean isFile, isDirectory, isHidden;
  private boolean canExecute, canRead, canWrite;

  long            lastModified, length;

  public File(final File parent, final String child) {
    this(parent + separator + child);

  }

  public File(final String newPathname) {
    pathname = newPathname;
  }

  public boolean canExecute() {
    access();
    return canExecute;
  }

  public boolean canRead() {
    access();
    return canRead;
  }

  public boolean canWrite() {
    access();
    return canWrite;
  }

  @Override
  public int compareTo(final File comp) {
    return toString().compareTo(comp.toString());
  }

  public String getAbsolutePath() {
    return null;
  }

  public boolean isDirectory() {
    stat();
    return isDirectory();
  }

  public boolean isFile() {
    stat();
    return isFile;
  }

  public boolean isHidden() {
    stat();
    return isHidden();
  }

  public long lastModified() {
    stat();
    return lastModified;
  }

  public long length() {
    stat();
    return length;
  }

  @Override
  public String toString() {
    return pathname;
  }

  private native void access();

  private native void creat();

  private native void delete();

  private native void stat();
}
