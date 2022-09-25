package java.io;

import java.nio.ImplFileChannel;
import java.nio.channels.FileChannel;

public class FileInputStream extends InputStream {

  private final FileDescriptor fdObj;
  private boolean              isClosed;
  private final FileChannel    fc;

  public FileInputStream(final File file) throws FileNotFoundException, IOException {
    isClosed = false;
    fdObj = new FileDescriptor(file);
    fdObj.open(false);
    fc = new ImplFileChannel(fdObj);
  }

  public FileInputStream(final FileDescriptor fd) {
    isClosed = false;
    if (fd == null)
      throw new NullPointerException();
    fdObj = fd;
    fc = new ImplFileChannel(fdObj);
  }

  @Override
  public int available() throws IOException {
    if (isClosed)
      throw new IOException();
    return 1;
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
  public final FileDescriptor getFD() throws IOException {
    return fdObj;
  }

  @Override
  native public int read() throws IOException;

  @Override
  public int read(final byte[] b) throws IOException {
    return read(b, 0, b.length);
  }

  @Override
  native public int read(byte[] b, int off, int len) throws IOException;

  @Override
  native public long skip(long n);
}
