package java.util;

import java.io.Serializable;

public class EventObject implements Serializable {
  private static final long serialVersionUID = 1L;
  protected Object source;

  public EventObject(final Object nSource)
  {
    if (nSource == null)
      throw new IllegalArgumentException();
    source = nSource;
  }

  public Object getSource() {
    return source;
  }
}
