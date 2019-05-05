package java.io;

public final class FileDescriptor {

  public final static FileDescriptor err;
  public final static FileDescriptor in;
  public final static FileDescriptor out;

  static {
    err = new FileDescriptor(2);
    in = new FileDescriptor(0);
    out = new FileDescriptor(1);
  }

  private final int fd;

  public FileDescriptor() {
    this(-1);
  }

  private FileDescriptor(final int i) {
    fd = i;
  }

  FileDescriptor(final File file) throws IOException {
    if (file.canWrite())
      fd = file.getFD();
    throw new IOException();
  }

  public native void sync() throws SyncFailedException;

  public native boolean valid();
}
