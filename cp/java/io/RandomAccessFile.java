package java.io;

import java.nio.channels.FileChannel;

public class RandomAccessFile implements DataOutput, DataInput, Closeable {

  private final File     file;
  private final String   mode;
  private FileDescriptor fd;
  private FileChannel    fc;

  public RandomAccessFile(final File nFile, final String nMode) {
    file = nFile;
    mode = nMode;
  }

  public RandomAccessFile(final String name, final String nMode) throws FileNotFoundException {
    if ((file = new File(name)) == null)
      throw new FileNotFoundException();
    mode = nMode;
  }

  @Override
  public void close() throws IOException {
    // TODO Auto-generated method stub

  }

  public final FileChannel getChannel() {
    return fc;
  }

  public final FileDescriptor getFD() throws IOException {
    return fd;
  }

  @Override
  public boolean readBoolean() throws IOException {
    // TODO Auto-generated method stub
    return false;
  }

  @Override
  public byte readByte() throws IOException {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public char readChar() throws IOException {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public double readDouble() throws IOException {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public float readFloat() throws IOException {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public int readInt() throws IOException {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public String readLine() throws IOException {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public long readLong() throws IOException {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public short readShort() throws IOException {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public int readUnsignedByte() throws IOException {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public int readUnsignedShort() throws IOException {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public String readUTF() throws IOException {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public void write(final byte[] b) throws IOException {
    // TODO Auto-generated method stub

  }

  @Override
  public void write(final byte[] b, final int off, final int len) throws IOException {
    // TODO Auto-generated method stub

  }

  @Override
  public void write(final int b) throws IOException {
    // TODO Auto-generated method stub

  }

  @Override
  public void writeBoolean(final boolean v) throws IOException {
    // TODO Auto-generated method stub

  }

  @Override
  public void writeByte(final int v) throws IOException {
    // TODO Auto-generated method stub

  }

  @Override
  public void writeBytes(final String s) throws IOException {
    // TODO Auto-generated method stub

  }

  @Override
  public void writeChar(final int v) throws IOException {
    // TODO Auto-generated method stub

  }

  @Override
  public void writeChars(final String s) throws IOException {
    // TODO Auto-generated method stub

  }

  @Override
  public void writeDouble(final double v) throws IOException {
    // TODO Auto-generated method stub

  }

  @Override
  public void writeFloat(final float v) throws IOException {
    // TODO Auto-generated method stub

  }

  @Override
  public void writeInt(final int v) throws IOException {
    // TODO Auto-generated method stub

  }

  @Override
  public void writeLong(final long v) throws IOException {
    // TODO Auto-generated method stub

  }

  @Override
  public void writeShort(final int v) throws IOException {
    // TODO Auto-generated method stub

  }

  @Override
  public void writeUTF(final String s) throws IOException {
    // TODO Auto-generated method stub

  }

}
