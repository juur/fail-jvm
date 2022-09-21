package java.util;

public final class Objects
{
	public static int hash(final Object... values)
	{
		return Arrays.hashCode(values);
	}

	public static int hashCode(final Object o)
	{
		if (o == null) return 0;
		return o.hashCode();
	}

  public static boolean equals(Object a, Object b) {
    if(a == null && b == null)
      return true;
    if(a == null || b == null)
      return false;
    return a.equals(b);
  }
}
