package java.nio.charset;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;

public abstract class CharsetDecoder {

  private final float       averageCharsPerByte;
  private final Charset     cs;
  private CodingErrorAction malformedInputAction;
  private final float       maxCharsPerByte;
  private String            replacement;
  private CodingErrorAction unmappableCharacterAction;

  protected CharsetDecoder(final Charset cs, final float averageCharsPerByte, final float maxCharsPerByte) {
    this.cs = cs;
    this.averageCharsPerByte = averageCharsPerByte;
    this.maxCharsPerByte = maxCharsPerByte;
    replacement = "\uFFFD";
  }

  public float averageCharsPerByte() {
    return averageCharsPerByte;
  }

  public Charset charset() {
    return cs;
  }

  public final CharBuffer decode(final ByteBuffer in) throws CharacterCodingException {
    final CharBuffer out = CharBuffer.allocate((int) (maxCharsPerByte * in.limit()));
    reset();
    final CoderResult cr = decode(in, out, true);
    if (cr.isError())
      cr.throwException();
    out.limit(out.position());
    out.rewind();
    return out;
  }

  public final CoderResult decode(final ByteBuffer in, final CharBuffer out, final boolean endOfInput) {
    boolean     running = true;
    CoderResult cr      = null;

    while (running) {
      cr = decodeLoop(in, out);
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

  public Charset detectedCharset() {
    throw new UnsupportedOperationException();
  }

  public final CoderResult flush(final CharBuffer out) {
    return implFlush();
  }

  public boolean isAutoDetecting() {
    return false;
  }

  public boolean isCharsetDetected() {
    throw new UnsupportedOperationException();
  }

  public CodingErrorAction malformedInputAction() {
    return malformedInputAction;
  }

  public float maxCharsPerByte() {
    return maxCharsPerByte;
  }

  public final CharsetDecoder onMalformedInput(final CodingErrorAction newAction) {
    malformedInputAction = newAction;
    implOnmalformedInput(newAction);
    return this;
  }

  public final CharsetDecoder onUnmappableCharacter(final CodingErrorAction newAction) {
    unmappableCharacterAction = newAction;
    implOnUnmappableCharacter(newAction);
    return this;
  }

  public String replacement() {
    return replacement;
  }

  public final CharsetDecoder replaceWith(final String newReplacement) {
    replacement = newReplacement;
    implReplaceWith(newReplacement);
    return this;
  }

  public final CharsetDecoder reset() {
    implReset();
    return this;
  }

  public CodingErrorAction unmappableCharacterAction() {
    return unmappableCharacterAction;
  }

  protected abstract CoderResult decodeLoop(ByteBuffer in, CharBuffer out);

  protected CoderResult implFlush() {
    return CoderResult.UNDERFLOW;
  }

  protected void implOnmalformedInput(final CodingErrorAction newAction) {}

  protected void implOnUnmappableCharacter(final CodingErrorAction newAction) {}

  protected void implReplaceWith(final String newReplacement) {}

  protected void implReset() {}
}
