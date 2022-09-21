package java.io;


public interface ObjectOutput extends DataOutput {

  void close() throws IOException;

  void flush() throws IOException;

  void writeObject(Object obj) throws IOException;
}
