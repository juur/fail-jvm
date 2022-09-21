package java.nio.charset;

public class CodingErrorAction {

  public final static CodingErrorAction IGNORE;
  public final static CodingErrorAction REPLACE;
  public final static CodingErrorAction REPORT;
  static {
    IGNORE = new CodingErrorAction("IGNORE");
    REPLACE = new CodingErrorAction("REPLACE");
    REPORT = new CodingErrorAction("REPORT");
  }

  private final String name;

  private CodingErrorAction(final String newName) {
    name = newName;
  }

  @Override
  public String toString() {
    return name;
  }
}
