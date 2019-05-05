package spod.cs;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CoderResult;

public class UTF_16 extends Charset {

  private static class Decoder extends CharsetDecoder {

    private Charset detected = null;
    private boolean start;

    protected Decoder(final Charset cs) {
      super(cs, 2.0f, 4.0f);
      start = true;
    }

    @Override
    public Charset detectedCharset() {
      if (detected == null)
        throw new IllegalStateException();
      return detected;
    }

    @Override
    public boolean isAutoDetecting() {
      return true;
    }

    @Override
    public boolean isCharsetDetected() {
      return !start;
    }

    @Override
    protected CoderResult decodeLoop(final ByteBuffer in, final CharBuffer out) {
      while (in.remaining() >= 2) {
        if (out.position() < out.capacity()) {
          if (start) {
            start = false;

            final byte first  = in.get();
            final byte second = in.get();

            if (first == 0xFE || first == 0xFF)
              if (first == 0xFE && second == 0xFF) {
                in.order(ByteOrder.BIG_ENDIAN);
                detected = UTF_16BE.INSTANCE;
                continue;
              } else if (first == 0xFF && second == 0xFE) {
                in.order(ByteOrder.LITTLE_ENDIAN);
                detected = UTF_16LE.INSTANCE;
                continue;
              }
            in.rewind();
          }
          out.put(in.getChar());
        } else
          return CoderResult.OVERFLOW;
      }
      if (in.remaining() == 1)
        return CoderResult.malformedForLength(in.position());

      return CoderResult.UNDERFLOW;
    }
  }

  public static final Charset INSTANCE;

  static {
    INSTANCE = new UTF_16();
  }

  protected UTF_16() {
    super("UTF-16", new String[] { "UTF-16", "csUTF-16" });
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
    return new UTF_16BE.Encoder(this);
  }

}
