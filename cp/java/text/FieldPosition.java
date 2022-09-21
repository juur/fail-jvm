package java.text;

import java.text.Format.Field;

public class FieldPosition {

  private Field attribute;
  private int   bi;
  private int   ei;
  private int   fieldId;

  public FieldPosition(final Format.Field newAttribute) {
    this.attribute = newAttribute;
  }

  public FieldPosition(final Format.Field newAttribute, final int fieldID) {
    this.attribute = newAttribute;
    fieldId = fieldID;
  }

  public FieldPosition(final int field) {
    fieldId = field;
  }

  public int getBeginIndex() {
    return bi;
  }

  public int getEndIndex() {
    return ei;
  }

  public int getField() {
    return fieldId;
  }

  public Format.Field getFieldAttribute() {
    return attribute;
  }

  public void setBeginIndex(final int beginIndex) {
    this.bi = beginIndex;
  }

  public void setEndIndex(final int endIndex) {
    this.ei = endIndex;
  }
}
