package java.io;

import java.nio.ImplFileChannel;
import java.nio.channels.FileChannel;

public class FileOutputStream extends OutputStream {

  private final FileDescriptor fdObj;
  private boolean              isClosed;
  private final FileChannel    fc;

  public FileOutputStream(final File file) throws FileNotFoundException, IOException {
    this(file, true);
  }

  public FileOutputStream(final File file, final boolean append) throws FileNotFoundException, IOException {
    isClosed = false;
    fdObj = new FileDescriptor(file);
    fdObj.open(true);
    fc = new ImplFileChannel(fdObj);
  }

  public FileOutputStream(final FileDescriptor fd) {
    isClosed = false;
    if (fd == null)
      throw new NullPointerException();
    fdObj = fd;
    fc = new ImplFileChannel(fdObj);
  }

  @Override
  public void close() throws IOException {
    if (isClosed)
      return;
    isClosed = true;
    fdObj.close();
    fc.close();
  }

  public FileChannel getChannel() {
    return fc;
  }

  /**
   * @throws IOException
   */
  public FileDescriptor getFD() throws IOException {
    return fdObj;
  }

  @Override
  native public void write(final byte[] b) throws IOException;

  @Override
  native public void write(int b) throws IOException;

}
