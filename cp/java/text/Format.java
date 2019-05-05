package java.text;

public abstract class Format {

  public static class Field extends AttributedCharacterIterator.Attribute {

    public Field(final String name) {
      super(name);
    }

  }

  protected Format() {

  }

  public final String format(final Object obj) {
    return format(obj, new StringBuffer(), new FieldPosition(0)).toString();
  }

  public abstract StringBuffer format(Object obj, StringBuffer toAppendTo, FieldPosition pos);

  public AttributedCharacterIterator formatToCharacterIterator(final Object obj) {
    return null;
  }

  public Object parseObject(final String source) {
    return parseObject(source, null);
  }

  public abstract Object parseObject(String source, ParsePosition pos);

}
