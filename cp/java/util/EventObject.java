package java.util;

import java.io.Serializable;

public class EventObject implements Serializable {
  protected Object source;
  private static final long serialVersionUID = 1L;
  
  public EventObject(Object source)
  {
    if (source == null)
      throw new IllegalArgumentException();
    this.source = source;
  }

  public Object getSource() {
    return source;
  }
}
