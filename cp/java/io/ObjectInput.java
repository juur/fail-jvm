package java.io;

public interface ObjectInput extends DataInput {

  int available() throws IOException;

  void close() throws IOException;

  int read() throws IOException;

  int read(byte[] b) throws IOException;

  int read(byte[] b, int off, int len) throws IOException;

  Object readObject() throws ClassNotFoundException, IOException;

  long skip(long n) throws IOException;
}
