package java.lang;

public interface CharSequence {

  char charAt(int index);

  int length();

  CharSequence subSequence(int start, int end);
}
