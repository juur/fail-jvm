package java.util.zip;

import java.util.Arrays;

public class ZipEntry implements Cloneable {

  public static final int  CENATT   = 36;
  public static final int  CENATX   = 38;
  public static final int  CENCOM   = 32;
  public static final int  CENCRC   = 16;
  public static final int  CENDSK   = 34;
  public static final int  CENEXT   = 30;
  public static final int  CENFLG   = 8;
  public static final int  CENHDR   = 46;
  public static final int  CENHOW   = 10;
  public static final int  CENLEN   = 24;
  public static final int  CENNAM   = 28;
  public static final int  CENOFF   = 42;
  public static final long CENSIG   = 33639248L;
  public static final int  CENSIZ   = 20;
  public static final int  CENTIM   = 12;
  public static final int  CENVEM   = 4;
  public static final int  CENVER   = 6;
  public static final int  DEFLATED = 8;
  public static final int  ENDCOM   = 20;
  public static final int  ENDHDR   = 22;
  public static final int  ENDOFF   = 16;
  public static final long ENDSIG   = 101010256L;
  public static final int  ENDSIZ   = 12;
  public static final int  ENDSUB   = 8;
  public static final int  ENDTOT   = 10;
  public static final int  EXTCRC   = 4;
  public static final int  EXTHDR   = 16;
  public static final int  EXTLEN   = 12;
  public static final long EXTSIG   = 134695760L;
  public static final int  EXTSIZ   = 8;
  public static final int  LOCCRC   = 14;
  public static final int  LOCEXT   = 28;
  public static final int  LOCFLG   = 6;
  public static final int  LOCHDR   = 30;
  public static final int  LOCHOW   = 8;
  public static final int  LOCLEN   = 22;
  public static final int  LOCNAM   = 26;
  public static final long LOCSIG   = 67324752L;
  public static final int  LOCSIZ   = 18;
  public static final int  LOCTIM   = 10;
  public static final int  LOCVER   = 4;
  public static final int  STORED   = 0;

  final private String name;
  private long             time     = -1;
  private long             size     = -1;
  private long             compSize = -1;
  private long             crc      = -1;
  private int              method   = -1;
  private byte[]           extra    = null;
  private String       comment;

  public ZipEntry(final String nName) {
    name = nName;
  }

  public ZipEntry(final ZipEntry e) {
    name = e.getName();
  }

  @Override
  public boolean equals(final Object obj) {
    if (this == obj) {
      return true;
    }
    if (!(obj instanceof ZipEntry)) {
      return false;
    }
    final ZipEntry other = (ZipEntry) obj;
    if (comment == null) {
      if (other.comment != null) {
        return false;
      }
    } else if (!comment.equals(other.comment)) {
      return false;
    }
    if (compSize != other.compSize) {
      return false;
    }
    if (crc != other.crc) {
      return false;
    }
    if (!Arrays.equals(extra, other.extra)) {
      return false;
    }
    if (method != other.method) {
      return false;
    }
    if (name == null) {
      if (other.name != null) {
        return false;
      }
    } else if (!name.equals(other.name)) {
      return false;
    }
    if (size != other.size) {
      return false;
    }
    if (time != other.time) {
      return false;
    }
    return true;
  }

  public String getComment() {
    return comment;
  }

  public long getCompressedSize() {
    return compSize;
  }

  public long getCrc() {
    return crc;
  }

  public byte[] getExtra() {
    return extra == null ? null : extra.clone();
  }

  public int getMethod() {
    return method;
  }

  public String getName() {
    return name;
  }

  public long getSize() {
    return size;
  }

  public long getTime() {
    return time;
  }

  @Override
  public int hashCode() {
    final int prime  = 31;
    int       result = 1;
    result = prime * result + (comment == null ? 0 : comment.hashCode());
    result = prime * result + (int) (compSize ^ compSize >>> 32);
    result = prime * result + (int) (crc ^ crc >>> 32);
    result = prime * result + Arrays.hashCode(extra);
    result = prime * result + method;
    result = prime * result + (name == null ? 0 : name.hashCode());
    result = prime * result + (int) (size ^ size >>> 32);
    result = prime * result + (int) (time ^ time >>> 32);
    return result;
  }

  public boolean isDirectory() {
    return name.endsWith("/");
  }

  public void setComment(final String nComment) {
    comment = nComment;
  }

  public void setCompressedSize(final long nCompressedSize) {
    compSize = nCompressedSize;
  }

  public void setCrc(final long nCrc) {
    if (nCrc < 0 || nCrc > 0xffffffff)
      throw new IllegalArgumentException();

    crc = nCrc;
  }

  public void setExtra(final byte[] nExtra) {
    extra = nExtra.clone();
  }

  public void setMethod(final int nMethod) {
    switch (nMethod) {
      case STORED:
      case DEFLATED:
        break;
      default:
        throw new IllegalArgumentException();
    }

    method = nMethod;
  }

  public void setSize(final long nSize) {
    size = nSize;
  }

  public void setTime(final long nTime) {
    time = nTime;
  }

  @Override
  public String toString() {
    return getName();
  }
}
