package java.lang;

public class StackTraceElement {

  final private String  declaringClass;
  final private String  fileName;
  final private boolean isNative;
  final private int     lineNumber;
  final private String  methodName;

  public StackTraceElement(final String newDeclaringClass, final String newMethodName, final String newFileName, final int newLineNumber) {
    this.declaringClass = newDeclaringClass;
    this.methodName = newMethodName;
    this.fileName = newFileName;
    this.lineNumber = newLineNumber;
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
