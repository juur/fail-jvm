package java.io;

public class SyncFailedException extends IOException {

  public SyncFailedException(final String desc) {
    super(desc);
  }
}
