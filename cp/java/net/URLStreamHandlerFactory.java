package java.net;


public interface URLStreamHandlerFactory {

  public URLStreamHandler createURLStreamHandler(String protocol);
}
