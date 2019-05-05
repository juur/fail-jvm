package java.nio;

public abstract class DoubleBuffer extends Buffer {

  public DoubleBuffer(final int size) {
    super(size);
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

}
