package spod.cs;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CoderResult;

public class UTF_16BE extends Charset {

  private static class Decoder extends CharsetDecoder {

    private Decoder(final Charset cs) {
      super(cs, 2.0f, 4.0f);
    }

    @Override
    protected CoderResult decodeLoop(final ByteBuffer in, final CharBuffer out) {
      if (!in.order().equals(ByteOrder.BIG_ENDIAN))
        in.order(ByteOrder.BIG_ENDIAN);

      while (in.remaining() >= 2) {
        if (out.position() < out.capacity()) {
          out.put(in.getChar());
        } else
          return CoderResult.OVERFLOW;
      }

      if (in.remaining() == 1)
        return CoderResult.malformedForLength(in.position());
      return CoderResult.UNDERFLOW;
    }

  }

  static class Encoder extends CharsetEncoder {

    Encoder(final Charset cs) {
      super(cs, 2.0f, 4.0f);
    }

    @Override
    protected CoderResult encodeLoop(final CharBuffer in, final ByteBuffer out) {
      if (!out.order().equals(ByteOrder.BIG_ENDIAN))
        out.order(ByteOrder.BIG_ENDIAN);
      while (in.hasRemaining()) {
        if (out.position() < out.capacity()) {
          out.putChar(in.get());
        } else
          return CoderResult.OVERFLOW;
      }
      return CoderResult.UNDERFLOW;
    }

  }

  public static final Charset INSTANCE;

  static {
    INSTANCE = new UTF_16BE();
  }

  private UTF_16BE() {
    super("UTF-16BE", new String[] { "UTF-16BE" });
  }

  @Override
  public boolean contains(final Charset cs) {
    // TODO Auto-generated method stub
    return false;
  }

  @Override
  public CharsetDecoder newDecoder() {
    return new Decoder(this);
  }

  @Override
  public CharsetEncoder newEncoder() {
    return new Encoder(this);
  }

}
