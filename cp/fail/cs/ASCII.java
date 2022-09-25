package fail.cs;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CoderResult;

public class ASCII extends Charset {

  private static class Decoder extends CharsetDecoder {

    private Decoder(final Charset cs) {
      super(cs, 1.0f, 1.0f);
    }

    @Override
    protected CoderResult decodeLoop(final ByteBuffer in, final CharBuffer out) {
      while (in.hasRemaining()) {
        if (out.position() < out.capacity()) {
          final byte inb = in.get();
          if (inb > 0x7F)
            return CoderResult.malformedForLength(in.position());
          out.put((char) inb);
        } else
          return CoderResult.OVERFLOW;
      }
      return CoderResult.UNDERFLOW;
    }
  }

  private static class Encoder extends CharsetEncoder {

    private Encoder(final Charset cs) {
      super(cs, 1.0f, 1.0f);
    }

    @Override
    protected CoderResult encodeLoop(final CharBuffer in, final ByteBuffer out) {
      while (in.hasRemaining()) {
        if (out.position() < out.capacity()) {
          final char inch = in.get();
          if (inch < 0x80) {
            out.put((byte) (inch & 0x7F));
          } else
            out.put(replacement());
        } else
          return CoderResult.OVERFLOW;
      }
      return CoderResult.UNDERFLOW;
    }

  }

  public static final Charset INSTANCE;

  static {
    INSTANCE = new ASCII();
  }

  private ASCII() {
    super("us-ascii", new String[] { "us-ascii", "ASCII", "CP347", "ISO646-US" });
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
