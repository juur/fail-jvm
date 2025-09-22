package java.nio;

public abstract class FloatBuffer extends Buffer {

  public FloatBuffer(final int size) {
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

  public abstract float get();

  public abstract float get(int index);

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

  public abstract FloatBuffer put(float f);

  public abstract FloatBuffer put(int index, float f);
}
