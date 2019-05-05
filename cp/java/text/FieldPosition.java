package java.text;

import java.text.Format.Field;

public class FieldPosition {

  private Field attribute;
  private int   bi;
  private int   ei;
  private int   fieldId;

  public FieldPosition(final Format.Field attribute) {
    this.attribute = attribute;
  }

  public FieldPosition(final Format.Field attribute, final int fieldID) {
    this.attribute = attribute;
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

  public void setBeginIndex(final int bi) {
    this.bi = bi;
  }

  public void setEndIndex(final int ei) {
    this.ei = ei;
  }
}
