package java.lang;


public final class Compiler {

  native public static Object command(Object any);

  native public static boolean compileClass(Class<?> clazz);

  native public static boolean compileClasses(String string);

  native public static void disable();

  native public static void enable();
}
