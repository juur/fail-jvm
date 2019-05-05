package java.lang.reflect;

public interface Member {

  public static final int DECLARED = 1;
  public static final int PUBLIC   = 0;

  public Class<?> getDeclaringClass();

  public int getModifiers();

  public String getName();

  public boolean isSynthetic();
}
