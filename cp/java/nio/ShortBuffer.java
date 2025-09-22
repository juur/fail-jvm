package java.nio;

public abstract class ShortBuffer extends Buffer {

  public ShortBuffer(final int size) {
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

  public abstract short get();

  public abstract short get(int index);

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

  public abstract ShortBuffer put(int index, short s);

  public abstract ShortBuffer put(short s);
}
