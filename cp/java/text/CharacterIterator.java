package java.text;

public interface CharacterIterator {

  public static final char DONE = 65535;

  public char current();

  public char first();

  public int getBeginIndex();

  public int getEndIndex();

  public char last();

  public char next();

  public char previous();

  public char setIndex(int position);
}
