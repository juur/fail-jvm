package java.util.regex;


public interface MatchResult {

  int end();

  int end(int group);

  String group();

  String group(int group);

  int groupCount();

  int start();

  int start(int group);
}
