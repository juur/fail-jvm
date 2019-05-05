package spod.cs;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CoderResult;

public class CP1252 extends Charset {

  private static class Decoder extends CharsetDecoder {

    private Decoder(final Charset cs) {
      super(cs, 1.0f, 1.0f);
    }

    @Override
    protected CoderResult decodeLoop(final ByteBuffer in, final CharBuffer out) {
      while (in.hasRemaining()) {
        if (out.position() < out.capacity()) {
          final byte inb = in.get();
          char       rep = (char) inb;
          System.out.println("in=" + rep);
          switch (inb) {
            // case (byte)0x81: rep=0x0; break;
            case (byte) 0x82:
              rep = 0x201a;
              break;
            case (byte) 0x83:
              rep = 0x192;
              break;
            case (byte) 0x84:
              rep = 0x201e;
              break;
            case (byte) 0x85:
              rep = 0x2026;
              break;
            case (byte) 0x86:
              rep = 0x2020;
              break;
            case (byte) 0x87:
              rep = 0x2021;
              break;
            case (byte) 0x88:
              rep = 0x2c6;
              break;
            case (byte) 0x89:
              rep = 0x2030;
              break;
            case (byte) 0x8a:
              rep = 0x160;
              break;
            case (byte) 0x8b:
              rep = 0x2039;
              break;
            case (byte) 0x8c:
              rep = 0x152;
              break;
            case (byte) 0x8d:
              rep = 0x0;
              break;
            case (byte) 0x8e:
              rep = 0x17d;
              break;
            // case (byte)0x8f: rep=0x0; break;
            // case (byte)0x90: rep=0x0; break;
            case (byte) 0x91:
              rep = 0x2018;
              break;
            case (byte) 0x92:
              rep = 0x2019;
              break;
            case (byte) 0x93:
              rep = 0x201c;
              break;
            case (byte) 0x94:
              rep = 0x201d;
              break;
            case (byte) 0x95:
              rep = 0x2022;
              break;
            case (byte) 0x96:
              rep = 0x2013;
              break;
            case (byte) 0x97:
              rep = 0x2014;
              break;
            case (byte) 0x98:
              rep = 0x2dc;
              break;
            case (byte) 0x99:
              rep = 0x2122;
              break;
            case (byte) 0x9a:
              rep = 0x161;
              break;
            case (byte) 0x9b:
              rep = 0x203a;
              break;
            case (byte) 0x9c:
              rep = 0x153;
              break;
            // case (byte)0x9d: rep=0x0; break;
            case (byte) 0x9e:
              rep = 0x17e;
              break;
            case (byte) 0x9f:
              rep = 0x178;
              break;
          }
          out.put(rep);
        } else
          return CoderResult.OVERFLOW;
      }
      return CoderResult.UNDERFLOW;
    }
  }

  private static class Encoder extends CharsetEncoder {

    private Encoder(final Charset cs) {
      super(cs, 1.0f, 1.0f);
      System.out.println("CP1252: created Encoder");
    }

    @Override
    protected CoderResult encodeLoop(final CharBuffer in, final ByteBuffer out) {
      System.out.println("encodeLoop: called");
      System.out.println(
        "encodeLoop: in limit=" + in.limit() + ", remaining=" + in.remaining() + ", pos=" + in.position() + " cap=" + in.capacity());
      while (in.hasRemaining()) {
        System.out.println("encodeLoop: hasRemaining");
        if (out.position() < out.capacity()) {
          final char inch = in.get();
          System.out.println("encodeLoop: in=" + inch);
          if (inch < 0x80 || inch > 0xBF)
            out.put((byte) (inch & 0xFF));
          else if (inch < 0x100) {
            byte rep;
            switch (inch) {
              case 0x20ac:
                rep = (byte) 0x80;
                break;
              case 0x201a:
                rep = (byte) 0x82;
                break;
              case 0x192:
                rep = (byte) 0x83;
                break;
              case 0x201e:
                rep = (byte) 0x84;
                break;
              case 0x2026:
                rep = (byte) 0x85;
                break;
              case 0x2020:
                rep = (byte) 0x86;
                break;
              case 0x2021:
                rep = (byte) 0x87;
                break;
              case 0x2c6:
                rep = (byte) 0x88;
                break;
              case 0x2030:
                rep = (byte) 0x89;
                break;
              case 0x160:
                rep = (byte) 0x8a;
                break;
              case 0x2039:
                rep = (byte) 0x8b;
                break;
              case 0x152:
                rep = (byte) 0x8c;
                break;
              case 0x17d:
                rep = (byte) 0x8e;
                break;
              case 0x2018:
                rep = (byte) 0x91;
                break;
              case 0x2019:
                rep = (byte) 0x92;
                break;
              case 0x201c:
                rep = (byte) 0x93;
                break;
              case 0x201d:
                rep = (byte) 0x94;
                break;
              case 0x2022:
                rep = (byte) 0x95;
                break;
              case 0x2013:
                rep = (byte) 0x96;
                break;
              case 0x2014:
                rep = (byte) 0x97;
                break;
              case 0x2dc:
                rep = (byte) 0x98;
                break;
              case 0x2122:
                rep = (byte) 0x99;
                break;
              case 0x161:
                rep = (byte) 0x9a;
                break;
              case 0x203a:
                rep = (byte) 0x9b;
                break;
              case 0x153:
                rep = (byte) 0x9c;
                break;
              case 0x17e:
                rep = (byte) 0x9e;
                break;
              case 0x178:
                rep = (byte) 0x9f;
                break;
              default:
                if (inch < 0x100)
                  rep = (byte) inch;
                else {
                  out.put(replacement());
                  continue;
                }
            }
            out.put(rep);
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
    INSTANCE = new CP1252();
  }

  private CP1252() {
    super("CP1252", new String[] { "CP1252" });
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
