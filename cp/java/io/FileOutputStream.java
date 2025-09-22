package java.io;

import java.nio.channels.FileChannel;
import java.nio.channels.ImplFileChannel;

public class FileOutputStream extends OutputStream {

  private final FileDescriptor  fdObj;
  private boolean               isClosed;
  private final ImplFileChannel fc;

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
  public native void write(final byte[] b)
    throws IOException;/*
                        * { String debugText = "FileOutputStream.write: ";
                        * fail.Debug.print(debugText.getBytes()); fail.Debug.print(new
                        * String(b).getBytes()); fc.write(ByteBuffer.wrap(b)); }
                        */

  @Override
  public native void write(final int b) throws IOException;/*
                                                            * { fc.write(ByteBuffer.allocate(4).putInt(b)); }
                                                            */

}
