package spod.cs;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CoderResult;

public class UTF_8 extends Charset {

  private static class Decoder extends CharsetDecoder {

    private boolean start;

    private Decoder(final Charset cs) {
      super(cs, 1.0f, 4.0f);
      start = true;
    }

    @Override
    protected CoderResult decodeLoop(final ByteBuffer in, final CharBuffer out) {
      while (in.hasRemaining()) {
        if (out.position() < out.limit()) {
          byte inch = in.get(in.position());

          if (inch < 0x80) {
            //System.out.println("1: Got: " + (int) inch);
            start = false;
            // Byte 1
            out.put((char) in.get());
          } else if (start && inch == 0xEF && in.remaining() > 2) {
            // skip UTF-8 encoding of a BOM
            start = false;
            if (in.get(in.position() + 1) != 0xbb)
              continue;
            if (in.get(in.position() + 2) != 0xbf)
              continue;
            in.position(in.position() + 3);
          } else {
            start = false;
            int tmp = 0;

            if ((inch & 0xF8) == 0xF0) {
              if (in.remaining() < 4)
                return CoderResult.UNDERFLOW;

              //System.out.println("4: ");

              // Byte 1
              inch = in.get();
              tmp = (char) (inch & 0x7);
              tmp <<= 6;
              // Byte 2
              inch = in.get();
              tmp |= (char) (inch & 0x7F);
              tmp <<= 6;
              // Byte 3
              inch = in.get();
              tmp |= (char) (inch & 0x7F);
              tmp <<= 6;
              // Byte 4
              inch = in.get();
              tmp |= (char) (inch & 0x7F);

              // tmp contains a 32bit code point now

              tmp -= 0x10000;
              out.put((char) ((tmp & 0x3ff) + 0xd800));
              tmp >>= 10;
              out.put((char) ((tmp & 0x3ff) + 0xdc00));
              // are these two in the right order?

              return CoderResult.unmappableForLength(out.position());
            } else if ((inch & 0xF0) == 0xE0) {
              if (in.remaining() < 3)
                return CoderResult.UNDERFLOW;

              // Byte 1
              inch = in.get();
              //System.out.println("3: Got: " + (int) inch);
              tmp = (char) (inch & 0x0F);
              tmp <<= 6;
              //System.out.println("3: now: " + tmp);
              // Byte 2
              inch = in.get();
              //System.out.println("3: Got: " + (int) inch);
              tmp |= (char) (inch & 0x7F);
              tmp <<= 6;
              //System.out.println("3: now: " + tmp);
              // Byte 3
              inch = in.get();
              //System.out.println("3: Got: " + (int) inch);
              tmp |= (char) (inch & 0x7f);
              //System.out.println("3: now: " + tmp);
              out.put((char) tmp);
            } else if ((inch & 0xE0) == 0xC0) {
              if (in.remaining() < 2)
                return CoderResult.UNDERFLOW;

              //System.out.println("2: ");

              // Byte 1
              inch = in.get();
              tmp = (char) (inch & 0x1F);
              tmp <<= 6;
              // Byte 2
              inch = in.get();
              tmp |= (char) (inch & 0x7f);

              out.put((char) tmp);
            }
          }
        } else
          return CoderResult.OVERFLOW;
      }

      return CoderResult.UNDERFLOW;
    }
  }

  private static class Encoder extends CharsetEncoder {

    private Encoder(final Charset cs) {
      super(cs, 1.0f, 4.0f);
    }

    @Override
    protected CoderResult encodeLoop(final CharBuffer in, final ByteBuffer out) {
      while (in.hasRemaining()) {
        if (out.position() < out.limit()) {
          final char ch = in.get();
          if (ch < 0x80) {
            out.put((byte) ch);
          } else {
            final int left = out.capacity() - out.position();

            if (ch > 0xD800 && ch < 0xE000)
              return CoderResult.unmappableForLength(in.position());
            else if (ch > 0x800) {
              if (left < 3)
                return CoderResult.OVERFLOW;
              out.put((byte) (ch >> 12 | 0xE0));
              out.put((byte) (ch >> 6 & 0x3F | 0x80));
              out.put((byte) (ch >> 0 & 0x3F | 0x80));
            } else if (ch > 0x80) {
              if (left < 2)
                return CoderResult.OVERFLOW;
              out.put((byte) (ch >> 6 | 0xC0));
              out.put((byte) (ch >> 0 & 0x3F | 0x80));
            }
          }
        } else
          return CoderResult.OVERFLOW;
      }

      return CoderResult.UNDERFLOW;
    }

  }

  final public static Charset INSTANCE;

  static {
    INSTANCE = new UTF_8();
  }

  private UTF_8() {
    super("UTF-8", new String[] { "UTF-8" });
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
