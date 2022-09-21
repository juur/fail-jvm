package java.nio.charset;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;

public abstract class CharsetEncoder {

  private final float       averageBytesPerChar;
  private final Charset     cs;
  private CodingErrorAction malformedInputAction;
  private final float       maxBytesPerChar;
  private byte[]            replacement;
  private CodingErrorAction unmappableCharacter;

  protected CharsetEncoder(final Charset newCharset, final float newAverageBytesPerChar, final float newMaxBytesPerChar) {
    this.cs = newCharset;
    this.averageBytesPerChar = newAverageBytesPerChar;
    this.maxBytesPerChar = newMaxBytesPerChar;
    replacement = new byte[] { (byte) '?' };
  }

  protected CharsetEncoder(final Charset newCharset, final float newAverageBytesPerChar, final float newMaxBytesPerChar, final byte[] newReplacement) {
    this(newCharset, newAverageBytesPerChar, newMaxBytesPerChar);
    this.replacement = newReplacement;
  }

  public float averageBytesPerChar() {
    return averageBytesPerChar;
  }

  public boolean canEncode(final char c) {
    return true;
  }

  public boolean canEncode(final CharSequence newCharset) {
    for (int i = 0; i < newCharset.length(); i++)
      if (!canEncode(newCharset.charAt(i)))
        return false;
    return true;
  }

  public Charset charset() {
    return cs;
  }

  public ByteBuffer encode(final CharBuffer in) {
    reset();
    final ByteBuffer  out = ByteBuffer.allocate((int) (in.length() * maxBytesPerChar()));
    final CoderResult res = encode(in, out, true);
    if (res.isError())
      return null;
    out.limit(out.position());
    out.rewind();
    return out;
  }

  public CoderResult encode(final CharBuffer in, final ByteBuffer out, final boolean endOfInput) {
    boolean     running = true;
    CoderResult cr      = null;

    while (running) {
      cr = encodeLoop(in, out);
      if (cr.isUnderflow() && in.hasRemaining())
        continue;
      else if (cr.isOverflow() && out.hasRemaining())
        continue;
      else if (cr.isUnmappable())
        out.put(replacement());
      else if (cr.isMalformed() || cr.isError())
        running = false;
      else
        running = false;
    }

    return cr;
  }

  public CoderResult flush() {
    return implFlush();
  }

  public boolean isLegalReplacement(final byte[] reply) {
    return true;
  }

  public CodingErrorAction malformedInputAction() {
    return malformedInputAction;
  }

  public float maxBytesPerChar() {
    return maxBytesPerChar;
  }

  public CharsetEncoder onMalformedInput(final CodingErrorAction newAction) {
    malformedInputAction = newAction;
    implMalformedInput(newAction);
    return this;
  }

  public CharsetEncoder onUnmappableCharacter(final CodingErrorAction newAction) {
    unmappableCharacter = newAction;
    implUnmappableCharacter(newAction);
    return this;
  }

  public byte[] replacement() {
    return replacement;
  }

  public byte[] replacement(final byte[] newReplacement) {
    replacement = newReplacement;
    return newReplacement;
  }

  public CharsetEncoder reset() {
    implReset();
    return this;
  }

  public CodingErrorAction unmappableCharacterAction() {
    return unmappableCharacter;
  }

  protected abstract CoderResult encodeLoop(CharBuffer in, ByteBuffer out);

  protected CoderResult implFlush() {
    return CoderResult.UNDERFLOW;
  }

  protected void implMalformedInput(final CodingErrorAction newAction) {}

  protected void implReplaceWith(final byte[] newReplacement) {}

  protected void implReset() {}

  protected void implUnmappableCharacter(final CodingErrorAction newAction) {}
}
