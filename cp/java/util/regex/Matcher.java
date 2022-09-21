package java.util.regex;

public class Matcher implements MatchResult {

  private class MatchResultImpl implements MatchResult {

    @Override
    public int end() {
      // TODO Auto-generated method stub
      return 0;
    }

    @Override
    public int end(final int group) {
      // TODO Auto-generated method stub
      return 0;
    }

    @Override
    public String group() {
      // TODO Auto-generated method stub
      return null;
    }

    @Override
    public String group(final int group) {
      // TODO Auto-generated method stub
      return null;
    }

    @Override
    public int groupCount() {
      // TODO Auto-generated method stub
      return 0;
    }

    @Override
    public int start() {
      // TODO Auto-generated method stub
      return 0;
    }

    @Override
    public int start(final int group) {
      // TODO Auto-generated method stub
      return 0;
    }

  }

  public static String quoteReplacement(final String s) {
    return s;
  }

  private CharSequence input;
  private Pattern            pattern;
  private boolean            anchor;
  private boolean            trans;
  private int                regionEnd;
  private int                regionStart;
  private int                index;

  Matcher(final Pattern newPattern, final CharSequence newInput) {
    pattern = newPattern;
    input = newInput;
    index = 0;
  }

  public Matcher appendReplacement(final StringBuffer sb, final String replacement) {
    // TODO
    return this;
  }

  public StringBuffer appendTail(final StringBuffer sb) {
    return sb;
  }

  @Override
  public int end() {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public int end(final int group) {
    // TODO Auto-generated method stub
    return 0;
  }

  public boolean find() {
    return false;
  }

  public boolean find(final int newStart) {
    reset();
    index = newStart;
    return find();
  }

  @Override
  public String group() {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public String group(final int group) {
    // TODO Auto-generated method stub
    return null;
  }

  @Override
  public int groupCount() {
    // TODO Auto-generated method stub
    return 0;
  }

  public boolean hasAnchoringBounds() {
    return anchor;
  }

  public boolean hasTransparentBounds() {
    return trans;
  }

  public boolean hitEnd() {
    // TODO
    return false;
  }

  public boolean lookingAt() {
    // TODO
    return false;
  }

  public boolean matches() {
    // TODO
    return false;
  }

  public Pattern pattern() {
    return pattern;
  }

  public Matcher region(final int newStart, final int newEnd) {
    if (regionStart < 0 || regionEnd < 0 || regionEnd < regionStart || regionStart > input.length() || regionEnd > input.length())
      throw new IndexOutOfBoundsException();
    regionStart = newStart;
    regionEnd = newEnd;
    return this;
  }

  public int regionEnd() {
    return regionEnd;
  }

  public int regionStart() {
    return regionStart;
  }

  public String replaceAll(final String replacement) {
    return "";
  }

  public String replaceFirst(final String replacement) {
    return "";
  }

  public boolean requireEnd() {
    // TODO
    return false;
  }

  public Matcher reset() {
    index = 0;
    regionStart = 0;
    regionEnd = input.length();
    return this;
  }

  public Matcher reset(final CharSequence newInput) {
    input = newInput;
    reset();
    return this;
  }

  @Override
  public int start() {
    // TODO Auto-generated method stub
    return 0;
  }

  @Override
  public int start(final int group) {
    // TODO Auto-generated method stub
    return 0;
  }

  public MatchResult toMatchResult() {
    return new MatchResultImpl();
  }

  public Matcher useAnchoringBounds(final boolean b) {
    anchor = b;
    return this;
  }

  public Matcher usePattern(final Pattern newPattern) {
    pattern = newPattern;
    return this;
  }

  public Matcher useTransparentBounds(final boolean b) {
    trans = b;
    return this;
  }

}