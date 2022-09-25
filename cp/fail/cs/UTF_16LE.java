package fail.cs;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CoderResult;

public class UTF_16LE extends Charset {

  private static class Decoder extends CharsetDecoder {

    public Decoder(final Charset cs) {
      super(cs, 2.0f, 4.0f);
    }

    @Override
    protected CoderResult decodeLoop(final ByteBuffer in, final CharBuffer out) {
      if (!in.order().equals(ByteOrder.LITTLE_ENDIAN))
        in.order(ByteOrder.LITTLE_ENDIAN);

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

  private static class Encoder extends CharsetEncoder {

    public Encoder(final Charset cs) {
      super(cs, 2.0f, 4.0f);
    }

    @Override
    protected CoderResult encodeLoop(final CharBuffer in, final ByteBuffer out) {
      if (!out.order().equals(ByteOrder.LITTLE_ENDIAN))
        out.order(ByteOrder.LITTLE_ENDIAN);
      while (in.hasRemaining()) {
        if (out.position() < out.capacity()) {
          out.putChar(in.get());
        } else
          return CoderResult.OVERFLOW;
      }
      return CoderResult.UNDERFLOW;
    }

  }

  public static final UTF_16LE INSTANCE;

  static {
    INSTANCE = new UTF_16LE();
  }

  private UTF_16LE() {
    super("UTF-16LE", new String[] { "UTF-16LE" });
  }

  @Override
  public boolean contains(final Charset cs) {
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
