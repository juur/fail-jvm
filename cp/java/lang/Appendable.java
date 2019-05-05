package java.lang;

import java.io.IOException;

public interface Appendable {

  Appendable append(char c) throws IOException;

  Appendable append(CharSequence csq) throws IOException;

  Appendable append(CharSequence csq, int start, int end) throws IOException;
}
