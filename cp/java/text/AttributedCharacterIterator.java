package java.text;

public interface AttributedCharacterIterator extends CharacterIterator {

  public static class Attribute {

    public static final Attribute INPUT_METHOD_SEGMENT;
    public static final Attribute LANGUAGE;
    public static final Attribute READING;
    static {
      LANGUAGE = new Attribute("LANGUAGE");
      READING = new Attribute("READING");
      INPUT_METHOD_SEGMENT = new Attribute("INPUT_METHOD_SEGMENT");
    }

    private final String name;

    public Attribute(final String name) {
      this.name = name;
    }

    protected String getName() {
      return name;
    }

    protected Object readResolve() {
      return null;
    }
  }
}
