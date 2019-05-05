package java.text;

public class ParsePosition {

  private int index, errIndex = -1;

  public ParsePosition(final int index) {
    this.index = index;
  }

  public int getErrorIndex() {
    return errIndex;
  }

  public int getIndex() {
    return index;
  }

  public void setErrorIndex(final int ei) {
    errIndex = ei;
  }

  public void setIndex(final int index) {
    this.index = index;
  }
}
