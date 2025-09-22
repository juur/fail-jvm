package java.util.zip;


public class Inflater {
  private final boolean nowrap;

  public Inflater()
  {
    this(false);
  }

  public Inflater(final boolean nowrap)
  {
    this.nowrap = nowrap;
  }

  public void end() {

  }

  public boolean finished() {
    return true;
  }

  public int getAdler() {
    return 0;
  }

  public long getBytesRead() {
    return 0;
  }

  public long getBytesWritten() {
    return 0;
  }

  public int getRemaining() {
    return 0;
  }

  public int getTotalIn() {
    return (int) getBytesRead();
  }

  public int getTotalOut() {
    return (int) getBytesWritten();
  }

  public int inflate(final byte[] b) throws DataFormatException {
    return inflate(b, 0, b.length);
  }

  public int inflate(final byte[] b, final int off, final int len) throws DataFormatException
  {
    return 0;
  }

  public boolean needsDictionary() {
    return false;
  }

  public boolean needsInput() {
    return false;
  }

  public void reset() {

  }

  public void setDictionary(final byte[] b) {
    setDictionary(b, 0, b.length);
  }

  public void setDictionary(final byte[] b, final int off, final int len) {

  }

  public void setInput(final byte[] b) {
    setInput(b, 0, b.length);
  }

  public void setInput(final byte[] b, final int off, final int len)
  {

  }
}
