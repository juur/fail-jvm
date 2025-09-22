package java.io;

public final class FileDescriptor {

  public final static FileDescriptor err;
  public final static FileDescriptor in;
  public final static FileDescriptor out;

  static {
    in = new FileDescriptor(0);
    out = new FileDescriptor(1);
    err = new FileDescriptor(2);
  }

  private int  fd = -1;
  private File file;

  FileDescriptor(final File newFile) {
    file = newFile;
  }

  FileDescriptor(final int i) {
    fd = i;
  }

  public void close() throws IOException {
    if (fd == -1)
      throw new IOException("invalid FileDescriptor (-1)");
    sysClose(fd);
    fd = -1;
  }

  public void open(final boolean b) throws FileNotFoundException, IOException {
    if (file == null || fd != -1)
      throw new IOException("file or fd invalid");
    fd = sysOpen(file.toString(), b);
  }

  public native void sync() throws SyncFailedException;

  private native void sysClose(int fdesc) throws IOException;

  private native int sysOpen(String path, boolean isWrite) throws IOException, FileNotFoundException;

  public native boolean valid();
}
