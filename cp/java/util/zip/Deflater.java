package java.util.zip;

public class Deflater {

  public static final int BEST_COMPRESSION    = 9;
  public static final int BEST_SPEED          = 1;
  public static final int DEFAULT_COMPRESSION = -1;
  public static final int DEFAULT_STRATEGY    = 0;
  public static final int DEFLATED            = 8;
  public static final int FILTERED            = 1;
  public static final int HUFFMAN_ONLY        = 2;
  public static final int NO_COMPRESSION      = 0;
  private final int       comp_level;
  private final boolean   nowrap;

  public Deflater() {
    this(DEFAULT_COMPRESSION, false);
  }

  public Deflater(final int level) {
    this(level, false);
  }

  public Deflater(int level, final boolean nowrap) {
    if (level == DEFAULT_COMPRESSION)
      level = 4;

    if (level < BEST_SPEED || level > BEST_COMPRESSION)
      throw new IllegalArgumentException("Invalid level");

    this.nowrap = nowrap;
    comp_level = level;
  }
}
