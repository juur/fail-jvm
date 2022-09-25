package fail;

import java.util.Comparator;

public class CaseInsensitiveOrder implements Comparator<String> {

  @Override
  public int compare(final String o1, final String o2) {
    return o1.toLowerCase().compareTo(o2.toLowerCase());
  }

}
