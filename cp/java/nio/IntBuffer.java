package java.nio;

public abstract class IntBuffer extends Buffer {

  public IntBuffer(final int size) {
    super(size);
    // TODO Auto-generated constructor stub
  }

  @Override
  public Object array() {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public int arrayOffset() {
    // TODO Auto-generated method stub
    return 0;
  }

  public abstract int get();

  public abstract int get(int index);

  @Override
  public boolean hasArray() {
    // TODO Auto-generated method stub
    return false;
  }

  @Override
  public boolean isDirect() {
    // TODO Auto-generated method stub
    return false;
  }

  @Override
  public boolean isReadOnly() {
    // TODO Auto-generated method stub
    return false;
  }

  public abstract IntBuffer put(int i);

  public abstract IntBuffer put(int index, int i);
}
