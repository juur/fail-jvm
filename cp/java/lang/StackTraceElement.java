package java.lang;

public class StackTraceElement {

  final private String  declaringClass;
  final private String  fileName;
  final private boolean isNative;
  final private int     lineNumber;
  final private String  methodName;

  public StackTraceElement(final String declaringClass, final String methodName, final String fileName, final int lineNumber) {
    this.declaringClass = declaringClass;
    this.methodName = methodName;
    this.fileName = fileName;
    this.lineNumber = lineNumber;
    isNative = false;
  }

  public String getFileName() {
    return fileName;
  }

  public int getLineNumber() {
    return lineNumber;
  }

  public String getMethodName() {
    return declaringClass + "." + methodName;
  }

  public boolean isNative() {
    return isNative;
  }
}
