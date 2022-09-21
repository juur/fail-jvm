package java.io;

public class FileOutputStream extends OutputStream {

  private final FileDescriptor fdObj;
  private boolean              isClosed;

  public FileOutputStream(final File file) throws FileNotFoundException, IOException {
    isClosed = false;
    fdObj = new FileDescriptor(file);
    fdObj.open(true);
  }

  public FileOutputStream(final FileDescriptor fd) {
    isClosed = false;
    if (fd == null)
      throw new NullPointerException();
    fdObj = fd;
  }

  @Override
  public void close() throws IOException {
    if (isClosed)
      return;
    isClosed = true;
    fdObj.close();
  }

  @Override
  native public void write(final byte[] b) throws IOException;

  @Override
  native public void write(int b) throws IOException;

}
