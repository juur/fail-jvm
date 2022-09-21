package java.util;

import java.io.Serializable;

public final class UUID implements Serializable, Comparable<UUID> {

  private static final long serialVersionUID = 1L;
  
  /* msb */
  private static final long TIME_LOW_MASK = 0xFFFFFFFF00000000L;
  private static final long TIME_MID_MASK = 0x00000000FFFF0000L;
  private static final long VER_MASK      = 0x000000000000F000L;
  private static final long TIME_HI_MASK  = 0x0000000000000FFFL;

  /* lsb */
  private static final long VARIANT_MASK   = 0xC000000000000000L;
  private static final long CLOCK_SEQ_MASK = 0x3FFF000000000000L;
  private static final long NODE_MASK      = 0x0000FFFFFFFFFFFFL;

  public static UUID fromString(final String name) {
    if (name == null || name.length() != 36)
      throw new IllegalArgumentException();

    //System.out.println("name      = "+name);

    final String time_lowStr              = name.substring(0, 8);
    final String time_midStr              = name.substring(9, 13);
    final String time_high_and_versionStr = name.substring(14, 18);
    final String variant_and_sequenceStr  = name.substring(19, 23);
    final String nodeStr                  = name.substring(24, 36);

    //System.out.println("time_low  = "+time_lowStr);
    //System.out.println("time_mid  = "+time_midStr);
    //System.out.println("time_high = "+time_high_and_versionStr);
    //System.out.println("variant   = "+variant_and_sequenceStr);
    //System.out.println("node      = "+nodeStr);

    final long node                  = Long.parseLong(nodeStr.toString(), 16);
    final long variant_and_sequence  = Long.parseLong(variant_and_sequenceStr.toString(), 16);
    final long time_high_and_version = Long.parseLong(time_high_and_versionStr.toString(), 16);
    final long time_mid              = Long.parseLong(time_midStr.toString(), 16);
    final long time_low              = Long.parseLong(time_lowStr.toString(), 16);

    //System.out.println("time_low  = " + time_low);
    //System.out.println("time_mid  = " + time_mid);
    //System.out.println("time_high = " + time_high_and_version);
    //System.out.println("variant   = " + variant_and_sequence);
    //System.out.println("node      = " + node);

    long lsb = node & NODE_MASK;
    lsb += (variant_and_sequence & ((VARIANT_MASK + CLOCK_SEQ_MASK) >> 48)) << 48;

    long msb = time_high_and_version & TIME_HI_MASK + VER_MASK;
    msb += (time_mid & TIME_MID_MASK >> 16) << 16;
    msb += (time_low & TIME_LOW_MASK >> 32) << 32;

    return new UUID(msb, lsb);

  }

  public static void main(final String[] args) {
    fromString("59fc8c84-0af6-11eb-adc1-0242ac120002");
  }

  public static UUID randomUUID() {
    return null;
  }

  private final long msb;
  private final long lsb;
  private final long node;
  private final int  clockSeq;
  private final int  variant;
  private final long time;
  private final int  version;

  public UUID(final long mostSigBits, final long leastSigBits) {
    msb = mostSigBits;
    lsb = leastSigBits;

    version = (int) ((msb & VER_MASK) >> 12);
    variant = (int) ((lsb & VARIANT_MASK) >> 48 + 14);

    if (version == 1) {
      node = lsb & NODE_MASK;
      clockSeq = (int) ((lsb & CLOCK_SEQ_MASK) >> 48);

      long tmptime = (msb & TIME_HI_MASK) << 48;
      tmptime |= (msb & TIME_MID_MASK) << 16;
      tmptime |= (msb & TIME_LOW_MASK) >> 32;
      time = tmptime;
    } else {
      node = -1L;
      clockSeq = -1;
      time = -1L;
    }

  }

  public long clockSequence() {
    if ( version != 1 )
      throw new UnsupportedOperationException();
    return clockSeq;
  }

  @Override
  public int compareTo(final UUID o) {
    return 0;
  }

  public long getLeastSignificantBits() {
    return lsb;
  }

  public long getMostSignificantBits() {
    return msb;
  }

  public long node() {
    if ( version != 1 )
      throw new UnsupportedOperationException();
    return node;
  }

  public long timestamp() {
    if ( version != 1 )
      throw new UnsupportedOperationException();
    return time;
  }

  @Override
  public String toString() {
    return "00000000-0000-0000-0000-000000000000";
  }

  public int variant() {
    return variant;
  }

  public int version() {
    return version;
  }
}
