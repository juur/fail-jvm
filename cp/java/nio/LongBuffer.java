package java.nio;

public abstract class LongBuffer extends Buffer {

  public LongBuffer(final int size) {
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

  public abstract long get();

  public abstract long get(int index);

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

  public abstract LongBuffer put(int index, long l);

  public abstract LongBuffer put(long l);
}
