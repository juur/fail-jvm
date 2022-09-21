package java.nio.channels;


public abstract class SelectionKey {

  public static final int OP_ACCEPT  = 16;
  public static final int OP_CONNECT = 8;
  public static final int OP_READ    = 1;
  public static final int OP_WRITE   = 4;
  private Object          attached;

  protected SelectionKey() {

  }

  public final Object attach(final Object ob)
  {
    final Object old = attached;
    attached = ob;
    return old;
  }

  public final Object attachment()
  {
    return attached;
  }

  public abstract void cancel();

  public abstract SelectableChannel channel();

  public abstract int interestOps();

  public abstract SelectionKey interestOps(int ops);

  public final boolean isAcceptable() {
    return (readyOps() & OP_ACCEPT) != 0;
  }

  public final boolean isConnectable()
  {
    return (readyOps() & OP_CONNECT) != 0;
  }

  public final boolean isReadable() {
    return (readyOps() & OP_READ) != 0;
  }

  public abstract boolean isValid();

  public final boolean isWritable() {
    return (readyOps() & OP_WRITE) != 0;
  }

  public abstract int readyOps();

  public abstract Selector selector();
}
