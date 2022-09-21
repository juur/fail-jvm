package java.io;


public interface Externalizable extends Serializable {

  void readExternal(ObjectInput in) throws IOException, ClassNotFoundException;

  void writeExternal(ObjectOutput in) throws IOException;
}
