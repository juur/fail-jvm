package java.io;

public class FileOutputStream extends OutputStream {

  private final FileDescriptor fdObj;
  private boolean              isClosed;

  public FileOutputStream(final File file) throws FileNotFoundException {
    isClosed = false;
    try {
      fdObj = new FileDescriptor(file);
    } catch (final IOException e) {
      throw new FileNotFoundException();
    }
  }

  public FileOutputStream(final FileDescriptor fdObj) {
    isClosed = false;
    if (fdObj == null)
      throw new NullPointerException();
    this.fdObj = fdObj;
  }

  @Override
  public void close() throws IOException {
    isClosed = true;
  }

  @Override
  native public void write(final byte[] b) throws IOException;

  @Override
  native public void write(int b) throws IOException;

}
