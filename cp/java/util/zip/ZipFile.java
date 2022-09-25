package java.util.zip;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Enumeration;

public class ZipFile {

  public static final int  CENATT      = 36;

  public static final int  CENATX      = 38;
  public static final int  CENCOM      = 32;
  public static final int  CENCRC      = 16;
  public static final int  CENDSK      = 34;
  public static final int  CENEXT      = 30;
  public static final int  CENFLG      = 8;
  public static final int  CENHDR      = 46;
  public static final int  CENHOW      = 10;
  public static final int  CENLEN      = 24;
  public static final int  CENNAM      = 28;
  public static final int  CENOFF      = 42;
  public static final long CENSIG      = 33639248L; // 0x2014B50
  public static final int  CENSIZ      = 20;
  public static final int  CENTIM      = 12;
  public static final int  CENVEM      = 4;
  public static final int  CENVER      = 6;
  public static final int  ENDCOM      = 20;
  public static final int  ENDHDR      = 22;
  public static final int  ENDOFF      = 16;
  public static final long ENDSIG      = 101010256L; // 0x6054B50
  public static final int  ENDSIZ      = 12;
  public static final int  ENDSUB      = 8;
  public static final int  ENDTOT      = 10;
  public static final int  EXTCRC      = 4;
  public static final int  EXTHDR      = 16;
  public static final int  EXTLEN      = 12;
  public static final long EXTSIG      = 134695760L;
  public static final int  EXTSIZ      = 8;
  public static final int  LOCCRC      = 14;
  public static final int  LOCEXT      = 28;
  public static final int  LOCFLG      = 6;
  public static final int  LOCHDR      = 30;
  public static final int  LOCHOW      = 8;
  public static final int  LOCLEN      = 22;
  public static final int  LOCNAM      = 26;
  public static final long LOCSIG      = 67324752L; // 0x04034b50 or "PK\3\4"
  public static final int  LOCSIZ      = 18;
  public static final int  LOCTIM      = 10;
  public static final int  LOCVER      = 4;
  public static final int  OPEN_DELETE = 4;
  public static final int  OPEN_READ   = 1;

  private final InputStream is;
  private final int  mode;
  private final File file;

  public ZipFile(final File nFile) throws IOException {
    this(nFile, OPEN_READ);
  }

  public ZipFile(final File nFile, final int nMode) throws IOException {
    file = nFile;
    mode = nMode;
    is = new FileInputStream(file);

    parseFile();
  }

  public ZipFile(final String name) throws IOException {
    this(new File(name), OPEN_READ);
  }

  public void close() throws IOException {
    is.close();
  }

  public Enumeration<? extends ZipEntry> entries() {
    return null;
  }

  public ZipEntry getEntry(final String name) {
    return null;
  }

  public InputStream getInputStream(final ZipEntry entry) {
    return is;
  }

  public String getName() {
    return file.toString();
  }

  public int size() {
    return 0;
  }

  private long bytesToLong(final byte[] ba) {
    long      value = 0;
    int  off   = 0;

    for (final byte b : ba)
      value += (b & 0xffL) << 8 * off++;

    return value;
  }

  private void parseFile() throws IOException {
    final byte[] b4 = new byte [4];

    is.reset();
    is.read(b4);

    System.out.println("Read:");
    System.out.println(Long.toString(bytesToLong(b4)));
  }
}
