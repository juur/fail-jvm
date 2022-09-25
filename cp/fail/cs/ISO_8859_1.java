package fail.cs;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CoderResult;

public class ISO_8859_1 extends Charset {

  private static class Decoder extends CharsetDecoder {

    private Decoder(final Charset cs) {
      super(cs, 1.0f, 1.0f);
    }

    @Override
    protected CoderResult decodeLoop(final ByteBuffer in, final CharBuffer out) {
      while (in.hasRemaining()) {
        if (out.position() < out.capacity()) {
          final byte inb = in.get();
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
          if (inch < 0x100)
            out.put((byte) (inch & 0xFF));
          else
            out.put(replacement());
        } else
          return CoderResult.OVERFLOW;
      }
      return CoderResult.UNDERFLOW;
    }

  }

  public static final Charset INSTANCE;

  static {
    INSTANCE = new ISO_8859_1();
  }

  private ISO_8859_1() {
    super("ISO-8859-1", new String[] { "ISO-8859-1", "latin1", "CP819", "IBM819", "iso-r-100", "csISOLatin1", "l1" });
  }

  @Override
  public boolean contains(final Charset cs) {
    if (cs.name().equals("us-ascii"))
      return true;
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
