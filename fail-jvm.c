#define _XOPEN_SOURCE 700
#undef NDEBUG

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define ERR_AT  __FILE__ ":" TOSTRING(__LINE__)
#define STRING_TOO_LONG() warnx("snprintf truncation:" ERR_AT)

#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <stdint.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <math.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* macros etc */
#define COLOUR
#ifdef COLOUR
#define ANSI_INSTR  "\x1b[1;32m"
#define ANSI_BRED   "\x1b[1;31m"
#define ANSI_BYEL   "\x1b[1;33m"
#define ANSI_BMAG   "\x1b[1;35m"
#define ANSI_RESET  "\x1b[0m"
#else
#define ANSI_INSTR  ""
#define ANSI_BRED   ""
#define ANSI_BYEL   ""
#define ANSI_BMAG   ""
#define ANSI_RESET  ""
#endif

/* access flags */

#define ACC_PUBLIC      ((unsigned)0x0001)
#define ACC_PRIVATE     ((unsigned)0x0002)
#define ACC_PROTECTED   ((unsigned)0x0004)
#define ACC_STATIC      ((unsigned)0x0008)
#define ACC_FINAL       ((unsigned)0x0010)
#define ACC_SUPER       ((unsigned)0x0020)
#define ACC_SYNCH       ((unsigned)0x0020)
#define ACC_VOLATILE    ((unsigned)0x0040)
#define ACC_BRIDGE      ((unsigned)0x0040)
#define ACC_TRANSIENT   ((unsigned)0x0080)
#define ACC_NATIVE      ((unsigned)0x0100)
#define ACC_INTERFACE   ((unsigned)0x0200)
#define ACC_ABSTRACT    ((unsigned)0x0400)
#define ACC_STRICT      ((unsigned)0x0800)
#define ACC_SYNTHETIC   ((unsigned)0x1000)
#define ACC_ANNOTATION  ((unsigned)0x2000)
#define ACC_ENUM        ((unsigned)0x4000)

#define TYPE_BYTE       ((char)'B')
#define TYPE_CHAR       ((char)'C')
#define TYPE_DOUBLE     ((char)'D')
#define TYPE_FLOAT      ((char)'F')
#define TYPE_INT        ((char)'I')
#define TYPE_LONG       ((char)'J')
#define TYPE_OREF       ((char)'L')
#define TYPE_AREF       ((char)'[')
#define TYPE_SHORT      ((char)'S')
#define TYPE_BOOL       ((char)'Z')
#define TYPE_VOID       ((char)'V')
#define TYPE_NULL       ((char)'N')
#define TYPE_RETADDR    ((char)'\r')

#define T_BOOLEAN   ((int)4)
#define T_CHAR      ((int)5)
#define T_FLOAT     ((int)6)
#define T_DOUBLE    ((int)7)
#define T_BYTE      ((int)8)
#define T_SHORT     ((int)9)
#define T_INT       ((int)10)
#define T_LONG      ((int)11)
#define T_MAX       ((int)12)

static const uint8_t atypemap[T_MAX] = {
    /*  0, 0, 0, 0,
        T_BOOLEAN,
        T_CHAR,
        T_FLOAT,
        T_DOUBLE,
        T_BYTE,
        T_SHORT,
        T_INT,
        T_LONG*/
    0, 0, 0, 0,
    TYPE_BOOL,
    TYPE_CHAR,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_BYTE,
    TYPE_INT,
    TYPE_LONG
};

/*
 * Constant Pool
 */

/* value of .tag */

#define CONSTANT_Class              ((uint8_t)7)
#define CONSTANT_Double             ((uint8_t)6)
#define CONSTANT_Fieldref           ((uint8_t)9)
#define CONSTANT_Float              ((uint8_t)4)
#define CONSTANT_Integer            ((uint8_t)3)
#define CONSTANT_InterfaceMethodref ((uint8_t)11)
#define CONSTANT_InvokeDynamic      ((uint8_t)18)
#define CONSTANT_Long               ((uint8_t)5)
#define CONSTANT_MethodHandle       ((uint8_t)15)
#define CONSTANT_MethodType         ((uint8_t)16)
#define CONSTANT_Methodref          ((uint8_t)10)
#define CONSTANT_NameAndType        ((uint8_t)12)
#define CONSTANT_String             ((uint8_t)8)
#define CONSTANT_Utf8               ((uint8_t)1)

struct _cp_info;

typedef struct _cp_Utf8_info {
    uint16_t     length;
    uint16_t     utf16len;
    uint16_t    *utf16;
    char        *ascii;
    uint8_t     *bytes;
} cp_Utf8_info;

typedef struct _cp_MethodType_info {
    uint16_t    descriptor_index;
} cp_MethodType_info;

typedef struct _cp_MethodHandle_info {
    uint8_t     reference_kind;
    uint16_t    reference_index;
} cp_MethodHandle_info;

typedef struct _cp_NameAndType_info {
    uint16_t    name_index;
    uint16_t    descriptor_index;

    struct _cp_info *name;
    struct _cp_info *descriptor;

    //const char      *original_name;
    //const char      *original_descriptor;
} cp_NameAndType_info;

typedef struct _cp_InvokeDynamic_info {
    uint16_t    bootstrap_method_attr_index;
    uint16_t    name_and_type_index;
} cp_InvokeDynamic_info;

typedef struct _cp_Long_info {
    int64_t c_long;
} cp_Long_info;

typedef struct _cp_Integer_info {
    int32_t c_int;
} cp_Integer_info;

typedef struct _cp_Double_info {
    double      c_double;
} cp_Double_info;

typedef struct _cp_Float_info {
    float       c_float;
} cp_Float_info;

typedef struct _cp_String_info {
    uint16_t    string_index;

    /* char[] */
    struct _Object *intern;
    struct _cp_info      *string;
} cp_String_info;

typedef struct _cp_Class_info {
    uint16_t    name_index;

    struct _cp_info *name;
    //const char      *original_name;
} cp_Class_info;


/* Actually covers both Fieldref_info and Methodref_info */
typedef struct _cp_Fieldref_info {
    uint16_t    class_index;
    uint16_t    name_and_type_index;

    struct _cp_info *class;
    struct _cp_info *nametype;
} cp_Fieldref_info;

typedef struct _cp_info {
    uint8_t tag;

    union {
        cp_Utf8_info            utf8;
        cp_MethodHandle_info    mh;
        cp_MethodType_info      mt;
        cp_NameAndType_info     nt;
        cp_InvokeDynamic_info   id;
        cp_Long_info            tlong;
        cp_Integer_info         tint;
        cp_Float_info           tfloat;
        cp_Double_info          tdouble;
        cp_String_info          str;
        cp_Class_info           class;
        cp_Fieldref_info        fr;
    } d;
} cp_info;

/*
 * attribute_info
 */

enum std_attr_e {
    ATTR_INVALID=0,
    ATTR_CONSTANT_VALUE,
    ATTR_CODE,
    ATTR_STACKMAPTABLE,
    ATTR_EXCEPTIONS,
    ATTR_BOOTSTRAPMETHODS,
    ATTR_INNERCLASSES,
    ATTR_SOURCEFILE,
    ATTR_LINENUMBERTABLE
};

struct predefinedAttribute {
    enum std_attr_e  attr;
    const char      *name;
    size_t           size;
};

struct _ClassFile;

typedef struct _line_number {
    uint16_t    start_pc;
    uint16_t    end_pc;
    uint16_t    line_number;
} line_number_t;

typedef struct _LineNumberTable_attribute {
    uint16_t          line_number_table_length;
    line_number_t    *line_number_table;
} LineNumberTable_attribute;

typedef struct _inner_class_def {
    uint16_t    inner_class_info_index;
    uint16_t    outer_class_info_index;
    uint16_t    inner_name_index;
    uint16_t    inner_class_access_flags;

    cp_info         *inner_ci;
    cp_info         *outer_ci;
    struct _ClassFile   *inner_class;
    cp_info         *inner_name;
} inner_class_def;

typedef struct _InnerClasses_attribute {
    uint16_t          number_of_classes;
    inner_class_def **classes;
} InnerClasses_attribute;

typedef struct _SourceFile_attribute {
    uint16_t         sourcefile_index;
    cp_info *name;
} SourceFile_attribute;

typedef struct _Exceptions_attribute {
    uint16_t          number_of_exceptions;
    cp_info **exception_index_table;
} Exceptions_attribute;

typedef union _OperandValue {
    struct _Object  *vref;
    int64_t          vlong;
    int32_t          vint;
    int8_t           vbyte;
    double           vdouble;
    float            vfloat;
    int16_t          vshort;
    bool             vbool;
    uint16_t         vchar;
    uint32_t         vretaddr;
} OperandValue;

typedef struct _Operand {
    uint8_t         type;
    OperandValue    val;
    bool            freed;
} Operand;

typedef struct _ConstantValue_attribute {
    uint16_t         constantvalue_index;
    cp_info *entry;
    Operand          op;
} ConstantValue_attribute;

typedef struct _bootstrap_method {
    uint16_t         bootstrap_method_ref;
    uint16_t         num_bootstrap_arguments;
    uint16_t    *bootstrap_arguments;
} bootstrap_method;

typedef struct _BootstrapMethods_attribute {
    uint16_t                  num_bootstrap_methods;
    bootstrap_method    **bootstrap_methods;
} BootstrapMethods_attribute;

typedef struct _exception {
    uint16_t start_pc;
    uint16_t end_pc;
    uint16_t handler_pc;
    uint16_t catch_type_idx;

    cp_info *catch_type;
} exception;

struct _attribute_info;

typedef struct _Code_attribute {
    uint16_t          max_stack;
    uint16_t          max_locals;
    uint32_t          code_length;
    uint8_t  *code;
    uint16_t          exception_table_length;
    exception   **exception_table;
    uint16_t          attributes_count;

    struct _method_info      *method;
    struct _attribute_info  **attributes;
    LineNumberTable_attribute    *lineno;
} Code_attribute;

typedef struct _Object_variable_info {
    uint8_t     tag;
    uint16_t    cpool_index;
} Object_variable_info;

typedef struct _Uninitialized_variable_info {
    uint8_t     tag;
    uint16_t    offset;
} Uninitialized_variable_info;

typedef struct _Basic_variable_info {
    uint8_t     tag;
} Basic_variable_info;

union verification_type_info {
    Object_variable_info        ovi;
    Basic_variable_info         bvi;
    Uninitialized_variable_info uvi;
};

typedef struct _chop_frame {
    uint16_t offset_delta;
} chop_frame;

typedef struct _append_frame {
    uint16_t    offset_delta;
    uint8_t     local_num;

    union verification_type_info **locals;
} append_frame;

typedef struct _full_frame {
    uint16_t    offset_delta;
    uint16_t    number_of_locals;
    uint16_t    number_of_stack_items;

    union verification_type_info **locals;
    union verification_type_info **stack;
} full_frame;

typedef struct _same_frame_extended {
    uint16_t    offset_delta;
} same_frame_extended;

typedef struct _samelocalsi_frame {
    union verification_type_info stack;
} samelocalsi_frame;

enum frametype_en {
    FT_NULL = 0,
    FT_SAME,
    FT_SAME_LOCALS_1_STACK_ITEM,
    FT_SAME_LOCALS_1_STACK_ITEM_EXTENDED,
    FT_CHOP,
    FT_SAME_FRAME_EXTENDED,
    FT_APPEND,
    FT_FULL_FRAME
};

typedef struct _stack_map_frame {
    uint8_t tag;
    enum    frametype_en type;

    union {
        chop_frame          cf;     // FT_CHOP
        append_frame        af;     // FT_APPEND
        full_frame          ff;     // FT_FULL_FRAME
        samelocalsi_frame   slsi;   // FT_SAME_LOCALS_1_STACK_ITEM
        same_frame_extended sfe;    // FT_SAME_FRAME_EXTENDED
    } i;
} stack_map_frame;

#define ITEM_Top                ((int)0)
#define ITEM_Integer            ((int)1)
#define ITEM_Float              ((int)2)
#define ITEM_Double             ((int)3)
#define ITEM_Long               ((int)4)
#define ITEM_Null               ((int)5)
#define ITEM_UninitialisedThis  ((int)6)
#define ITEM_Object             ((int)7)
#define ITEM_Uninitialised      ((int)8)

typedef struct _StackMapTable_attribute {
    uint16_t          number_of_entries;

    stack_map_frame **entries;
} StackMapTable_attribute;

typedef struct _attribute_info {
    uint16_t         attribute_name_index;
    uint32_t         attribute_length;

    union {
        Code_attribute          *code;
        Exceptions_attribute        *exception;
        StackMapTable_attribute *stackmap;
        ConstantValue_attribute *constant;
        InnerClasses_attribute  *inner;
        SourceFile_attribute        *srcfile;
        LineNumberTable_attribute   *lineno;
        void                        *data;
    } d;

    cp_info *name;

    /* must be last for unknown attributes */
    uint8_t  info[];
} attribute_info;

/* descriptors */

typedef struct _des_Field {
    uint8_t  baseType;
    uint8_t  array;

    const char  *className;
    const char  *orig_className;
} des_Field;

typedef struct _des_Method {
    des_Field     ret;
    uint16_t      num_params;

    des_Field   **params;
} des_Method;

/* field_info */

struct _Operand;

typedef struct _field_info {
    uint16_t        access_flags;
    uint16_t        name_index;
    uint16_t        descriptor_index;
    uint16_t        attributes_count;

    attribute_info  **attributes;

    des_Field        desc;

    cp_info *name;
    cp_info *descriptor;
    struct _Operand *static_operand;
    struct _ClassFile       *class;
} field_info;

/* method_info */

struct _Thread;
struct _Object;

typedef struct _Operand *(*intStaticMethod)(struct _Thread *,
        struct _ClassFile *);
typedef struct _Operand *(*intMethod)(struct _Thread *,
        struct _ClassFile *,
        struct _Object *o);

typedef struct _method_info {
    uint16_t        access_flags;
    uint16_t        name_index;
    uint16_t        descriptor_index;
    uint16_t        attributes_count;

    des_Method       desc;

    struct _ClassFile    *class;
    attribute_info  **attributes;
    cp_info          *name;
    //const char       *original_name;
    cp_info          *descriptor;
    //const char       *original_descriptor;
    union {
        intStaticMethod mstatic;
        intMethod       mvirtual;
    } nativeMethod;
    Exceptions_attribute    *exceptions;
    Code_attribute      *code;
} method_info;

/* ClassFile */

struct _Object;

typedef struct _ClassFile {
    uint32_t      magic;
    uint16_t      minor_version;
    uint16_t      major_version;
    uint16_t      constant_pool_count;
    cp_info     **constant_pool;
    uint16_t      access_flags;
    uint16_t      this_class_idx;
    uint16_t      super_class_idx;
    uint16_t      interfaces_count;
    uint16_t     *interfaces;
    cp_info     **interface_class;
    uint16_t      fields_count;
    field_info  **fields;
    uint16_t      methods_count;
    method_info **methods;
    uint16_t      attributes_count;
    attribute_info **attributes;

    cp_info      *this_class;
    cp_info      *super_class;
    cp_info      *srcfile;
    const char   *super_name;
    //const char   *canonical_name;
    //const char   *original_name;
    bool          isInit;
    int32_t       heaploc;
    bool          isLinked;
    bool          isInnerClass;

    struct _Object   *class_object;

    pthread_rwlock_t      rwlock;
} ClassFile;

/* Object */

typedef struct _Field {
    Operand             *op;
    field_info  *field_info;
} Field;

#define OBJ_CLASS   ((uint8_t)1)
#define OBJ_ARRAY   ((uint8_t)2)

typedef struct _Object {
    uint8_t  type;
    uint32_t lock;
    int32_t  heaploc;

    pthread_mutex_t  monitor;
    const ClassFile *class;
    struct _Object  *class_obj;

    /* place other common to both array & object things above here */

    /* things not common in this union */
    union {
        struct {
            uint16_t  num_fields;
            Field    *fields;
            void     *native_data;
            void     (*native_free)(void *);
        } obj;
        struct {
            uint8_t           type;
            uint8_t           dimensions;
            uint32_t          len;
            int32_t          *sizes;
            struct _Operand **data;
        } array;
    } data;
    char *className;
} Object;

static uint16_t framecounter = 0;

typedef struct _Frame {
    uint16_t      frameNum;
    uint16_t      sp;
    uint16_t      num_local;
    uint16_t      num_stack;
    uint32_t      pc;
    Operand      *ret;
    Operand     **local;

    ClassFile          *class;
    const method_info  *mi;
    Operand           **stack;
} Frame;

typedef enum _th_state_en {
    TH_NEW,
    TH_RUNNING,
    TH_FINISHED,
    TH_DEAD
} th_state_en;

#define MAX_THREADS ((unsigned)16)
#define MAX_STACK   ((unsigned)512)
#define MAX_HEAP    ((unsigned)32768)
#define DEF_HEAP    ((unsigned)128)
#define MAX_METHOD  ((unsigned)128)

typedef struct _Thread {
    Frame           *stack[MAX_STACK];
    Object          *thread;
    uint32_t         pc;
    uint32_t         frame_free;
    pthread_t        pthread;
    uint16_t         thread_id;
    th_state_en      state;

    pthread_rwlock_t     rwlock;
    struct _JVM *jvm;
    const method_info   *cur_method;
} Thread;

typedef struct _JVM {
    pthread_rwlock_t    rwlock;

    bool       VM;
    bool       ClassInit;
    bool       primUp;
    uint16_t   maxHeap;
    Thread    *cur_thread;
    Object    **heap;
    ClassFile *method_area[MAX_METHOD];
    Thread    *threads[MAX_THREADS];
} JVM;

/* hard coded classes */

#define MAX_FIELDS      ((unsigned)16)
#define MAX_METHODS     ((unsigned)16)
#define MAX_INTERFACES  ((unsigned)16)

typedef struct _internalField {
    const char *name;
    const char *desc;
    uint16_t access;
} internalField;

typedef struct _internalMethod {
    const char      *name;
    const char      *desc;
    const uint16_t   access;
    union {
        intStaticMethod mstatic;
        intMethod       mvirtual;
    } meth;
} internalMethod;

typedef struct _internalClass {
    const char *super;
    const char *name;
    const bool override;
    const uint16_t access;
    internalField fields[MAX_FIELDS];
    internalMethod methods[MAX_METHODS];
    const char *interfaces[MAX_INTERFACES];
} internalClass;

typedef struct _prim_class_map {
    const char   type;
    const char  *c_name;
    Object      *class;
} prim_class_map_t;

typedef struct _ary_map {
    const char  *canonical;
    Object      *object;
} ary_map_t;

/*
 * Constants
 */

static const struct predefinedAttribute predefinedAttrs[] = {
    {ATTR_CONSTANT_VALUE,   "ConstantValue",    sizeof(ConstantValue_attribute)},
    {ATTR_CODE,             "Code",             sizeof(Code_attribute)},
    {ATTR_STACKMAPTABLE,    "StackMapTable",    sizeof(StackMapTable_attribute)},
    {ATTR_EXCEPTIONS,       "Exceptions",       sizeof(Exceptions_attribute)},
    {ATTR_BOOTSTRAPMETHODS, "BootstrapMethods", sizeof(BootstrapMethods_attribute)},
    {ATTR_INNERCLASSES,     "InnerClasses",     sizeof(InnerClasses_attribute)},
    {ATTR_SOURCEFILE,       "SourceFile",       sizeof(SourceFile_attribute)},
    {ATTR_LINENUMBERTABLE,  "LineNumberTable",  sizeof(LineNumberTable_attribute)},
    {0, NULL, 0},
};

static prim_class_map_t primitiveClassMap[] = {
    {TYPE_INT,      "java.lang.Integer",    NULL},
    {TYPE_CHAR,     "java.lang.Character",  NULL},
    {TYPE_BOOL,     "java.lang.Boolean",    NULL},
    {TYPE_LONG,     "java.lang.Long",       NULL},
    {TYPE_FLOAT,    "java.lang.Float",      NULL},
    {TYPE_DOUBLE,   "java.lang.Double",     NULL},
    {TYPE_SHORT,    "java.lang.Short",      NULL},
    {TYPE_BYTE,     "java.lang.Byte",       NULL},
    {TYPE_VOID,     "java.lang.Void",       NULL},

    {0, NULL, NULL}
};

/*
 * Forward function declarations
 */

static Object *getClassObject(Thread *, ClassFile *);
static bool linkCpString(Thread *, cp_info *);
static bool fixupCpString(Thread *, cp_info *, cp_info **, uint16_t);
static bool linkClass(Thread *, ClassFile *);
static Operand *newStringFromChar(Thread *, const Object *, Object **);
static field_info *findClassFieldForClass(Thread *, ClassFile *, const char *);
static bool isClassLoaded(Thread *, const char *);
static void freeClass(JVM *jvm, ClassFile *cf);
static bool freeObject(JVM *jvm, Object *o);
static Operand *newString(Thread *, const char *, Object **);
static bool fixupCpUtf8(cp_info *);
static ClassFile *findClass2(Thread *, const char *, bool);
static Operand *throw(Thread *, const char *, const char *, int64_t pc);
static Operand *startThread(Thread *);
static Thread *newThread(JVM *, Object *);
static ClassFile *loadClass(Thread *, const char *);
static Object *newArray(Thread *, uint8_t, int32_t, const ClassFile *);
static Object *newMultiArray(Thread *thread, uint8_t type, int32_t *sizes, uint8_t dim, const ClassFile *clsType);
static Object *newObject(Thread *, const ClassFile *const);
static Operand *dupOperand(const Operand *);
static Operand *newOperand(uint8_t, const void *);
static bool addClass(Thread *, ClassFile *);
static ClassFile *findClass(Thread *, const char *);
static method_info *findClassMethod(Thread *, const char *, const char *, const char *);
static method_info *findMethodByClass(Thread *, const ClassFile *, const char *, const char *);
static Operand *classInit(Thread *, ClassFile *cls);
static uint32_t freeAndNullOperand(Operand **o);
static uint32_t freeOperand(Operand *);
static Operand *invokeMethod(Thread *, const method_info *, bool, int64_t);
static Object *createClass(Thread *, ClassFile *);

#ifdef DEBUG
static void dumpClass(const ClassFile *);
#endif
#ifdef DEBUG_CODE
static const char *printObject(const Object *o);
static void dumpFrame(const Frame *f);
static void dumpStack(const Frame *frame, const char *op);
static void disasm(const uint8_t *const code, uint32_t len);
#endif

#if 0
static ClassFile *buildInternalClass(internalClass *ic);
#endif

/*
 * Inline function defintions
 */

/*
   static int max(const int a, const int b)
   {
   return a < b ? b : a;
   }
   */

static unsigned int umax(unsigned int a, unsigned int b)
{
    return a < b ? b : a;
}

static Frame *currentFrame(Thread * thread)
{
    if (thread->frame_free == 0) return NULL;
    return thread->stack[thread->frame_free-1];
}

/*
 * Function defintions
 */

static const char *printOpType(uint8_t type)
{
    static char buf [BUFSIZ];
    buf[0] = '\0';

    switch(type)
    {
        case TYPE_BYTE:     return "byte";          break;
        case TYPE_CHAR:     return "char";          break;
        case TYPE_DOUBLE:   return "double";        break;
        case TYPE_FLOAT:    return "float";         break;
        case TYPE_OREF:     return "reference (L)"; break;
        case TYPE_AREF:     return "reference ([)"; break;
        case TYPE_SHORT:    return "short";         break;
        case TYPE_BOOL:     return "bool";          break;
        case TYPE_VOID:     return "void";          break;
        case TYPE_NULL:     return "null";          break;
        case TYPE_RETADDR:  return "returnAddress"; break;
        case TYPE_INT:      return "int";           break;
        case TYPE_LONG:     return "long";          break;
        default:
                            snprintf(buf, sizeof(buf), "<<unknown 0x%02x>>", type);
                            return buf;
                            break;
    }
}

#define BT_SIZE 200
#ifdef DEBUG_CODE
//static void *bt_buffer[BT_SIZE];
#endif
static const char *printOpValue(const Operand *op);

static char *printArray(const Object *o)
{
    static char buf[BUFSIZ/2];
    FILE *bfp = fmemopen(buf, sizeof(buf), "w");

    for(uint32_t i = 0; i < o->data.array.len; i++)
    {
        if (o->data.array.data[i])
            fprintf(bfp, "%s", printOpValue(o->data.array.data[i]));
        if (o->data.array.type != TYPE_CHAR && i + 1 < o->data.array.len)
            fprintf(bfp, ",");
    }

    fclose(bfp);
    return buf;
}

static const char *printOpValue(const Operand *op)
{
    if (op == NULL)
        return "<>";

    static char buf[BUFSIZ];
    buf[0] = '\0';

    switch(op->type)
    {
        case TYPE_OREF:
            if (op->val.vref == NULL) {
                snprintf(buf, sizeof(buf), "<op null>");
                return buf;
            }
        default:
            break;
    }

    switch(op->type)
    {
        case TYPE_BYTE:     snprintf(buf, sizeof(buf), "%d", op->val.vbyte); break;
        case TYPE_CHAR:     snprintf(buf, sizeof(buf), "%c", op->val.vchar); break;
        case TYPE_DOUBLE:   snprintf(buf, sizeof(buf), "%d", (int)op->val.vdouble); break; // FIXME printf
        case TYPE_FLOAT:    snprintf(buf, sizeof(buf), "%d", (int)op->val.vfloat); break; // FIXME printf
        case TYPE_OREF:     snprintf(buf, sizeof(buf), "%s",
                                    op->val.vref->class->this_class->d.class.name ? op->val.vref->class->this_class->d.class.name->d.utf8.ascii
                                    : "<null class name>"); break;
        case TYPE_AREF:     snprintf(buf, sizeof(buf), "%s:%s",
                                    printOpType(op->val.vref->data.array.type),
                                    printArray(op->val.vref)); break;
        case TYPE_SHORT:    snprintf(buf, sizeof(buf), "%d", op->val.vshort); break;
        case TYPE_BOOL:     snprintf(buf, sizeof(buf), "%d", op->val.vbool); break;
        case TYPE_VOID:     snprintf(buf, sizeof(buf), "<void>"); break;
        case TYPE_NULL:     snprintf(buf, sizeof(buf), "<null>"); break;
        case TYPE_RETADDR:  snprintf(buf, sizeof(buf), "%d", op->val.vretaddr); break;
        case TYPE_INT:      snprintf(buf, sizeof(buf), "%d", op->val.vint); break;
        case TYPE_LONG:     snprintf(buf, sizeof(buf), "%ld", op->val.vlong); break;
        default:            snprintf(buf, sizeof(buf), "<<unknown type>>"); break;
    }

    return buf;
}

#ifdef DEBUG_CODE
static void dumpStack(const Frame *const frame, const char *op)
{
    if (!frame->sp) {
        printf(" %4s\t<now empty>\n", op);
        return;
    }

    for(uint32_t i = frame->sp; i > 0; i--)
    {
        printf(" %4s\t[%d] ",
                i == frame->sp ? op : "",
                i-1);
        printf("%s [%s] ",
                printOpType(frame->stack[i-1]->type),
                printOpValue(frame->stack[i-1])
              );
        printf("\n");
    }
    printf("\n");
}
#endif

static bool push(Frame *const frame, const Operand *op)
{
    assert(op != 0);
    assert(op->type != 0);

    if (frame->sp >= frame->num_stack)
        return false;

    frame->stack[frame->sp++] = (Operand *)op;
    return true;
}

__attribute__((pure)) static Operand *peek(Frame *const frame)
{
	assert(frame->sp > 0);
	return frame->stack[frame->sp - 1];
}

static Operand *pop(Frame *frame)
{
    assert(frame->sp > 0);

    Operand *ret = (Operand *)frame->stack[--frame->sp];
    frame->stack[frame->sp] = NULL;

    return ret;
}

__attribute__((pure)) static Operand *getLocal(Frame *const frame, uint16_t slot)
{
    assert(slot < frame->num_local);

    return frame->local[slot];
}

static void clearAndFreeLocal(Frame *frame, uint16_t slot)
{
    const Operand *const value = getLocal(frame, slot);
    if (value == NULL) {
        return;
    }

    if (value->type == TYPE_LONG || value->type == TYPE_DOUBLE) {
        if (slot-1U < frame->num_local && frame->local[slot-1U] == value)
            frame->local[slot-1U] = NULL;
        else if (slot < frame->num_local-1U && frame->local[slot+1U] == value)
            frame->local[slot+1U] = NULL;
    }

    frame->local[slot] = NULL;
    freeOperand((Operand *)value);
}

static Operand *setLocal(Frame *frame, uint16_t slot, Operand *value)
{
    if (value == NULL || slot >= frame->num_local) return NULL;

    const bool islong = (value->type == TYPE_LONG || value->type == TYPE_DOUBLE);

    clearAndFreeLocal(frame, slot);
    if (islong) clearAndFreeLocal(frame, (uint16_t)(slot+1U));

    frame->local[slot] = value;
    if (islong) frame->local[slot+1U] = value;

    return value;
}

static void printAccessFlags(uint16_t flag, char *const buf, size_t len)
{
    //static char buf[BUFSIZ];
    buf[0] = '\0';

    if (flag & ACC_PUBLIC) strncat(buf, "public ",len);
    if (flag & ACC_PRIVATE) strncat(buf, "private ",len);
    if (flag & ACC_PROTECTED) strncat(buf, "protected ",len);
    if (flag & ACC_STATIC) strncat(buf, "static ",len);
    if (flag & ACC_FINAL) strncat(buf,"final ",len);
    if (flag & ACC_SUPER) strncat(buf,"super ",len);
    if (flag & ACC_VOLATILE) strncat(buf,"volatile ",len);
    if (flag & ACC_NATIVE) strncat(buf,"native ",len);
    if (flag & ACC_INTERFACE) strncat(buf,"interface ",len);
    if (flag & ACC_ABSTRACT) strncat(buf,"abstract ",len);
    if (flag & ACC_SYNTHETIC) strncat(buf,"synth ",len);
    if (flag & ACC_ANNOTATION) strncat(buf,"annotation ",len);
    if (flag & ACC_ENUM) strncat(buf,"enum ",len);

    //return buf;
}

static bool read4(FILE *f, uint32_t *dst)
{
    size_t ret;
    if ( (ret = fread(dst, 1, sizeof(uint32_t), f)) != sizeof(uint32_t)) {
        warn("read4 %ld", ret);
        return false;
    }
    *dst = ntohl(*dst);
    return true;
}

static bool read2(FILE *f, uint16_t *dst)
{
    if ( (fread(dst, 1, sizeof(uint16_t), f)) != sizeof(uint16_t)) return false;
    *dst = ntohs(*dst);
    return true;
}

static bool read1(FILE *f, uint8_t *dst)
{
    if ( (fread(dst, 1, sizeof(uint8_t), f)) != sizeof(uint8_t)) return false;
    return true;
}

static Object *findObjForPrim(uint8_t type)
{
    for (ssize_t i = 0; primitiveClassMap[i].c_name; i++)
        if (primitiveClassMap[i].type == type)
            return primitiveClassMap[i].class;

    warnx("unable to find ObjForPrim %c", type);
    return NULL;
}

static cp_info *readConstantPool(FILE *f)
{
    uint8_t tag;
    uint32_t tmp;
    uint16_t tmp16;

    cp_info *ret = NULL;
    if (!read1(f, &tag)) return NULL;

    switch(tag)
    {
        case CONSTANT_Class:
            if ((ret = calloc(1, sizeof(cp_info))) == NULL) return NULL;
            ret->tag = tag;
            if (!read2(f, &ret->d.class.name_index)) goto fail;
            break;

        case CONSTANT_Fieldref:
        case CONSTANT_Methodref:
        case CONSTANT_InterfaceMethodref:
            if ((ret = calloc(1, sizeof(cp_info))) == NULL) return NULL;
            if (!read2(f, &ret->d.fr.class_index)) goto fail;
            if (!read2(f, &ret->d.fr.name_and_type_index))
                goto fail;
            break;

        case CONSTANT_String:
            if ((ret = calloc(1, sizeof(cp_info))) == NULL)
                return NULL;
            if (!read2(f, &ret->d.str.string_index)) goto fail;
            break;

        case CONSTANT_InvokeDynamic:
            if ((ret = calloc(1, sizeof(cp_info))) == NULL)
                return NULL;
            if (!read2(f, &ret->d.id.bootstrap_method_attr_index)) goto fail;
            if (!read2(f, &ret->d.id.name_and_type_index)) goto fail;
            break;

        case CONSTANT_Double:
            {
                if ((ret = calloc(1, sizeof(cp_info))) == NULL) return NULL;
                uint64_t dbl = 0;
                if (!read4(f, &tmp)) goto fail;
                dbl = ((uint64_t)tmp << 32);
                if (!read4(f, &tmp)) goto fail;
                dbl += tmp;
                double val = 0;

                //printf("CONSTANT_Double: raw=%lu ", dbl);

                if (dbl == 0x7ff0000000000000L)
                    val = -INFINITY;
                else if (dbl == 0xfff0000000000000L)
                    val = INFINITY;
                else if ((dbl >= 0x7ff0000000000001L && dbl <= 0x7ff0000000000001L) ||
                        (dbl >= 0xfff0000000000001L && dbl <= 0xffffffffffffffffL))
                    val = NAN;
                else {
                    const int s = ((dbl >> 63) == 0) ? 1 : -1;
                    const int e = (int)((dbl >> 52) & 0x7FFL);
                    const long m = (e == 0) ?
                        (long)((dbl & 0xfffffffffffffL) << 1) :
                        (long)((dbl & 0xfffffffffffffL) | 0x10000000000000L);
                    val = (double)(s * m) * pow(2, e - 1075);
                }
                //printf("val = %g\n", val);
                ret->d.tdouble.c_double = val;
            }
            break;

        case CONSTANT_Long:
            if ((ret = calloc(1, sizeof(cp_info))) == NULL) return NULL;
            if (!read4(f, &tmp)) goto fail;
            ret->d.tlong.c_long = ((int64_t)tmp) << 32;
            if (!read4(f, &tmp)) goto fail;
            ret->d.tlong.c_long += tmp;
            break;

        case CONSTANT_Utf8:
            if (!read2(f, &tmp16)) goto fail;
            if ((ret = calloc(1, sizeof(cp_info))) == NULL)
                return NULL;
            ret->d.utf8.length = tmp16;
            //if (tmp16 == 0) {
            //  warnx("constant pool is zero length");
            //}
            if ((ret->d.utf8.bytes = calloc(tmp16 + 1U, sizeof(uint8_t))) == NULL)
                goto fail;
            if ((fread(ret->d.utf8.bytes, 1, tmp16, f)) != tmp16)
                goto fail;
            fixupCpUtf8(ret);
            break;

        case CONSTANT_NameAndType:
            if ((ret = calloc(1, sizeof(cp_info))) == NULL)
                return NULL;
            if (!read2(f, &ret->d.nt.name_index)) goto fail;
            if (!read2(f, &ret->d.nt.descriptor_index)) goto fail;
            break;

        case CONSTANT_MethodHandle:
            if ((ret = calloc(1, sizeof(cp_info))) == NULL)
                return NULL;
            if (!read1(f, &ret->d.mh.reference_kind))
                goto fail;
            if (!read2(f, &ret->d.mh.reference_index))
                goto fail;
            break;

        case CONSTANT_MethodType:
            if ((ret = calloc(1, sizeof(cp_info))) == NULL)
                return NULL;
            if (!read2(f, &ret->d.mt.descriptor_index))
                goto fail;
            break;

        case CONSTANT_Float:
            {
                if ((ret = calloc(1, sizeof(cp_info))) == NULL)
                    return NULL;
                if (!read4(f, &tmp))
                    goto fail;
                float tmpf = 0;
                if (tmp == 0x7f800000) tmpf = INFINITY;
                else if (tmp == 0xff800000) tmpf = -INFINITY;
                else if (   (tmp > 0x7f800000 && tmp < 0x80000000) ||
                        (tmp > 0xf8000000 && tmp <= 0xffffffff))
                    tmpf = NAN;
                else {
                    const int s = ((tmp >> 31) == 0) ? 1 : -1;
                    const int e = ((tmp >> 23) & 0xFF);
                    const int m = (e == 0) ?
                        (int)((tmp & 0x7FFFFF) << 1) :
                        (int)((tmp & 0x7FFFFF) | 0x800000);
                    tmpf = (float)(s * m * pow(2, e - 150));
                }
                ret->d.tfloat.c_float = tmpf;
            }
            break;

        case CONSTANT_Integer:
            if ((ret = calloc(1, sizeof(cp_info))) == NULL) return NULL;
            if (!read4(f, &tmp)) goto fail;
            ret->d.tint.c_int = (int32_t)tmp;

            //((cp_Integer_info *)ret)->c_int =
            //  ntohl(((cp_Integer_info *)ret)->c_int);
            break;

        default:
            errx(EXIT_FAILURE, "Unknown CONSTANT %d", tag);
    }
    ret->tag = tag;
    return ret;
fail:
    if (ret) free(ret);
    return NULL;
}

static Frame *newFrame(uint16_t num_stacks, uint16_t num_locals)
{
    Frame *ret = calloc(1, sizeof(Frame));

    if (ret == NULL) {
        warn("newFrame");
        goto fail;
    }

    ret->frameNum = framecounter++;
    ret->num_stack = (uint16_t)umax(1U,num_stacks);
    ret->num_local = (uint16_t)umax(1U,num_locals);

    if ((ret->stack = calloc(num_stacks, sizeof(Operand *))) == NULL) {
        warn("newFrame");
        goto fail;
    }

    if ((ret->local = calloc(ret->num_local, sizeof(Operand *))) == NULL) {
        warn("newFrame");
        goto fail;
    }

    return ret;

fail:
    if (ret) {
        if (ret->local) free(ret->local);
        if (ret->stack) free(ret->stack);
        ret->local = ret->stack = NULL;
        free(ret);
        ret = NULL;
    }
    return NULL;
}

static void freeFrame(Frame *f)
{
#ifdef DEBUG
//  printf("(freeFrame %p num_local=%d sp=%d\n", (void *)f, f->num_local, f->num_stack);
#endif

    if (f->local) {
        for (uint16_t i = 0; i < f->num_local; i++)
            clearAndFreeLocal(f, i);

        free(f->local);
        f->local = NULL;
    }

    if (f->stack) {
        for (uint16_t i = 0; i < f->num_stack; i++)
            if (f->stack[i]) {
                freeAndNullOperand((Operand **)&f->stack[i]);
            }
        free(f->stack);
        f->stack = NULL;
    }

    if (f->ret) {
        freeAndNullOperand(&f->ret);
    }

    free(f);
}

static exception *readException(uint32_t me, FILE *f, cp_info **cp, uint32_t cpcnt)
{
    exception *ret = NULL;
    if ((ret = calloc(1, sizeof(exception))) == NULL) return NULL;
    if (!read2(f, &ret->start_pc)) goto fail;
    if (!read2(f, &ret->end_pc)) goto fail;
    if (!read2(f, &ret->handler_pc)) goto fail;
    if (!read2(f, &ret->catch_type_idx)) goto fail;
    if (ret->catch_type_idx)
        ret->catch_type = cp[ret->catch_type_idx];
    else
        ret->catch_type = NULL;

    return ret;
fail:
    warnx("readException: %d", me);
    if(ret) free(ret);
    return NULL;
}

#ifdef DEBUG_CODE
static void disasm(const uint8_t *const code, uint32_t len)
{
    const uint8_t *ptr = code;
    uint16_t tmp16;
    uint8_t tmp8,tmp82;

    while(ptr < code + len)
    {
        printf("%ld: ", ptr - code);
        const uint8_t opcode = *ptr;
        switch(opcode)
        {
            case 0x01:
                printf("aconst_null");
                break;
            case 0x02:
                printf("iconst_m1");
                break;
            case 0x03:
                printf("iconst_0");
                break;
            case 0x04:
                printf("iconst_1");
                break;
            case 0x05:
                printf("iconst_2");
                break;
            case 0x09:
                printf("lconst_0");
                break;
            case 0x0a:
                printf("lconst_1");
                break;
            case 0x2a:
                printf("aload_0");
                break;
            case 0x2b:
                printf("aload_1");
                break;
            case 0x2c:
                printf("aload_2");
                break;
            case 0xbe:
                printf("arraylength");
                break;
            case 0x3b:
                printf("istore_0");
                break;
            case 0x3c:
                printf("istore_1");
                break;
            case 0x3d:
                printf("istore_2");
                break;
            case 0x3e:
                printf("istore_3");
                break;
            case 0x1a:
                printf("iload_0");
                break;
            case 0x1b:
                printf("iload_1");
                break;
            case 0x1c:
                printf("iload_2");
                break;
            case 0x1d:
                printf("iload_3");
                break;
            case 0xb0:
                printf("areturn");
                break;
            case 0xac:
                printf("ireturn");
                break;
            case 0xb1:
                printf("return");
                break;
            case 0xc0:
                tmp16 = (uint16_t)(256U * (*(ptr+1)) + (*(ptr+2))); ptr+=2;
                //tmp16 = (((uint16_t)*(++ptr))<<8); tmp16 += *(++ptr);
                printf("checkcast     #%d", tmp16);
                break;
            case 0xb2:
                tmp16 = (uint16_t)(256U * (*(ptr+1)) + (*(ptr+2))); ptr+=2;
                //tmp16 = (((uint16_t)*(++ptr))<<8); tmp16 += *(++ptr);
                printf("getstatic     #%d", tmp16);
                break;
            case 0xb3:
                tmp16 = (uint16_t)(256U * (*(ptr+1)) + (*(ptr+2))); ptr+=2;
                //tmp16 = (((uint16_t)*(++ptr))<<8); tmp16 += *(++ptr);
                printf("putstatic     #%d", tmp16);
                break;
            case 0x12:
                tmp8 = *(++ptr);
                printf("ldc           #%d", tmp8);
                break;
            case 0x13:
                //tmp16 = (((uint16_t)*(++ptr))<<8); tmp16 += *(++ptr);
                tmp16 = (uint16_t)(256U * (*(ptr+1)) + (*(ptr+2))); ptr+=2;
                printf("ldc_w         #%d", tmp16);
                break;
            case 0x14:
                //tmp16 = (((uint16_t)*(++ptr))<<8); tmp16 += *(++ptr);
                tmp16 = (uint16_t)(256U * (*(ptr+1)) + (*(ptr+2))); ptr+=2;
                printf("ldc2_w        #%d", tmp16);
                break;
            case 0xb6:
                //tmp16 = (((uint16_t)*(++ptr))<<8); tmp16 += *(++ptr);
                tmp16 = (uint16_t)(256U * (*(ptr+1)) + (*(ptr+2))); ptr+=2;
                printf("invokevirtual #%d", tmp16);
                break;
            case 0xb7:
                //tmp16 = (((uint16_t)*(++ptr))<<8); tmp16 += *(++ptr);
                tmp16 = (uint16_t)(256U * (*(ptr+1)) + (*(ptr+2))); ptr+=2;
                printf("invokespecial #%d", tmp16);
                break;
            case 0xb8:
                //tmp16 = (((uint16_t)*(++ptr))<<8); tmp16 += *(++ptr);
                tmp16 = (uint16_t)(256U * (*(ptr+1)) + (*(ptr+2))); ptr+=2;
                printf("invokestatic  #%d", tmp16);
                break;
            case 0xb9:
                //tmp16 = (((uint16_t)*(++ptr))<<8); tmp16 += *(++ptr);
                tmp16 = (uint16_t)(256U * (*(ptr+1)) + (*(ptr+2))); ptr+=2;
                printf("invokeinterface #%d", tmp16);
                break;
            case 0xbb:
                //tmp16 = (((uint16_t)*(++ptr))<<8); tmp16 += *(++ptr);
                tmp16 = (uint16_t)(256U * (*(ptr+1)) + (*(ptr+2))); ptr+=2;
                printf("new           #%d", tmp16);
                break;
            case 0x57:
                printf("pop");
                break;
            case 0x59:
                printf("dup");
                break;
            case 0xbf:
                printf("athrow");
                break;
            case 0xa7:
                //tmp16 = (((uint16_t)*(++ptr))<<8); tmp16 += *(++ptr);
                tmp16 = (uint16_t)(256U * (*(ptr+1)) + (*(ptr+2))); ptr+=2;
                printf("goto        %4ld", (ptr - code - 2) + tmp16);
                break;
            case 0x84:
                tmp8 = *(++ptr);
                tmp82 = *(++ptr);
                printf("iinc          %d,%d", tmp8, tmp82);
                break;
            case 0x9a:
                //tmp16 = (((uint16_t)*(++ptr))<<8); tmp16 += *(++ptr);
                tmp16 = (uint16_t)(256U * (*(ptr+1)) + (*(ptr+2))); ptr+=2;
                printf("ifne        %4ld", (ptr - code - 2) + tmp16);
                break;
            case 0x9e:
                //tmp16 = (((uint16_t)*(++ptr))<<8); tmp16 += *(++ptr);
                tmp16 = (uint16_t)(256U * (*(ptr+1)) + (*(ptr+2))); ptr+=2;
                printf("ifle        %4ld", (ptr - code - 2) + tmp16);
                break;
            case 0xa2:
                tmp16 = (uint16_t)(256U * (*(ptr+1)) + (*(ptr+2))); ptr += 2;
                //tmp16 = (((uint16_t)*(++ptr))<<8); tmp16 += *(++ptr);
                printf("if_icmpge     %4ld", (ptr - code - 2) + tmp16);
                break;
            case 0xc7:
                //tmp16 = (((uint16_t)*(++ptr))<<8); tmp16 += *(++ptr);
                tmp16 = (uint16_t)(256U * (*(ptr+1)) + (*(ptr+2))); ptr+=2;
                printf("ifnonnull     %4ld", (ptr - code - 2) + tmp16);
                break;
            case 0x00:
                printf("nop");
                break;
            default:
                printf("Unknown opcode: %02x", opcode);
        }

        ptr++;
        printf("\n");
    }
    printf("\n");
}
#endif

static union verification_type_info *readVerificationTypeInfo(FILE *f)
{
    uint8_t tag;
    if (!read1(f, &tag))
        return NULL;
    union verification_type_info *ret = NULL;

    switch(tag)
    {
        case ITEM_Top:
        case ITEM_Integer:
        case ITEM_Float:
        case ITEM_Null:
        case ITEM_Long:
        case ITEM_Double:
        case ITEM_UninitialisedThis:
            {
                if ((ret = calloc(1, sizeof(union verification_type_info))) == NULL)
                    return NULL;
                ret->bvi.tag = tag;
            }
            break;
        case ITEM_Object:
            {
                if ((ret = calloc(1, sizeof(union verification_type_info))) == NULL)
                    return NULL;
                ret->ovi.tag = tag;
                if (!read2(f, &ret->ovi.cpool_index)) {
                    free(ret);
                    return NULL;
                }
            }
            break;
        case ITEM_Uninitialised:
            {
                if ((ret = calloc(1, sizeof(union verification_type_info))) == NULL)
                    return NULL;
                ret->uvi.tag = tag;
                if (!read2(f, &ret->uvi.offset)) {
                    free(ret);
                    return NULL;
                }
            }
            break;
        default:
            warnx("unknown tag: %d", tag);
            return NULL;
    }

    return ret;
}

static attribute_info *readAttribute(Thread *thread, uint32_t me, FILE *f, cp_info **cp, uint32_t cp_cnt)
{
    const JVM *jvm = thread->jvm;
    attribute_info *ret = NULL;
    uint16_t idx;
    uint32_t len;

    if (!read2(f, &idx)) { 
        warnx("[%3d] readAttribute: read idx", me);
        return NULL;
    }
    if (idx > cp_cnt || cp[idx] == NULL || cp[idx]->tag != CONSTANT_Utf8)
        warnx("[%3d] attribute: invalid attribute_name_index %d", me, idx);

    if (!read4(f, &len)) {
        warnx("[%3d] readAttribute: read len", me);
        return NULL;
    }

    bool found = false;
    const struct predefinedAttribute *pa;

    for (int i = 0; ((pa = &predefinedAttrs[i])->name) != NULL; i++)
    {
        
        if (strcmp(cp[idx]->d.utf8.ascii, pa->name)) continue;
        switch (pa->attr)
        {
            case ATTR_LINENUMBERTABLE:
                {
                    found = true;
                    if ((ret = calloc(1, sizeof(attribute_info))) == NULL) goto fail;
                    ret->attribute_name_index = idx;
                    ret->attribute_length = len;

                    LineNumberTable_attribute *attr = calloc(1, pa->size);
                    if (attr == NULL) goto fail;
                    if (!read2(f, &attr->line_number_table_length)) goto fail;
                    if ((attr->line_number_table = calloc(attr->line_number_table_length,
                                    sizeof(line_number_t))) == NULL)
                        goto fail;

                    for (ssize_t i = 0; i < attr->line_number_table_length; i++) {
                        if (!read2(f, &attr->line_number_table[i].start_pc)) goto fail;
                        if (!read2(f, &attr->line_number_table[i].line_number)) goto fail;
                        if (i == attr->line_number_table_length - 1)
                            attr->line_number_table[i].end_pc = 0xFFFF;
                        if (i > 0)
                            attr->line_number_table[i-1].end_pc =
                                attr->line_number_table[i].start_pc;
                    }

                    ret->d.lineno = attr;
                }
                break;

            case ATTR_SOURCEFILE:
                {
                    found = true;
                    if ((ret = calloc(1, sizeof(attribute_info))) == NULL) goto fail;
                    ret->attribute_name_index = idx;
                    ret->attribute_length = len;

                    SourceFile_attribute *attr = calloc(1, pa->size);
                    if (attr == NULL) goto fail;
                    if (!read2(f, &attr->sourcefile_index)) goto fail;
                    attr->name = cp[attr->sourcefile_index];

                    ret->d.srcfile = attr;
                }
                break;

            case ATTR_CODE:
                {
                    found = true;
                    if ((ret = calloc(1, sizeof(attribute_info))) == NULL) {
                        warn("readAttribute: ATTR_CODE: calloc ret");
                        goto fail;
                    }
                    ret->attribute_name_index = idx;
                    ret->attribute_length = len;

                    Code_attribute *attr = calloc(1, pa->size);
                    if (attr == NULL) { 
                        warn("readAttribute: ATTR_CODE: calloc attr");
                        goto fail;
                    }
                    if (!read2(f, &attr->max_stack)) { warnx("readAttribute: ATTR_CODE read max_stack"); goto fail; }
                    if (!read2(f, &attr->max_locals)) { warnx("readAttribute: ATTR_CODE read max_locals"); goto fail; }
                    if (!read4(f, &attr->code_length)) { warnx("readAttribute: ATTR_CODE read code_length"); goto fail; }

                    uint8_t *code = NULL;
                    if ((attr->code = code = calloc(1, attr->code_length)) == NULL) {
                        warn("readAttribute: ATTR_CODE: calloc(%d) code", attr->code_length);
                        goto fail;
                    }

                    /* FIXME attr->code memory leak */
                    if (fread(code, 1, attr->code_length, f) != attr->code_length) {
                        warnx("readAttribute: ATTR_CODE read code != code_length");
                        goto fail;
                    }
                    if (!read2(f, &attr->exception_table_length)) { warnx("readAttribute: ATTR_CODE read ex_tbl_len"); goto fail; }
                    if ((attr->exception_table = calloc(attr->exception_table_length+1U,
                                    sizeof(exception *))) == NULL) goto fail;

                    /* FIXME attr->exception_table memory leak */
                    for (uint32_t j = 0; j < attr->exception_table_length; j++)
                        if ((attr->exception_table[j] =
                                    readException(j, f, cp, cp_cnt)) == NULL) {
                            warnx("readAttribute: readException %d", j);
                            goto fail;
                        }
                    if (!read2(f, &attr->attributes_count)) { warnx("readAttribute: ATTR_CODE read attr_cnt"); goto fail; }
                    if ((attr->attributes = calloc(attr->attributes_count+1U,
                                    sizeof(attribute_info *))) == NULL) {
                        warn("readAttribute: calloc(%d) attributes", attr->attributes_count+1U);
                        goto fail;
                    }
                    /* FIXME attr->attributes memory leak */
                    for (uint32_t j = 0; j < attr->attributes_count; j++) {
                        if ((attr->attributes[j] = readAttribute(thread, j, f, cp, cp_cnt)) == NULL) {
                            warnx("readAttribute: readAttribute on ATTR_CODE failed");
                            goto fail;
                        }
                        if (!strcmp(attr->attributes[j]->name->d.utf8.ascii, "LineNumberTable"))
                            attr->lineno = attr->attributes[j]->d.lineno;

                    }
                    //disasm(attr->code, attr->code_length);
                    ret->d.code = attr;
                }
                break;

            case ATTR_CONSTANT_VALUE:
                {
                    found = true;
                    if ((ret = calloc(1, sizeof(attribute_info))) == NULL) {
                        warn("[%d] readAttribute: CONSTANT_VALUE", me);
                        goto fail;
                    }
                    ret->attribute_name_index = idx;
                    ret->attribute_length = len;
                    ConstantValue_attribute *attr = calloc(1, pa->size);
                    if (attr == NULL) {
                        warn("[%d] readAttribute: CONSTANT_VALUE", me);
                        goto fail;
                    }
                    if (!read2(f, &attr->constantvalue_index)) goto fail;

                    ret->d.constant = attr;
                    attr->entry = cp[ret->d.constant->constantvalue_index];

                    switch(attr->entry->tag)
                    {
                        case CONSTANT_Integer:
                            attr->op.type = TYPE_INT;
                            attr->op.val.vint = attr->entry->d.tint.c_int;
                            break;
                        case CONSTANT_Long:
                            attr->op.type = TYPE_LONG;
                            attr->op.val.vlong = attr->entry->d.tlong.c_long;
                            break;
                        case CONSTANT_Float:
                            attr->op.type = TYPE_FLOAT;
                            attr->op.val.vfloat = attr->entry->d.tfloat.c_float;
                            break;
                        case CONSTANT_Double:
                            attr->op.type = TYPE_DOUBLE;
                            attr->op.val.vdouble = attr->entry->d.tdouble.c_double;
                            break;
                        case CONSTANT_String:
                            attr->op.type = TYPE_OREF;
                            if (jvm->VM) {
                                Object *tmp = NULL;
                                if (attr->entry->d.str.intern == NULL)
                                    linkCpString(thread, (cp_info *)attr->entry);

                                if (newStringFromChar(thread, attr->entry->d.str.intern, &tmp) != NULL) {
                                    warnx("readAttribute: newStringFromChar threw something");
                                    goto fail;
                                }

                                if (tmp == NULL) {
                                    warnx("readAttribute: newStringFromChar didn't set tmp");
                                    goto fail;
                                }


                                tmp->lock++;
                                attr->op.val.vref = tmp;
                            } else {
                                warnx("readAttribute: unable to parse CONSTANT_String during early boot");
                            }
                            break;
                        default:
                            warnx("unsupported ConstantValue attribute index tag %d",
                                    attr->entry->tag);
                            goto fail;
                    }
                }
                break;

            case ATTR_STACKMAPTABLE:
                {
                    found = true;
                    if ((ret = calloc(1, sizeof(attribute_info))) == NULL) goto fail;
                    ret->attribute_name_index = idx;
                    ret->attribute_length = len;
                    StackMapTable_attribute *attr = calloc(1, pa->size);
                    if (attr == NULL) goto fail;
                    if (!read2(f, &attr->number_of_entries)) goto fail;
                    if ((attr->entries = calloc(attr->number_of_entries+1U,
                                    sizeof(stack_map_frame *))) == NULL) {
                        warn("readAttribute: ATTR_STACKMAPTABLE: calloc(%d) entries", attr->number_of_entries+1U);
                        goto fail;
                    }
                    ret->d.stackmap = attr;

                    for(int i = 0; i < attr->number_of_entries; i++)
                    {
                        uint8_t tag;
                        uint16_t offset_delta;
                        if (!read1(f, &tag)) { warnx("readAttribute: ATTR_SMT read tag"); goto fail; }

                        stack_map_frame *smf = NULL;
                        if ((smf = calloc(1, sizeof(stack_map_frame))) == NULL)
                            goto fail;
                        smf->tag = tag;
                        attr->entries[i] = smf;

                        if (tag <= 63) {
                            /* SAME */
                            smf->type = FT_SAME;
                        } else if (tag <= 127) {
                            /* SAME_LOCALS_1_STACK_ITEM */
                            smf->type = FT_SAME_LOCALS_1_STACK_ITEM;
                            union verification_type_info *vi = readVerificationTypeInfo(f);
                            if (vi == NULL) {
                                warnx("readAttribute: ATTR_SMT: readVerificationTypeInfo failed");
                                goto fail;
                            }
                            memcpy(&smf->i.slsi.stack, vi, sizeof(union verification_type_info));
                            free(vi);
                            vi = NULL;
                        } else if (tag <= 246) {
                            errx(EXIT_FAILURE, "unsupported tag: %d", tag);
                        } else if (tag == 247) {
                            errx(EXIT_FAILURE, "unsupported tag: %d", tag);
                        } else if (tag <= 250) {
                            /* CHOP */
                            smf->type = FT_CHOP;
                            if (!read2(f, &offset_delta)) { warnx("readAttribute: ATTR_SMT read CHOP offset_delta"); goto fail; }
                            smf->i.cf.offset_delta = offset_delta;
                        } else if (tag == 251) {
                            smf->type = FT_SAME_FRAME_EXTENDED;
                            if (!read2(f, &offset_delta)) { warnx("readAttribute: ATTR_SMT read FT_S_F_E offset_delta"); goto fail; }
                            smf->i.sfe.offset_delta = offset_delta;
                        } else if (tag <= 254) {
                            /* APPEND */
                            smf->type = FT_APPEND;
                            uint32_t num_locals = tag - 251U;
                            if ((smf->i.af.locals = calloc(num_locals+1U, (size_t)sizeof(union verification_type_info *))) == NULL) {
                                warn("readAttribute: ATTR_SMT: APPEND: calloc(%d) locals", num_locals);
                                goto fail;
                            }
                            smf->i.af.local_num = (uint8_t)num_locals;
                            if (!read2(f, &offset_delta)) { warnx("readAttribute: ATTR_SMT read APPEND offset_delta"); goto fail; }
                            for (uint32_t j = 0; j < num_locals; j++)
                            {
                                if((smf->i.af.locals[j] = readVerificationTypeInfo(f)) == NULL) {
                                    warnx("readAttribute: ATTR_SMT: APPEND: readVerificationTypeInfo failed");
                                    goto fail;
                                }
                            }
                        } else if (tag == 255) {
                            /* FULL_FRAME */
                            smf->type = FT_FULL_FRAME;
                            if (!read2(f, &smf->i.ff.offset_delta)) goto fail;
                            if (!read2(f, &smf->i.ff.number_of_locals)) goto fail;
                            if ((smf->i.ff.locals = calloc(smf->i.ff.number_of_locals+1U,
                                            (size_t)sizeof(union verification_type_info *))) == NULL) {
                                warn("readAttribute: ATTR_SMT: FULL_FRAME: calloc(%d) locals", smf->i.ff.number_of_locals);
                                goto fail;
                            }
                            for (uint32_t j = 0; j < smf->i.ff.number_of_locals; j++)
                            {
                                if((smf->i.ff.locals[j] = readVerificationTypeInfo(f)) == NULL) {
                                    warnx("readAttribute: ATTR_SMT: FULL_FRAME: readVerificationTypeInfo l failed");
                                    goto fail;
                                }
                            }
                            if (!read2(f, &smf->i.ff.number_of_stack_items)) goto fail;
                            if (smf->i.ff.number_of_stack_items && (smf->i.ff.stack = calloc(smf->i.ff.number_of_stack_items+1U,
                                            (size_t)sizeof(union verification_type_info *))) == NULL) {
                                warn("readAttribute: ATTR_SMT: APPEND: calloc(%d) stack", smf->i.ff.number_of_stack_items);
                                goto fail;
                            }
                            for (uint32_t j = 0; j < smf->i.ff.number_of_stack_items; j++)
                            {
                                if((smf->i.ff.stack[j] = readVerificationTypeInfo(f)) == NULL) {
                                    warnx("readAttribute: ATTR_SMT: APPEND: readVerificationTypeInfo si failed");
                                    goto fail;
                                }
                            }
                        }
                    }
                }
                break;

            case ATTR_INNERCLASSES:
                {
                    found = true;
                    if ((ret = calloc(1, sizeof(attribute_info))) == NULL) goto fail;
                    ret->attribute_name_index = idx;
                    ret->attribute_length = len;
                    InnerClasses_attribute *attr = calloc(1, pa->size);
                    if (attr == NULL) goto fail;
                    if (!read2(f, &attr->number_of_classes)) goto fail;
                    if ((attr->classes = calloc(attr->number_of_classes+1U,
                                    sizeof(inner_class_def *))) == NULL) goto fail;
                    ret->d.inner = attr;

                    for (int j = 0; j < attr->number_of_classes; j++)
                    {
                        inner_class_def *def = NULL;

                        if ((def = calloc(1, sizeof(inner_class_def))) == NULL)
                            goto fail;
                        if (!read2(f, &def->inner_class_info_index)) goto fail;
                        if (!read2(f, &def->outer_class_info_index)) goto fail;
                        if (!read2(f, &def->inner_name_index)) goto fail;
                        if (!read2(f, &def->inner_class_access_flags)) goto fail;

                        if (def->inner_class_info_index)
                            def->inner_ci = cp[def->inner_class_info_index];
                        if (def->outer_class_info_index)
                            def->outer_ci = cp[def->outer_class_info_index];
                        if (def->inner_name)
                            def->inner_name = cp[def->inner_name_index];

                        ret->d.inner->classes[j] = def;
                    }
                }
                break;

            case ATTR_EXCEPTIONS:
                {
                    found = true;
                    if ((ret = calloc(1, sizeof(attribute_info))) == NULL) goto fail;
                    ret->attribute_name_index = idx;
                    ret->attribute_length = len;
                    Exceptions_attribute *attr = calloc(1, pa->size);
                    if (attr == NULL) goto fail;
                    if (!read2(f, &attr->number_of_exceptions)) goto fail;
                    if ((attr->exception_index_table = calloc(attr->number_of_exceptions+1U,
                                    sizeof(cp_info *))) == NULL) goto fail;
                    ret->d.exception = attr;

                    for (int j = 0; j < attr->number_of_exceptions; j++)
                    {
                        uint16_t tmp;
                        if (!read2(f, &tmp)) {
                            warnx("[%d] readAttribute: excep", me);
                            goto fail;
                        }
                        attr->exception_index_table[j] = cp[tmp];
                    }
                }
                break;
            case ATTR_BOOTSTRAPMETHODS:
            default:
                warnx("Unimplemented predefined attribute %s", pa->name);
                break;
            case ATTR_INVALID:
                errx(EXIT_FAILURE, "Invalid predefined attribute %s", pa->name);
                break;
        }
    }
    

    if (!found) {
        
        if ((ret = calloc(1, sizeof(attribute_info) + len)) == NULL) {
            warn("readAttribute: !found calloc(%ld) ret", sizeof(attribute_info) + len);
            return NULL;
        }
        
        ret->attribute_name_index = idx;
        ret->attribute_length = len;
        
        if (fread((uint8_t *)&ret->info, 1, len, f) != len) {
            warnx("readAttribute: fread failed to read %d", len);
            goto fail;
        }
        ret->d.data = NULL;
        ret->name = cp[idx];
    
    } else {
        ret->name = cp[idx];
    
    }

    
    return ret;
fail:
    warnx("[%d] readAttribute: failed on pa->attr == %d", me, pa->attr);
    if (ret) {
        if (ret->d.data) free((void *)ret->d.data);
        ret->d.data = NULL;
        free(ret);
    }
    return NULL;
}

static const char *parseParamDescriptor(des_Field *desc, const char *str)
{
    uint8_t *ptr = (uint8_t *)str;
loop:
    switch (*ptr)
    {
        case TYPE_BYTE:
        case TYPE_CHAR:
        case TYPE_DOUBLE:
        case TYPE_FLOAT:
        case TYPE_INT:
        case TYPE_LONG:
        case TYPE_SHORT:
        case TYPE_BOOL:
        case TYPE_VOID:
            desc->baseType = *ptr++;
            return (char *)ptr;
            break;

        case TYPE_OREF:
            {
                desc->baseType = *ptr;
                ptr += 1;
                uint8_t *tmp = ptr;
                while (*tmp && *tmp != ';') tmp++;
                if (!*tmp) {
                    warnx("broken %s", str);
                    return NULL;
                }
                tmp++; // skip ;
                desc->className = strndup((char *)ptr, (size_t)(tmp-ptr-1U));
                desc->orig_className = strdup(desc->className);
                for(ptr = (uint8_t *)desc->className; *ptr && *ptr != ';'; ptr++)
                    if(*ptr == '/') *ptr = '.';
                return (char *)tmp;
            }
            break;

        case TYPE_AREF:
            {
                while(*ptr == '[') {
                    desc->array++;
                    ptr++;
                }
                goto loop;
            }
            break;

        default:
            warnx("Unknown/null type %d(%c)", *ptr, *ptr ? *ptr : ' ');
            return NULL;
    }

    if (!*ptr) {
        warnx("Reached end of '%s'", str);
        return NULL;
    }

    return str;
}

#if 0
static const char *dumpParamDesc(const des_Field *desc)
{
    static char buf[BUFSIZ];
    FILE *bfp = fmemopen(buf, sizeof(buf), "w");

    fprintf(bfp, "type=%c", desc->baseType);
    if (desc->baseType == TYPE_OREF)
        fprintf(bfp, " class=%s", desc->className);
    if (desc->array)
        fprintf(bfp, " array=%d", desc->array);
    fprintf(bfp, "\n");

    fclose(bfp);
    return buf;
}
#endif

static field_info *readField(Thread *thread, FILE *f, cp_info **cp, uint32_t cp_cnt, ClassFile *cls)
{
    if (cls == NULL)
        errx(EXIT_FAILURE, "cls is null");

    field_info *ret = NULL;
    attribute_info **attrs = NULL;

    if ((ret = calloc(1, sizeof(field_info))) == NULL) return NULL;

    ret->class = cls;

    if (!read2(f, &ret->access_flags)) goto fail;
    if (!read2(f, &ret->name_index)) goto fail;
    if (!read2(f, &ret->descriptor_index)) goto fail;
    if (!read2(f, &ret->attributes_count)) goto fail;

    if (ret->descriptor_index >= cp_cnt || ret->name_index >= cp_cnt) goto fail;
    ret->name = cp[ret->name_index];
    ret->descriptor = cp[ret->descriptor_index];

    if (!parseParamDescriptor(&ret->desc, ret->descriptor->d.utf8.ascii)) {
        warnx("parseParamDescriptor");
        goto fail;
    }

    if ((attrs = calloc(ret->attributes_count+1U, sizeof(attribute_info *))) == NULL) {
        warn("readField");
        goto fail;
    }

    for (uint32_t i = 0; i < ret->attributes_count; i++)
        if ((attrs[i] = readAttribute(thread, i, f, cp, cp_cnt)) == NULL) {
            warnx("readAttribute");
            goto fail;
        }

    ret->attributes = attrs;

    if (ret->access_flags & ACC_STATIC)
        for (uint32_t i = 0; i < ret->attributes_count; i++)
            if (attrs[i] && !strcmp(attrs[i]->name->d.utf8.ascii, "ConstantValue"))
                ret->static_operand = dupOperand(&attrs[i]->d.constant->op);

    return ret;

fail:
    if (ret) {
        if (attrs) {
            for (int i = 0; i < ret->attributes_count; i++) {
                if (attrs[i]) {
                    free(attrs[i]);
                    attrs[i] = NULL;
                }
            }
            free(attrs);
            attrs = NULL;
            ret->attributes = NULL;
        }
        free(ret);
        ret = NULL;
    }
    return NULL;
}


static bool parseMethodDescriptor(des_Method *desc, const char *str)
{
    const char *ptr = str;
    des_Field **ret, **tmp;
    size_t num_fields = 0;

    if ((ret = calloc(1, sizeof(des_Field *))) == NULL) {
        warn("parseMethodDescriptor");
        return false;
    }

    /* FIXME memleak on ret */
    if (*ptr++ != '(') {
        warnx("parseMethodDesc: missing '(' on '%s'", str);
        goto fail;
    }

    desc->num_params = 0;
    while (*ptr && *ptr != ')') {
        if ((ret[num_fields] = calloc(1, sizeof(des_Field))) == NULL)
            goto fail;
        if ((ptr = parseParamDescriptor(ret[num_fields++], ptr)) == NULL)
            goto fail;
        if ((tmp = realloc(ret, sizeof(des_Field *) * (num_fields + 1U))) == NULL) {
            warn("parseMethodDescriptor");
            goto fail;
        }
        desc->num_params++;
        ret = tmp;
    }

    if (!*ptr) {
        warnx("parseMethodDesc: missing ')' on '%s'", str);
        goto fail;
    }

    ptr++;

    if (!parseParamDescriptor(&desc->ret, ptr)) {
        warn("parseMethodDesc: bad return Param");
        goto fail;
    }

    desc->params = ret;

    return true;
fail:
    if (ret)
    {
        for (size_t i = 0; i < num_fields; i++)
        {
            if (!ret[i]) continue;
            free(ret[i]);
            ret[i] = NULL;
        }
        free(ret);
        ret = NULL;
    }
    return false;
}

static method_info *readMethod(Thread *thread, FILE *f, cp_info **cp, uint32_t cp_cnt, ClassFile *class)
{
    method_info *ret = NULL;
    attribute_info **attrs = NULL;

    if ((ret = calloc(1, sizeof(method_info))) == NULL) {
        warn("readMethod: calloc");
        return NULL;
    }
    ret->class = class;

    if (!read2(f, &ret->access_flags)) { warnx("readMethod: access_flags"); goto fail; }
    if (!read2(f, &ret->name_index)) { warnx("readMethod: name_index"); goto fail; }
    if (!read2(f, &ret->descriptor_index)) { warnx("readMethod: descriptor_index"); goto fail; }
    if (!read2(f, &ret->attributes_count)) { warnx("readMethod: attributes_count"); goto fail; }

    if (ret->descriptor_index >= cp_cnt || ret->name_index >= cp_cnt) { 
        warnx("readMethod: desc_index or name_index OOB");
        goto fail;
    }

    ret->name = cp[ret->name_index];
    ret->descriptor = cp[ret->descriptor_index];

    if (!parseMethodDescriptor(&ret->desc, ret->descriptor->d.utf8.ascii)) {
        warnx("readMethod: parseMethodDescriptor fail");
        goto fail;
    }

    if ((attrs = calloc(ret->attributes_count+1U, sizeof(attribute_info *))) == NULL) {
        warn("readMethod: calloc(%d) attrs", ret->attributes_count);
        goto fail;
    }
    
    for (uint32_t i = 0; i < ret->attributes_count; i++) {
        if ((attrs[i] = readAttribute(thread, i, f, cp, cp_cnt)) == NULL) {
            warnx("readMethod: readAttribute failed");
            goto fail;
        }
    }
    
    ret->attributes = attrs;

    for (uint32_t i = 0; i < ret->attributes_count; i++) {
        if (!ret->attributes[i]) continue;
        else if (!strcmp(ret->attributes[i]->name->d.utf8.ascii, "Exceptions"))
            ret->exceptions = ret->attributes[i]->d.exception;
        else if (!strcmp(ret->attributes[i]->name->d.utf8.ascii, "Code"))
            ret->code = ret->attributes[i]->d.code;
    }
    
    return ret;
fail:
    if (attrs) {
        for (size_t i = 0; i < ret->attributes_count; i++) {
            if (attrs[i]) free(attrs[i]);
            attrs[i] = NULL;
        }
        free(attrs);
        attrs = NULL;
    }
    if (ret) free(ret);
    ret = NULL;
    return NULL;
}

static uint16_t *parseCpUtf8ToUtf16(const uint8_t *src, uint16_t len, uint16_t *utf16len)
{
    uint16_t ptr = 0;
    uint16_t dst = 0;
    uint16_t *ret = NULL;
    uint16_t *tmp = NULL;

    if (len == 0) {
        *utf16len = 0;
        return NULL;
    }

    if ((ret = malloc((int)len * sizeof(uint16_t))) == NULL) return NULL;

    while (ptr < len)
    {
        uint8_t val = src[ptr++];

        if (val == 0) {                 // 0000 0000
            warnx("parseCpUtf8ToUtf16: read 0");
            goto fail;
        }
        else if (val < 0x80)            // 0xxx xxxx
        {
            ret[dst] = val;
            dst++;
        }
        else if ((val & 0xe0) == 0xc0)  // 110x xxxx
        {
            ret[dst] = (uint16_t)(64U * (val & 0x1f));
            ret[dst] = (uint16_t)(ret[dst] + (src[ptr++] & 0x3F));
            dst++;
        }
        else if ((val & 0xf0) == 0xe0)  // 1110 xxxx
        {
            ret[dst] = (uint16_t)(4096U * (val & 0x0f));
            ret[dst] = (uint16_t)(ret[dst] + (uint16_t)(64U * (src[ptr++] & 0x3f)));
            ret[dst] = (uint16_t)(ret[dst] + (uint16_t)(src[ptr++] & 0x3f));
            dst++;
        }
    }

    if (dst == 0) goto fail;

    tmp = realloc(ret, dst * sizeof(uint16_t));
    if (!tmp) {
        warn("parseCpUtf8ToUtf17: realloc: %d", dst);
        goto fail;
    }
    ret = tmp;

    *utf16len = dst;

    return ret;
fail:
    if (ret) free(ret);
    return NULL;
}

static char *parseUtf16ToAscii(const uint16_t *src, uint32_t len)
{
    char *ret = NULL;
    uint32_t sptr = 0;
    uint32_t dptr = 0;

    if (len == 0) {
        return NULL;
    }

    if ((ret = malloc(len + 1U)) == NULL)
        return NULL;

    while(sptr < len)
    {
        if (src[sptr] < 0x80)
            ret[dptr++] = (char)(src[sptr]);
        sptr++;
    }

    ret[dptr++] = '\0';

    char *tmp = realloc(ret, dptr);
    if (!tmp) {
        free(ret);
        ret = NULL;
        return NULL;
    }
    ret = tmp;
    //printf("parseUtf16ToAscii: %s [%d]\n", ret, len);
    return ret;
}

static bool fixupCpUtf8(cp_info *ci)
{
    if (ci->d.utf8.utf16 == NULL)
        ci->d.utf8.utf16 = parseCpUtf8ToUtf16(ci->d.utf8.bytes, ci->d.utf8.length, &ci->d.utf8.utf16len);
    if (ci->d.utf8.ascii == NULL)
        ci->d.utf8.ascii = parseUtf16ToAscii(ci->d.utf8.utf16, ci->d.utf8.utf16len);
    return true;
}

/* TODO - do not normalise the name here */
static bool fixUtf8ClassName(cp_info *ci)
{
    if (ci->d.utf8.ascii) {
        if (strchr(ci->d.utf8.ascii, '/') == NULL)
            return true;
#ifdef DEBUG
        //printf("fixUtf8ClassName: %s => ", ci->d.utf8.ascii);
#endif
        free(ci->d.utf8.ascii);
        ci->d.utf8.ascii = NULL;
    }

    if (ci->d.utf8.utf16) {
        free(ci->d.utf8.utf16);
        ci->d.utf8.utf16 = NULL;
    }

    for (uint16_t i = 0; i < ci->d.utf8.length; i++)
    {
        uint8_t val = ci->d.utf8.bytes[i];

        if ((val & 0xe0) == 0xc0) {
            i++;
        } else if ((val & 0xf0) == 0xe0) {
            i++; i++;
        } else if (val == '/') {
            ci->d.utf8.bytes[i] = '.';
        }
    }

    if (!fixupCpUtf8(ci))
        return false;
#ifdef DEBUG
    //printf("%s\n", ci->d.utf8.ascii);
#endif
    return true;
}

static bool fixupCpString(Thread *thread, cp_info *ci, cp_info **pool, uint16_t cp_len)
{
    if (ci->d.str.string && ci->d.str.intern)
        return true;

    if (ci->d.str.string_index >= cp_len)
        return false;
    if (pool[ci->d.str.string_index]->tag != CONSTANT_Utf8)
        return false;

    ci->d.str.string = pool[ci->d.str.string_index];

    return true;
}

static bool linkCpString(Thread *thread, cp_info *ci)
{
    if ((ci->d.str.intern = newArray(thread, TYPE_CHAR, ci->d.str.string->d.utf8.utf16len, NULL)) == NULL)
        return false;

    // lock-- in freeClass
    ((Object *)ci->d.str.intern)->lock++;

    fixupCpUtf8((cp_info *)ci->d.str.string);

    for (uint32_t i = 0; i < ci->d.str.string->d.utf8.utf16len; i++) {
        freeAndNullOperand(&ci->d.str.intern->data.array.data[i]);
        ci->d.str.intern->data.array.data[i] = newOperand(TYPE_CHAR, &ci->d.str.string->d.utf8.utf16[i]);
    }

    return true;
}

static bool fixupCpClass(cp_info *ci, cp_info **pool, uint16_t cp_len)
{
    if (ci->d.class.name)
        return true;

    if (ci->d.class.name_index >= cp_len) return false;
    if (pool[ci->d.class.name_index]->tag != CONSTANT_Utf8) return false;

    ci->d.class.name = pool[ci->d.class.name_index];

    return true;
}

static bool fixupCpFieldref(cp_info *ci, cp_info **pool, uint16_t cp_len)
{
    if (ci->d.fr.class && ci->d.fr.nametype)
        return true;

    if (ci->d.fr.class_index >= cp_len || ci->d.fr.name_and_type_index >= cp_len)
        return false;

    ci->d.fr.class = pool[ci->d.fr.class_index];
    ci->d.fr.nametype = pool[ci->d.fr.name_and_type_index];

    if (ci->d.fr.class->tag != CONSTANT_Class || ci->d.fr.nametype->tag != CONSTANT_NameAndType)
        return false;

    return true;
}

static bool fixupCpNameAndType(cp_info *ci, cp_info **pool, uint16_t cp_len)
{
    if (ci->d.nt.name && ci->d.nt.descriptor) return true;

    if (ci->d.nt.name_index >= cp_len || ci->d.nt.descriptor_index >= cp_len)
        return false;

    ci->d.nt.name = pool[ci->d.nt.name_index];
    ci->d.nt.descriptor = pool[ci->d.nt.descriptor_index];

    if (ci->d.nt.name->tag != CONSTANT_Utf8 || ci->d.nt.descriptor->tag != CONSTANT_Utf8)
        return false;

    return true;
}

static bool fixupConstant(Thread *thread, cp_info *ci, cp_info **pool, uint16_t cp_len)
{
    switch(ci->tag)
    {
        case CONSTANT_NameAndType:
            return fixupCpNameAndType(ci, pool, cp_len);
        case CONSTANT_String:
            return fixupCpString(thread, ci, pool, cp_len);
        case CONSTANT_Class:
            return fixupCpClass(ci, pool, cp_len);
        case CONSTANT_Fieldref:
        case CONSTANT_Methodref:
        case CONSTANT_InterfaceMethodref:
            return fixupCpFieldref(ci, pool, cp_len);
        case CONSTANT_Utf8:
            return fixupCpUtf8(ci);
        default:
            break;
    }
    return true;
}

static bool fixupPostConstant(Thread *thread, cp_info *ci, cp_info **pool, uint16_t cp_len)
{
    bool ret = true;

    switch(ci->tag)
    {
        case CONSTANT_Class:
            
//          if (!ci->d.class.original_name)
//              ci->d.class.original_name = strdup(ci->d.class.name->d.utf8.ascii);

            return fixUtf8ClassName(ci->d.class.name);

        case CONSTANT_NameAndType:
//          if (!ci->d.nt.original_name)
//              ci->d.nt.original_name = strdup(ci->d.nt.name->d.utf8.ascii);
//          if (!ci->d.nt.original_descriptor)
//              ci->d.nt.original_descriptor = strdup(ci->d.nt.descriptor->d.utf8.ascii);

            return fixUtf8ClassName(ci->d.nt.descriptor);

        default:
            break;
    }
    return ret;
}

static Operand *loadInnerClass(Thread *thread, const InnerClasses_attribute *ic, const ClassFile *cf)
{
    if(thread == NULL || ic == NULL || cf == NULL)
        errx(EXIT_FAILURE, "loadInnerClass: shit");

    //char buf[BUFSIZ];
    
    for (int i = 0; i < ic->number_of_classes; i++)
    {
        inner_class_def *def = ic->classes[i];
        const char *clsname = def->inner_ci->d.class.name->d.utf8.ascii;

        if(!def || !strcmp(clsname, cf->this_class->d.class.name->d.utf8.ascii))
            continue;

        if(isClassLoaded(thread, clsname))
            continue;

        //snprintf(buf, sizeof(buf), "%s.class", def->inner_ci->d.class.name->d.utf8.ascii);

        if ((def->inner_class = loadClass(thread, clsname)) == NULL) {
            warnx("loadInnerClass: unable to load %s", clsname);
            return throw(thread, "java.lang.ClassNotFoundException", NULL, 0);
        }
#ifdef DEBUG
        printf("(loadInnerClass: %s)\n", clsname);
#endif
        def->inner_class->isInnerClass = true;
        if (thread->jvm->VM) linkClass(thread, (ClassFile *)def->inner_class);
        //addClass(thread, def->inner_class);
    }

    return NULL;
}

static Field *findFieldByName(const Object *const o, const char *const name)
{
    const field_info *fi;

    for (int i = 0; i < o->data.obj.num_fields; i++)
    {
        if ((fi = (field_info *)o->data.obj.fields[i].field_info) == NULL)
            continue;

        if (strcmp(fi->name->d.utf8.ascii, name)) continue;

        return &o->data.obj.fields[i];
    }

    return NULL;
}

/* not-jvm-safe */
static Operand *newStringFromChar(Thread *thread, const Object *buf,
        Object **result)
{
    assert(thread != NULL);
    assert(buf != NULL);
    assert(result != NULL);
    assert(thread->jvm->VM == true);
    Frame *cur_frame = currentFrame(thread);

    /* validate inputs */
    if (buf == NULL) {
        warnx("newStringFromChar: buf == NULL");
        return throw(thread, "java.lang.NullPointerException", ERR_AT, 0);
    }

    if (buf->type != OBJ_ARRAY || buf->data.array.type != TYPE_CHAR) {
        warnx("newStringFromChar: buf not [C");
        return throw(thread, "java.lang.IllegalArguementException", ERR_AT, 0);
    }

    /* create the String */
    ClassFile *str_cls = findClass(thread, "java.lang.String");
    assert(str_cls != NULL);
    Object *str_obj = newObject(thread, str_cls);
    assert(str_obj != NULL);
    const Operand *str_op = newOperand(TYPE_OREF, str_obj);
    assert(str_op != NULL);
    push(cur_frame, str_op);

    const Operand *arr_op = newOperand(TYPE_AREF, buf);
    assert(arr_op != NULL);
    push(cur_frame, arr_op);

    /* invoke String.init(char []) */
    const method_info *mi = findMethodByClass(thread, str_cls, "<init>", "([C)V");
    assert(mi != NULL);
    Operand *ex_opr = NULL;
    if ((ex_opr = invokeMethod(thread, mi, true, thread->pc)) != NULL) {
        warnx("newStringFromChar: <init> ([C)V threw");
        return ex_opr;
    }

    /* store the result if no exception thrown */
    *result = str_obj;
    return NULL;
}

/* not-jvm-safe */
static Operand *newString(Thread *thread, const char *const text, Object **result)
{
    assert(thread->jvm->VM == true);

    *result = NULL;

    /* create the char[] */
    const Object *const array = newArray(thread, TYPE_CHAR, (int32_t)strlen(text), NULL);

    if (array == NULL) {
        warnx("newString: newArray NULL");
        return throw(thread, "java.lang.RuntimeException", ERR_AT, 0);
    }

    for (uint32_t i = 0; text[i]; i++)
    {
        const uint16_t ch = (uint8_t)text[i];
        freeAndNullOperand(&array->data.array.data[i]);
        array->data.array.data[i] = newOperand(TYPE_CHAR, &ch);
        assert(array->data.array.data[i] != NULL);
    }

    return newStringFromChar(thread, array, result);
}

/* TODO move to JVM? */
static uint32_t num_ary_maps = 0;
static ary_map_t *ary_maps = NULL;
static pthread_rwlock_t clslock;

static Object *createClassForArray(Thread *thread, const uint8_t type, const uint8_t dim, const ClassFile *compClass)
{
    assert(thread->jvm->VM == true);
    int rc;

    char buf[BUFSIZ] = {0};
    char errbuf[BUFSIZ] = {0};

    for(ssize_t i = 0; i < dim; i++)
        strcat(buf, "[");

    buf[dim] = (char)(type);
    buf[dim+1] = '\0';

    if(compClass) {
        strcat(buf, compClass->this_class->d.class.name->d.utf8.ascii);
        strcat(buf, ";");
    }

    if ((rc = pthread_rwlock_tryrdlock(&clslock)) != 0) {
        strerror_r(rc, errbuf, sizeof(errbuf));
        warnx("createClassForArray(%s): pthread_rwlock_rdlock: %s", buf, errbuf);
        return NULL;
    }

    for (ssize_t i = 0; i < num_ary_maps; i++)
        if (ary_maps[i].canonical && !strcmp(buf, ary_maps[i].canonical)) {
            pthread_rwlock_unlock(&clslock);
            return ary_maps[i].object;
        }

    pthread_rwlock_unlock(&clslock);

#ifdef DEBUG
    printf("Creating class for %s\n", buf);
#endif

    ClassFile *class_cf = findClass2(thread, "java.lang.Class", false);
    if (!class_cf) return NULL;

    Frame *cur_frame = currentFrame(thread);

    Object *ret = newObject(thread, class_cf);

    if ((rc = pthread_rwlock_wrlock(&clslock)) != 0) {
        strerror_r(rc, buf, sizeof(buf));
        warnx("createClassForArray: pthread_rwlock_rdlock: %s", buf);
        return NULL;
    }

    ret->lock++; // lock -- freeJVM

    ary_map_t *tmp = realloc(ary_maps, sizeof(ary_map_t) * (num_ary_maps + 1));
    if (tmp == NULL) {
        ret->lock--;
        pthread_rwlock_unlock(&clslock);
        warn("createClassForArray: realloc");
        return NULL;
    }

    ary_maps = tmp;
    ary_maps[num_ary_maps].object = ret;
    ary_maps[num_ary_maps].canonical = strdup(buf);
    num_ary_maps++;

    pthread_rwlock_unlock(&clslock);

    const method_info *mi = findMethodByClass(thread, class_cf, "<init>", "()V");
    if (!push(cur_frame, newOperand(TYPE_OREF, ret))) {
        ret->lock--;
        warnx("createClassForArray: push");
        return NULL;
    }
    if (invokeMethod(thread, mi, true, thread->pc))
        errx(EXIT_FAILURE, "createClassForArray: thrown");

    Field *fld = NULL;
    Object *str = NULL;
    Operand tmpOp;

    if ((fld = findFieldByName(ret, "isArray")) == NULL) {
        ret->lock--;
        warnx("createClassForArray: isArray");
        return NULL;
    }
    freeAndNullOperand(&fld->op);
    tmpOp.val.vbool = true;
    fld->op = newOperand(TYPE_BOOL, &tmpOp.val.vbool);

    if ((fld = findFieldByName(ret, "name")) == NULL)
        errx(EXIT_FAILURE, "createClassForArray: name");
    freeAndNullOperand(&fld->op);

    if ((newString(thread, buf, &str)) != NULL)
        errx(EXIT_FAILURE, "createClassForArray: newString threw");
    fld->op = newOperand(TYPE_OREF, str);

    buf[0] = '\0';

    if (compClass)
        snprintf(buf, BUFSIZ, "%s", compClass->this_class->d.class.name->d.utf8.ascii); // FIXME
    else
        snprintf(buf, BUFSIZ, "%s", printOpType(type));

    for (ssize_t i = 0; i < dim; i++)
        strcat(buf, "[]");

    if ((fld = findFieldByName(ret, "canonicalName")) == NULL)
        errx(EXIT_FAILURE, "createClass: name");
    freeAndNullOperand(&fld->op);

    if ((newString(thread, buf, &str)) != NULL)
        errx(EXIT_FAILURE, "createClassForArray: newString threw");
    fld->op = newOperand(TYPE_OREF, str);

    if ((fld = findFieldByName(ret, "componentType")) == NULL)
        errx(EXIT_FAILURE, "createClassForArray: componentType");

    freeAndNullOperand(&fld->op);

    if (compClass) {
        fld->op = newOperand(TYPE_OREF, getClassObject(thread,
                    (ClassFile *)compClass));
    } else if (thread->jvm->primUp) {
        fld->op = newOperand(TYPE_OREF, findObjForPrim(type));
    } else {
        fld->op = newOperand(TYPE_NULL, 0);
        // FIXME need to go back over things prior to primUp somehow
    }

    // TODO componentType for primitives

    return ret;
}

/* not-jvm-safe */
static Object *createClass(Thread *thread, ClassFile *cf)
{
    assert(thread->jvm->VM == true);

    ClassFile *class_cf = findClass2(thread, "java.lang.Class", false);
    if (!class_cf) return NULL;

    Frame *cur_frame = currentFrame(thread);
    Object *ret = newObject(thread, class_cf);
    // lock-- in freeClass
    ret->lock++;
    method_info *mi = findMethodByClass(thread, class_cf, "<init>", "()V");
    push(cur_frame, newOperand(TYPE_OREF, ret));
    if (invokeMethod(thread, mi, true, thread->pc))
        errx(EXIT_FAILURE, "createClass: thrown");

    Field *fld = NULL;
    Object *str = NULL;

    if ((fld = findFieldByName(ret, "canonicalName")) == NULL)
        errx(EXIT_FAILURE, "createClass: canonicalName");
    if (newString(thread, cf->this_class->d.class.name->d.utf8.ascii, &str))
        errx(EXIT_FAILURE, "createClass: canonicalName newString");

    freeAndNullOperand(&fld->op);
    fld->op = newOperand(TYPE_OREF, str);

    char *simple_name;

    if ((simple_name = strrchr(cf->this_class->d.class.name->d.utf8.ascii, '.')) == NULL)
        simple_name = cf->this_class->d.class.name->d.utf8.ascii;
    else
        simple_name++;

    if ((fld = findFieldByName(ret, "simpleName")) == NULL)
        errx(EXIT_FAILURE, "createClass: simpleName");
    if (newString(thread, simple_name, &str))
        errx(EXIT_FAILURE, "createClass: simpleName newString");
    
    freeAndNullOperand(&fld->op);
    fld->op = newOperand(TYPE_OREF, str);

    if ((fld = findFieldByName(ret, "name")) == NULL)
        errx(EXIT_FAILURE, "createClass: name");
    if (newString(thread, cf->this_class->d.class.name->d.utf8.ascii, &str))
        errx(EXIT_FAILURE, "createClass: name newString");

    freeAndNullOperand(&fld->op);
    fld->op = newOperand(TYPE_OREF, str);

    return ret;
}

static ClassFile *parseClass(Thread *thread, FILE *fp)
{
    ClassFile *ret = calloc(1, sizeof(ClassFile));
    if (ret == NULL) err(EXIT_FAILURE, "parseClass: calloc");

    char buf[BUFSIZ] = {0};

    int rc;
    if ((rc = pthread_rwlock_init(&ret->rwlock, NULL))) {
        strerror_r(rc, buf, sizeof(buf));
        errx(EXIT_FAILURE, "parseClass: pthread_rwlock_init: %s", buf);
    }

    if (!read4(fp, &ret->magic)) errx(EXIT_FAILURE, "parseClass: read magic");

    if (ret->magic != 0xcafebabe)
        errx(EXIT_FAILURE, "Bad magic: 0x%0x", ret->magic);

    if (!read2(fp, &ret->minor_version)) err(EXIT_FAILURE, "parseClass: read minor");
    if (!read2(fp, &ret->major_version)) err(EXIT_FAILURE, "parseClass: read major");

    if (ret->major_version > 55) {
        warnx("Unsupported JRE version: %d.%d",
                ret->major_version, ret->minor_version);
        goto fail;
    }

    if (!read2(fp, &ret->constant_pool_count)) err(EXIT_FAILURE, "parseClass: read cp count");
    if ((ret->constant_pool = calloc(ret->constant_pool_count+2U,
                    sizeof(cp_info *))) == NULL)
        err(EXIT_FAILURE, "parseClass: calloc(%d)", ret->constant_pool_count+2U);

    for(int i = 1; i < ret->constant_pool_count; i++) {
        if ((ret->constant_pool[i] = readConstantPool(fp)) == NULL)
            errx(EXIT_FAILURE, "readConstantPool[%d]", i);
        else if (ret->constant_pool[i]->tag == CONSTANT_Long) i++;
        else if (ret->constant_pool[i]->tag == CONSTANT_Double) i++;
    }

    for(int i = 1; i < ret->constant_pool_count; i++) {
        if (!fixupConstant(thread, (cp_info *)ret->constant_pool[i], ret->constant_pool,
                    ret->constant_pool_count))
            errx(EXIT_FAILURE, "fixupConstantPool[%d]", i);

        if (ret->constant_pool[i]->tag == CONSTANT_Long ||
                ret->constant_pool[i]->tag == CONSTANT_Double) i++;
    }

    if (!read2(fp, &ret->access_flags)) err(EXIT_FAILURE, "parseClass");
    if (!read2(fp, &ret->this_class_idx)) err(EXIT_FAILURE, "parseClass");
    if (!read2(fp, &ret->super_class_idx)) err(EXIT_FAILURE, "parseClass");

    if (ret->this_class_idx > ret->constant_pool_count ||
            ret->super_class_idx > ret->constant_pool_count)
        err(EXIT_FAILURE, "parseClass: this_class or super_class idx too high");

    ret->this_class = ret->constant_pool[ret->this_class_idx];
    ret->super_class = ret->constant_pool[ret->super_class_idx];

    if (!read2(fp, &ret->interfaces_count)) err(EXIT_FAILURE, "parseClass: read");
    if ((ret->interfaces = calloc(ret->interfaces_count+1U,
                    sizeof(uint16_t))) == NULL)
        err(EXIT_FAILURE, "parseClass: calloc(%d) interfaces", ret->interfaces_count+1U);

    for(int i = 0; i < ret->interfaces_count; i++)
        if (!read2(fp, &ret->interfaces[i]))
            err(EXIT_FAILURE, "read interface[%d]", i);

    if (!read2(fp, &ret->fields_count)) err(EXIT_FAILURE, "parseClass: read fields_count");
    if ((ret->fields = calloc(ret->fields_count+1U,
                    sizeof(field_info *))) == NULL)
        err(EXIT_FAILURE, "parseClass: calloc(%d) fields", ret->fields_count+1U);

    for(int i = 0; i< ret->fields_count; i++)
        if ((ret->fields[i] = readField(thread, fp, ret->constant_pool,
                        ret->constant_pool_count, ret)) == NULL)
            errx(EXIT_FAILURE, "readField[%d]", i);

    if (!read2(fp, &ret->methods_count)) err(EXIT_FAILURE, "parseClass");
    if ((ret->methods = calloc(ret->methods_count+1U,
                    sizeof(method_info *))) == NULL)
        err(EXIT_FAILURE, "parseClass");



    for(int i = 0; i < ret->methods_count; i++)
    {
        
        if ((ret->methods[i] = readMethod(thread, fp, ret->constant_pool,
                        ret->constant_pool_count, ret)) == NULL)
            errx(EXIT_FAILURE, "readMethod[%d]", i);
 
//      ret->methods[i]->original_descriptor = strdup(ret->methods[i]->descriptor->d.utf8.ascii);
//      ret->methods[i]->original_name       = strdup(ret->methods[i]->name->d.utf8.ascii);
        
        /* fix class naming style - TODO preserve?*/
        if (strchr(ret->methods[i]->descriptor->d.utf8.ascii, '/') != NULL)
            fixUtf8ClassName(ret->methods[i]->descriptor);

        /* link Code segment */
        for(int j = 0; j < ret->methods[i]->attributes_count; j++)
        {
            if (!strcmp(ret->methods[i]->attributes[j]->name->d.utf8.ascii, "Code"))
                ((Code_attribute *)ret->methods[i]->attributes[j]->d.code)->method = ret->methods[i];
        }
    }

    if (!read2(fp, &ret->attributes_count)) err(EXIT_FAILURE, "parseClass: read attributes_count");
    if ((ret->attributes = calloc(ret->attributes_count+1U,
                    sizeof(attribute_info *))) == NULL)
        err(EXIT_FAILURE, "parseClass: calloc(%d) attribues", ret->attributes_count+1U);

    for(uint32_t i = 0; i < ret->attributes_count; i++)
        if ((ret->attributes[i] = readAttribute(thread, i, fp,
                        ret->constant_pool,
                        ret->constant_pool_count)) == NULL)
            errx(EXIT_FAILURE, "readAttribute[%d]", i);
    
//  ret->original_name = strdup(ret->this_class->d.class.name->d.utf8.ascii);

    for(int i = 1; i < ret->constant_pool_count; i++) {
        if (!fixupPostConstant(thread, (cp_info *)ret->constant_pool[i], ret->constant_pool,
                    ret->constant_pool_count))
            errx(EXIT_FAILURE, "fixupPostConstantPool[%d]", i);

        if (ret->constant_pool[i]->tag == CONSTANT_Long ||
                ret->constant_pool[i]->tag == CONSTANT_Double) i++;
    }

    if (ret->super_class)
        if ((ret->super_name = strdup(ret->super_class->d.class.name->d.utf8.ascii)) == NULL)
            err(EXIT_FAILURE, "parseClass: strdup(super_class name)");

    /* TODO preserve the on-disk format elsewhere, making this human readable */
//  ret->canonical_name = strdup(ret->this_class->d.class.name->d.utf8.ascii);

    if ((ret->interface_class = calloc(ret->interfaces_count+1U,
                    sizeof(cp_info *))) == NULL)
        errx(EXIT_FAILURE, "parseClass: calloc(%d) interface_class", ret->interfaces_count+1U);

    for(uint32_t i = 0; i < ret->interfaces_count; i++)
        ret->interface_class[i] = ret->constant_pool[ret->interfaces[i]];

    if ((rc = pthread_rwlock_wrlock(&ret->rwlock))) {
        strerror_r(rc, buf, sizeof(buf));
        errx(EXIT_FAILURE, "parseClass: pthread_rwlock_lock: %s", buf);
    }

    if (!addClass(thread, ret)) {
        warnx("parseClass: unable to addClass");
        return NULL;
    }

    for(uint32_t i = 0; i < ret->attributes_count; i++)
        if (!strcmp(ret->attributes[i]->name->d.utf8.ascii, "InnerClasses")) {
            loadInnerClass(thread, ret->attributes[i]->d.inner, ret);
        } else if (!strcmp(ret->attributes[i]->name->d.utf8.ascii, "SourceFile")) {
            ret->srcfile = ret->attributes[i]->d.srcfile->name;
        }

    ret->isLinked = false;
    pthread_rwlock_unlock(&ret->rwlock);
    return ret;
fail:
    if (ret) {
        pthread_rwlock_unlock(&ret->rwlock);
        pthread_rwlock_destroy(&ret->rwlock);
        free(ret);
        ret = NULL;
    }
    return NULL;
}

static bool linkClass(Thread *thread, ClassFile *ret)
{
    assert(thread->jvm->VM == true);

    if (ret->isLinked) return true;
    ret->isLinked = true;

    for (ssize_t i = 1; i < ret->constant_pool_count; i++)
    {
        if (ret->constant_pool[i]) switch(ret->constant_pool[i]->tag)
        {
            case CONSTANT_String:
                linkCpString(thread, (cp_info *)ret->constant_pool[i]);
                break;
            default:
                break;
        }
    }

    return true;
}

static Operand *throw(Thread *thread, const char *name, const char *text,
        int64_t pc)
{
    assert(thread->jvm->VM == true);

    Frame *cur_frame = currentFrame(thread);
    while(cur_frame->sp)
        freeOperand(pop(cur_frame));

    Operand *ex_opr = NULL;
    ClassFile *thrcls = findClass(thread, name);

    if (thrcls == NULL) {
        if(!strcmp(name, "java.lang.RuntimeException"))
            errx(EXIT_FAILURE,
                    "Cannot find java.lang.RuntimeException in throw");
        else
            return throw(thread, "java.lang.RuntimeException", name, pc);
    }

    Object *thr = newObject(thread, thrcls);
    if (!thr) errx(EXIT_FAILURE, "throw: unable to newObject");

    Operand *tmpOp;

    if (!push(cur_frame, tmpOp = newOperand(TYPE_OREF, thr))) {
        freeAndNullOperand(&tmpOp);
        errx(EXIT_FAILURE, "tUhrow: unable to push");
    }

    if (text) {
        Object *str = NULL;
        if ((ex_opr = newString(thread, text, &str)) != NULL) {
            warnx("throw unable to newString");
            return ex_opr;
        }

        if(!push(cur_frame, tmpOp = newOperand(TYPE_OREF,str))) {
            freeAndNullOperand(&tmpOp);
            errx(EXIT_FAILURE, "throw: unable to push string");
        }
    }

    method_info *mi = findMethodByClass(thread, thrcls,
            "<init>", text ? "(Ljava.lang.String;)V" : "()V");
    if ((ex_opr = invokeMethod(thread, mi, true, pc)) != NULL)
        return ex_opr;

    if(!push(cur_frame, newOperand(TYPE_OREF, thr)))
        errx(EXIT_FAILURE, "throw: unable to init");
    mi = findMethodByClass(thread, thrcls,
            "fillInStackTrace", "()Ljava.lang.Throwable;");
    if ((ex_opr = invokeMethod(thread, mi, true, pc)) != NULL)
        return ex_opr;

    freeOperand(pop(cur_frame));

    return newOperand(TYPE_OREF, thr);
}

static bool isClassLoaded(Thread *thread, const char *clsname)
{
    JVM *jvm = (JVM *)thread->jvm;

#ifdef DEBUG
    printf("(isClassLoaded: %s)\n", clsname);
#endif

    if (pthread_rwlock_rdlock(&jvm->rwlock))
        errx(EXIT_FAILURE, "isClassLoaded: pthread_rwlock_rdlock");

    for (ssize_t i = 0; i < MAX_METHOD; i++)
    {
        if (!jvm->method_area[i])
            continue;
        if (!strcmp(jvm->method_area[i]->this_class->d.class.name->d.utf8.ascii, clsname)) {
            pthread_rwlock_unlock(&jvm->rwlock);
            return true;
        }
    }

    pthread_rwlock_unlock(&jvm->rwlock);
    return false;
}

/* setting load to false prevents recursive hell */
static ClassFile *findClass2(Thread *const thread, const char *const clsname, const bool load)
{
    ClassFile *ret = NULL;
    JVM *const jvm = (JVM *)thread->jvm;
    int rc;
    char buf[BUFSIZ];

    if (!jvm) return NULL;

    if ((rc = pthread_rwlock_rdlock(&jvm->rwlock)) != 0) {
        strerror_r(rc, buf, sizeof(buf));
        errx(EXIT_FAILURE, "findClass: pthread_rwlock_rdlock: %s", buf);
    }

    for (uint16_t i = 0; i < MAX_METHOD; i++)
    {
        if (!jvm->method_area[i])
            continue;
        if (!strcmp(jvm->method_area[i]->this_class->d.class.name->d.utf8.ascii,
                    clsname)) {
            ret = jvm->method_area[i];
            break;
        }
    }
    if (pthread_rwlock_unlock(&jvm->rwlock))
        errx(EXIT_FAILURE, "findClass: pthread_rwlock_unlock");

    if (ret && !ret->isLinked) {
        warnx("findClass2: refusing to return %s as not linked", clsname);
        return NULL;
    }

    if (!ret && load) {
        ClassFile *tmp = NULL;
        //snprintf(buf, sizeof(buf), "%s.class", clsname);
        tmp = loadClass(thread, clsname);
        if (!tmp) return NULL;
        if (thread->jvm->VM && !linkClass(thread, tmp)) return NULL;
        ret = tmp;
    }
    return ret;
}

static bool canCast(Thread *thread, const char *from, const char *to)
{
    /* null can always be cast to any Object? */
    if (from == NULL)
        return true;

    if (!strcmp(from, to)) return true;
    if (!strcmp("java.lang.Object", to)) return true; /* this is a bit shit FIXME */

    ClassFile *fromCls = findClass(thread, from);
    ClassFile *toCls = findClass(thread, to);

    if(!fromCls || !toCls) return false;

    if(*from == '[') {
        if (*to == '[') {
            // same primitive type, or canCast() on the type
        } else if(toCls->access_flags & ACC_INTERFACE) {
            // must be one of CLoneable & Serializable i think?? jls 4.10.3
        } else {
            return false;
        }
    } else if (fromCls->access_flags & ACC_INTERFACE) {
        if (toCls->access_flags & ACC_INTERFACE) {
            // toCls must match fromCls or a superinterface
        } else {
            return false;
        }
    } else {
        if (toCls->access_flags & ACC_INTERFACE) {
            // fromCls must implement toCls
        } else {
            // fromCls must inherit from toCls
        }
    }

    return true;
}

/* not-jvm-safe */
static ClassFile *findClass(Thread *const thread, const char *const clsname)
{
    assert(thread->jvm->VM == true);
    return findClass2(thread, clsname, true);
}

static method_info *findMethodByClass(Thread *thread, const ClassFile *cls, const char *mthname, const char *desc)
{
    if (cls == NULL)
        cls = findClass(thread, "java.lang.Object");

    for (ssize_t i = 0; i < cls->methods_count; i++)
    {
        //printf("check %s == %s\n", desc, cls->methods[i]->descriptor->d.utf8.ascii);
        if (!strcmp(cls->methods[i]->name->d.utf8.ascii, mthname) &&
                !strcmp(desc, cls->methods[i]->descriptor->d.utf8.ascii)) {
            return cls->methods[i];
        }
    }

    if (cls->interfaces_count > 0) {
        method_info *mi = NULL;
        for (ssize_t i = 0; i < cls->interfaces_count; i++) {
            if ((mi = findClassMethod(thread,
                            cls->interface_class[i]->d.class.name->d.utf8.ascii, mthname,
                            desc)) != NULL)
                return mi;
        }
    } 
    
    if (cls->super_name)
        return findClassMethod(thread, cls->super_name, mthname, desc);
    else
        return NULL;
}

static method_info *findClassMethod(Thread *thread, const char *clsname, const char *mthname, const char *desc)
{
    assert(thread->jvm->VM == true);
    ClassFile *cls = findClass(thread, clsname);
    if (cls == NULL) return NULL;

    return findMethodByClass(thread, cls, mthname, desc);
}

static field_info *findClassField(Thread *thread, const char *clsname, const char *fldname)
{
    assert(thread->jvm->VM == true);
    ClassFile *cls = findClass(thread, clsname);
    if (cls == NULL) {
        warnx("findClassField: no such class %s", clsname);
        return NULL;
    }

    return findClassFieldForClass(thread, cls, fldname);
}

static field_info *findClassFieldForClass(Thread *thread, ClassFile *cls, const char *fldname)
{
    for (ssize_t i = 0; i < cls->fields_count; i++)
        if (!strcmp(cls->fields[i]->name->d.utf8.ascii, fldname))
            return cls->fields[i];

    if(cls->super_class)
        return findClassField(thread, cls->super_name, fldname);
    return NULL;
}

static attribute_info *findAttribute(attribute_info **attrs, uint16_t num, const char *name)
{
    for(uint16_t i = 0; i < num; i++)
        if (!strcmp(attrs[i]->name->d.utf8.ascii, name))
            return (attrs[i]);
    return NULL;
}

static Operand *classInit(Thread *thread, ClassFile *cls)
{
    assert(thread->jvm->VM == true);

    char buf[BUFSIZ] = {0};
    int rc;
    method_info *mi = NULL;
    Operand *ret = NULL;
    Operand *ex_opr = NULL;

    if (!cls || cls->isInit) // FIXME lock the class first
        return NULL;

    if ((rc = pthread_rwlock_wrlock(&cls->rwlock))) {
        strerror_r(rc, buf, sizeof(buf));
        warnx("classInit: pthread_rwlock_wrlock(class): %s", buf);
        return NULL; // FIXME error?
    }

    if (cls->isInit)
        goto done;

    cls->isInit = true;

    /* we can't create String Objects during class load, as things may not be ready */
    for (ssize_t i = 0; i < cls->fields_count; i++) {
        if (!cls->fields[i] || cls->fields[i]->desc.baseType != TYPE_OREF)
            continue;

        for (ssize_t j = 0; j < cls->fields[i]->attributes_count; j++)
            if (cls->fields[i]->attributes[j] && !strcmp("ConstantValue",
                        cls->fields[i]->attributes[j]->name->d.utf8.ascii))
            {
                attribute_info *ai = cls->fields[i]->attributes[j];

                if (ai->d.constant->op.type == TYPE_OREF) {
                    field_info *fi = cls->fields[i];

                    if (ai->d.constant->op.val.vref == NULL) {
                        Object *tmp = NULL;
                        cp_String_info *si = &ai->d.constant->entry->d.str;
                        if ((ex_opr = newStringFromChar(thread, si->intern,
                                        &tmp)) != NULL) {
                            warnx("classInit unable to newString");
                            return ex_opr;
                        }
                        if (tmp)
                            tmp->lock++; // FIXME lock--
                        ((ConstantValue_attribute *)ai->d.constant)->op.val.vref = tmp;
                    }

                    if ( (fi->access_flags & ACC_STATIC) &&
                            fi->static_operand == NULL) {
                        ((field_info *)fi)->static_operand = dupOperand(&ai->d.constant->op);
                    }
                }
            }
    }

done:

    if ((mi = findMethodByClass(thread, cls, "<clinit>", "()V")) != NULL)
        ret = invokeMethod(thread, mi, false, thread->pc);
    else
        ret = NULL;

    pthread_rwlock_unlock(&cls->rwlock);

    return ret;
}

/* FIXME we should return throw() not NULL */
static Operand *getFieldInfo(Thread *thread, ClassFile *cls, const uint16_t which, field_info **fld)
{
    assert(thread->jvm->VM == true);
    Operand *ret = NULL;

    if (which >= cls->constant_pool_count) {
        warnx("getFieldInfo: Invalid which");
        return NULL;
    }

    if (cls->constant_pool[which]->tag != CONSTANT_Fieldref) {
        warnx("getFieldInfo: Not a field ref");
        return NULL;
    }

    cp_info *cp = cls->constant_pool[which];
    field_info *fi = findClassField(thread,
            cp->d.fr.class->d.class.name->d.utf8.ascii,
            cp->d.fr.nametype->d.nt.name->d.utf8.ascii);

    if (fi && !fi->class->isInit)
        if ((ret = classInit(thread, (ClassFile *)fi->class)) != NULL)
            return ret;

    if (!fi) {
        warnx("getFieldInfo: unable to find field for %s %s",
                cp->d.fr.class->d.class.name->d.utf8.ascii,
                cp->d.fr.nametype->d.nt.name->d.utf8.ascii);
    }

    *fld = fi;

    return NULL;
}

static uint16_t readShort(uint8_t *code, int64_t *pc)
{
    uint16_t ret = (uint16_t)(256U * code[(*pc)+1] + code[(*pc)+2]);
    *pc += 2;
    return ret;
}

static int32_t readInt(uint8_t *code, int64_t *pc)
{
    int32_t ret = (int32_t)(16777216U * code[(*pc)] + 65536U * code[(*pc)+1] + 256U * code[(*pc)+2] + code[(*pc)+3]);
    *pc += 4;
    return ret;
}

static int16_t readBranch(uint8_t *code, int64_t *pc, const int16_t off)
{
    int16_t bra = (int16_t)(256U * code[(*pc)+1] + code[(*pc)+2]);
    *pc += 2;
    return (int16_t)(bra - 3);
}

static int32_t readBranchWide(uint8_t *code, int64_t *pc, const int16_t off)
{
    int32_t bra = (int32_t)(16777216U * code[(*pc)] + 65536U * code[(*pc)+1] + 256U * code[(*pc)+2] + code[(*pc)+3]);
    *pc += 2;
    return (int32_t)(bra - 5);
}

static Operand *ldc(const int idx, Frame *cur_frame, const ClassFile *cls, Thread *thread, const int64_t pc)
{
    cp_info *cp = cls->constant_pool[idx];
    Operand *ex_opr = NULL;

    switch(cp->tag)
    {
        case CONSTANT_String:
            {
                ClassFile *strcls;
                Object *o = newObject(thread,
                        strcls = findClass(thread,
                            "java.lang.String"));

                if(!push(cur_frame, newOperand(TYPE_OREF, o)))
                    return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                if(!push(cur_frame, newOperand(TYPE_AREF, cp->d.str.intern)))
                    return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

                method_info *meth;
                meth = findMethodByClass(thread, strcls, "<init>", "([C)V");

                if (meth == NULL)
                    return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

                if((ex_opr = invokeMethod(thread, meth, true, pc)) != NULL)
                    return ex_opr;

                if(!push(cur_frame, newOperand(TYPE_OREF, o)))
                    return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
            }
            break;
        case CONSTANT_Class:
            {
#ifdef DEBUG
                printf("Class %s", cp->d.class.name->d.utf8.ascii);
#endif
                ClassFile *cls = findClass(thread, cp->d.class.name->d.utf8.ascii);
                if (!cls || !getClassObject(thread, (ClassFile *)cls))
                    return throw(thread, "java.lang.ClassNotFoundException", ERR_AT, pc);

                if (!push(cur_frame, newOperand(TYPE_OREF, getClassObject(thread, (ClassFile *)cls))))
                    return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
            }
            break;
        case CONSTANT_Float:
#ifdef DEBUG
            printf("float %d", (int)cp->d.tfloat.c_float); // FIXME printf
#endif
            if(!push(cur_frame, newOperand(TYPE_FLOAT, &cp->d.tfloat.c_float)))
                return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
            break;
        case CONSTANT_Integer:
            if(!push(cur_frame, newOperand(TYPE_INT, &cp->d.tint.c_int)))
                return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
            break;
        case CONSTANT_Long:
            if(!push(cur_frame, newOperand(TYPE_LONG, &cp->d.tlong.c_long)))
                return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
            break;
        default:
            errx(EXIT_FAILURE, "ldc: unknown constant pool tag: %d", cp->tag);
    }
    return NULL;
}

static Operand *runCode(Thread *thread, Code_attribute *attr, const int32_t pc_mod, int64_t *pcstore)
{
    int64_t pc      = 0 + pc_mod;
    uint16_t tmpw   = 0;
    uint8_t tmp     = 0;
    Operand *ex_opr = NULL;
    bool running    = true;
    bool is_wide    = false;

    assert(thread != NULL);
    assert(attr != NULL);

    uint8_t *code   = attr->code;
    ClassFile *cls  = thread->cur_method->class;

    assert(code != NULL);
    assert(cls != NULL);

    Operand tmpOp;

    if (!cls->isInit) {
        Operand *ret = classInit(thread, (ClassFile *)cls);
        if (ret != NULL) return ret;
    }


    while(running)
    {
        if (pc < 0 || pc >= attr->code_length) {
            warnx("runCode: %ld: invalid program counter", pc);
            return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
        }

        Frame *cur_frame = currentFrame(thread);
        if (cur_frame == NULL) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

        const uint8_t opc = code[pc];
        if (pcstore) *pcstore = pc;
        cur_frame->pc = (uint32_t)(pc);

#ifdef DEBUG_CODE
        printf("%03ld: [%02x] ", pc, opc);
#endif

        switch(opc)
        {
            case 0x00: /* nop */
                break;
            case 0x01: /* aconst_null */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " aconst_null\n" ANSI_RESET);
#endif
                    push(cur_frame, newOperand(TYPE_NULL, NULL));
                }
                break;
            case 0x02: /* iconst_m1 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " iconst_m1\n" ANSI_RESET);
#endif
                    tmpOp.val.vint = -1;
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                }
                break;
            case 0x03: /* iconst 0 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " iconst_0\n" ANSI_RESET);
#endif
                    tmpOp.val.vint = 0;
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                }
                break;
            case 0x04: /* iconst 1 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" iconst_1\n" ANSI_RESET);
#endif
                    tmpOp.val.vint = 1;
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                }
                break;
            case 0x05: /* iconst 2 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" iconst_2\n" ANSI_RESET);
#endif
                    tmpOp.val.vint = 2;
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                }
                break;
            case 0x06: /* iconst 3 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" iconst_3\n" ANSI_RESET);
#endif
                    tmpOp.val.vint = 3;
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                }
                break;
            case 0x07: /* iconst 4 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" iconst_4\n" ANSI_RESET);
#endif
                    tmpOp.val.vint = 4;
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                }
                break;
            case 0x08: /* iconst 5 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" iconst_5\n" ANSI_RESET);
#endif
                    tmpOp.val.vint = 5;
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                }
                break;
            case 0x09: /* lconst 0 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " lconst_0\n" ANSI_RESET);
#endif
                    tmpOp.val.vlong = 0;
                    push(cur_frame, newOperand(TYPE_LONG, &tmpOp.val.vlong));
                }
                break;
            case 0x0a: /* lconst 1 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " lconst_1\n" ANSI_RESET);
#endif
                    tmpOp.val.vlong = 1;
                    push(cur_frame, newOperand(TYPE_LONG, &tmpOp.val.vlong));
                }
                break;
            case 0x0b: /* fconst_0 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " fconst_0 " ANSI_RESET "\n");
#endif
                    tmpOp.val.vfloat = 0.0f;
                    push(cur_frame, newOperand(TYPE_FLOAT, &tmpOp.val.vfloat));
                }
                break;
            case 0x0c: /* fconst_1 */
                {
#ifdef  DEBUG_CODE
                    printf(ANSI_INSTR " fconst_1 " ANSI_RESET "\n");
#endif
                    tmpOp.val.vfloat = 1.0f;
                    push(cur_frame, newOperand(TYPE_FLOAT, &tmpOp.val.vfloat));
                }
                break;
            case 0x0d: /* fconst_2 */
                {
#ifdef  DEBUG_CODE
                    printf(ANSI_INSTR " fconst_2 " ANSI_RESET "\n");
#endif
                    tmpOp.val.vfloat = 2.0f;
                    push(cur_frame, newOperand(TYPE_FLOAT, &tmpOp.val.vfloat));
                }
                break;
            case 0x0e: /* dconst_0 */
                {
#ifdef  DEBUG_CODE
                    printf(ANSI_INSTR " dconst_0 " ANSI_RESET "\n");
#endif
                    tmpOp.val.vdouble = 0.0f;
                    push(cur_frame, newOperand(TYPE_DOUBLE, &tmpOp.val.vdouble));
                }
                break;
            case 0x0f: /* dconst_1 */
                {
#ifdef  DEBUG_CODE
                    printf(ANSI_INSTR " dconst_1 " ANSI_RESET "\n");
#endif
                    tmpOp.val.vdouble = 1.0f;
                    push(cur_frame, newOperand(TYPE_DOUBLE, &tmpOp.val.vdouble));
                }
                break;
            case 0x10: /* bipush */
                {
                    tmpOp.val.vint = code[++pc];
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " bipush " ANSI_RESET "%d\n", tmpOp.val.vint);
#endif
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                }
                break;
            case 0x11: /* sipush */
                {
                    tmpw = readShort(code, &pc);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " sipush " ANSI_RESET " %d\n", tmpw);
#endif
                    tmpOp.val.vint = tmpw;
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                }
                break;
            case 0x12: /* ldc */
                {
                    tmp = code[++pc];
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " ldc " ANSI_RESET "#%d ", tmp);
#endif
                    Operand *ex_opr = ldc(tmp, cur_frame, cls, thread, pc);
                    if (ex_opr) return ex_opr;
#ifdef DEBUG_CODE
                    printf("\n");
#endif
                }
                break;
            case 0x13: /* ldc_w */
                {
                    tmpw = readShort(code, &pc);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR "ldc_w " ANSI_RESET "%d\n", tmpw);
#endif
                    Operand *ex_opr = ldc(tmpw, cur_frame, cls, thread, pc);
                    if (ex_opr) return ex_opr;
#ifdef DEBUG_CODE
                    printf("\n");
#endif
                }
                break;
            case 0x14: /* ldc2_w */
                {
                    tmpw = readShort(code, &pc);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " ldc2_w " ANSI_RESET "%d\n", tmpw);
#endif
                    cp_info *cp = cls->constant_pool[tmpw];

                    switch(cp->tag)
                    {
                        case CONSTANT_Long:
                            if(!push(cur_frame, newOperand(TYPE_LONG, &cp->d.tlong.c_long)))
                                return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                            break;
                        case CONSTANT_Double:
                            if(!push(cur_frame, newOperand(TYPE_DOUBLE, &cp->d.tdouble.c_double)))
                                return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                            break;
                        default:
                            errx(EXIT_FAILURE, "ldc2_w: unknown constant pool tag: %d", cp->tag);
                    }
                }
                break;
            case 0x15: /* iload */
                {
                    tmp = code[++pc];
                    if (is_wide) {
                        tmp *= 256;
                        tmp += code[++pc];
                        is_wide = 0;
                    }
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " iload " ANSI_RESET "%d\n", tmp);
#endif
                    if(!push(cur_frame, dupOperand(getLocal(cur_frame, tmp))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x16: /* lload */
                {
                    tmp = code[++pc];
                    if (is_wide) {
                        tmp *= 256;
                        tmp += code[++pc];
                        is_wide = 0;
                    }
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " lload " ANSI_RESET "%d\n", tmp);
#endif
                    if(!push(cur_frame, dupOperand(getLocal(cur_frame, tmp))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x17: /* fload */
                {
                    tmp = code[++pc];
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " fload " ANSI_RESET "%d\n", tmp);
#endif
                    if(!push(cur_frame, dupOperand(getLocal(cur_frame, tmp))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x18: /* dload */
                {
                    tmp = code[++pc];
                    if (is_wide) {
                        tmp *= 256;
                        tmp += code[++pc];
                        is_wide = 0;
                    }
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " dload " ANSI_RESET "%d\n", tmp);
#endif
                    if(!push(cur_frame, dupOperand(getLocal(cur_frame, tmp))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x19: /* aload */
                {
                    tmp = code[++pc];
                    if (is_wide) {
                        tmp *= 256;
                        tmp += code[++pc];
                        is_wide = 0;
                    }
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " aload " ANSI_RESET "%d\n", tmp);
#endif
                    if(!push(cur_frame, dupOperand(getLocal(cur_frame, tmp))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x1a: /* iload_0 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " iload_0\n" ANSI_RESET);
#endif
                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,0))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x1b: /* iload_1 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" iload_1\n" ANSI_RESET);
#endif
                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,1))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x1c: /* iload_2 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" iload_2\n" ANSI_RESET);
#endif
                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,2))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x1d: /* iload_3 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" iload_3 " ANSI_RESET "%s %s\n",
                            printOpType(cur_frame->local[3]->type),
                            printOpValue(cur_frame->local[3]));
#endif
                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,3))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x1e: /* lload_0 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " lload_0 " ANSI_RESET "%s %s\n",
                            printOpType(cur_frame->local[0]->type),
                            printOpValue(cur_frame->local[0]));
#endif
                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,0))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x1f: /* lload_1 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " lload_1 " ANSI_RESET "%s %s\n",
                            printOpType(cur_frame->local[1]->type),
                            printOpValue(cur_frame->local[1]));
#endif
                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,1))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x20: /* lload_2 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " lload_2 " ANSI_RESET "%s %s\n",
                            printOpType(cur_frame->local[2]->type),
                            printOpValue(cur_frame->local[2]));
#endif
                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,2))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x21: /* lload_3 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " lload_3 " ANSI_RESET "%s %s\n",
                            printOpType(cur_frame->local[3]->type),
                            printOpValue(cur_frame->local[3]));
#endif
                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,3))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x22: /* fload_0 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " fload_0 " ANSI_RESET "%s %s\n",
                            printOpType(cur_frame->local[0]->type),
                            printOpValue(cur_frame->local[0]));
#endif
                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,0))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x23: /* fload_1 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " fload_1 " ANSI_RESET "%s %s\n",
                            printOpType(cur_frame->local[1]->type),
                            printOpValue(cur_frame->local[1]));
#endif
                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,1))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x24: /* fload_2 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " fload_2 " ANSI_RESET "%s %s\n",
                            printOpType(cur_frame->local[2]->type),
                            printOpValue(cur_frame->local[2]));
#endif
                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,2))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x25: /* fload_3 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " fload_3 " ANSI_RESET "%s %s\n",
                            printOpType(cur_frame->local[3]->type),
                            printOpValue(cur_frame->local[3]));
#endif
                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,3))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x26: /* dload_0 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " dload_0 " ANSI_RESET "%s %s\n",
                            printOpType(cur_frame->local[0]->type),
                            printOpValue(cur_frame->local[0]));
#endif
                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,0))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x27: /* dload_1 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " dload_1 " ANSI_RESET "%s %s\n",
                            printOpType(cur_frame->local[1]->type),
                            printOpValue(cur_frame->local[1]));
#endif
                    Operand *o = getLocal(cur_frame,1);
                    if(o == NULL || o->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
                    if(!push(cur_frame, dupOperand(o)))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x28: /* dload_2 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " dload_2 " ANSI_RESET "%s %s\n",
                            printOpType(cur_frame->local[2]->type),
                            printOpValue(cur_frame->local[2]));
#endif
                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,2))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x29: /* dload_3 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " dload_3 " ANSI_RESET "%s %s\n",
                            printOpType(cur_frame->local[3]->type),
                            printOpValue(cur_frame->local[3]));
#endif
                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,3))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x2a: /* aload_0 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " aload_0 " ANSI_RESET " %s %s\n",
                            printOpType(cur_frame->local[0]->type),
                            printOpValue(cur_frame->local[0]));
#endif
                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,0))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x2b: /* aload_1 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " aload_1" ANSI_RESET " %s %s\n",
                            printOpType(cur_frame->local[1]->type),
                            printOpValue(cur_frame->local[1]));
#endif
                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,1))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x2c: /* aload_2 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " aload_2" ANSI_RESET " %s %s\n",
                            printOpType(cur_frame->local[2]->type),
                            printOpValue(cur_frame->local[2]));
#endif
                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,2))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x2d: /* aload_3 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" aload_3" ANSI_RESET " %s %s\n",
                            printOpType(cur_frame->local[3]->type),
                            printOpValue(cur_frame->local[3]));
#endif
                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,3))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x2e: /* iaload */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " iaload\n" ANSI_RESET);
#endif
                    Operand *index = pop(cur_frame);
                    Operand *array = pop(cur_frame);

                    if (index->type == TYPE_NULL || array->type == TYPE_NULL) {
                        freeOperand(index);
                        freeOperand(array);
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
                    }

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->data.array.len) {
                        freeOperand(index);
                        freeOperand(array);
                        return throw(thread, "java.lang.IndexOutOfBoundsException", NULL, pc);
                    }

                    if (array->val.vref->data.array.data[index->val.vint])
                        push(cur_frame, dupOperand(array->val.vref->data.array.data[index->val.vint]));
                    else {
                        tmpOp.val.vint = 0;
                        push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                    }

                    freeOperand(array);
                    freeOperand(index);
                }
                break;
            case 0x2f: /* laload */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " laload\n" ANSI_RESET);
#endif
                    Operand *index = pop(cur_frame);
                    Operand *array = pop(cur_frame);

                    if (index->type == TYPE_NULL || array->type == TYPE_NULL) {
                        freeOperand(array);
                        freeOperand(index);
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
                    }

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->data.array.len) {
                        freeOperand(array);
                        freeOperand(index);
                        return throw(thread, "java.lang.IndexOutOfBoundsException", NULL, pc);
                    }

                    if (array->val.vref->data.array.data[index->val.vint])
                        push(cur_frame, dupOperand(array->val.vref->data.array.data[index->val.vint]));
                    else {
                        tmpOp.val.vlong = 0;
                        push(cur_frame, newOperand(TYPE_LONG, &tmpOp.val.vlong));
                    }

                    freeOperand(array);
                    freeOperand(index);
                }
                break;
            case 0x30: /* faload */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " faload\n" ANSI_RESET);
#endif
                    Operand *index = pop(cur_frame);
                    Operand *array = pop(cur_frame);

                    if (index->type == TYPE_NULL || array->type == TYPE_NULL) {
                        freeOperand(array);
                        freeOperand(index);
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
                    }

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->data.array.len) {
                        freeOperand(array);
                        freeOperand(index);
                        return throw(thread, "java.lang.IndexOutOfBoundsException", NULL, pc);
                    }

                    if (array->val.vref->data.array.data[index->val.vint])
                        push(cur_frame, dupOperand(array->val.vref->data.array.data[index->val.vint]));
                    else {
                        tmpOp.val.vfloat = 0.0f;
                        push(cur_frame, newOperand(TYPE_FLOAT, &tmpOp.val.vfloat));
                    }

                    freeOperand(array);
                    freeOperand(index);
                }
                break;
            case 0x31: /* daload */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " daload\n" ANSI_RESET);
#endif
                    Operand *index = pop(cur_frame);
                    Operand *array = pop(cur_frame);

                    if (index->type == TYPE_NULL || array->type == TYPE_NULL) {
                        freeOperand(array);
                        freeOperand(index);
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
                    }

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->data.array.len) {
                        freeOperand(array);
                        freeOperand(index);
                        return throw(thread, "java.lang.IndexOutOfBoundsException", NULL, pc);
                    }

                    if (array->val.vref->data.array.data[index->val.vint])
                        push(cur_frame, dupOperand(array->val.vref->data.array.data[index->val.vint]));
                    else {
                        tmpOp.val.vdouble = 0.0f;
                        push(cur_frame, newOperand(TYPE_DOUBLE, &tmpOp.val.vdouble));
                    }

                    freeOperand(array);
                    freeOperand(index);
                }
                break;
            case 0x32: /* aaload */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " aaload\n" ANSI_RESET);
#endif
                    Operand *index = pop(cur_frame);
                    Operand *array = pop(cur_frame);

                    if (index->type == TYPE_NULL || array->type == TYPE_NULL) {
                        freeOperand(array);
                        freeOperand(index);
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
                    }

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->data.array.len) {
                        freeOperand(array);
                        freeOperand(index);
                        return throw(thread, "java.lang.IndexOutOfBoundsException", NULL, pc);
                    }

                    if (array->val.vref->data.array.data[index->val.vint])
                        push(cur_frame, dupOperand(array->val.vref->data.array.data[index->val.vint]));
                    else
                        push(cur_frame, newOperand(TYPE_NULL, NULL));

                    freeOperand(array);
                    freeOperand(index);
                }
                break;
            case 0x33: /* baload */
                {
                    Operand *index = pop(cur_frame);
                    Operand *array = pop(cur_frame);

#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " baload\n" ANSI_RESET);
#endif
                    if (index->type == TYPE_NULL || array->type == TYPE_NULL) {
                        freeOperand(array);
                        freeOperand(index);
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
                    }

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->data.array.len) {
                        freeOperand(array);
                        freeOperand(index);
                        return throw(thread, "java.lang.IndexOutOfBoundsException", NULL, pc);
                    }

                    if (array->val.vref->data.array.data[index->val.vint]) {
                        if (array->val.vref->data.array.type == TYPE_BOOL)
                            tmpOp.val.vint = array->val.vref->data.array.data[index->val.vint]->val.vbool;
                        else
                            tmpOp.val.vint = array->val.vref->data.array.data[index->val.vint]->val.vbyte;

                        push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                    } else {
                        tmpOp.val.vint = 0;
                        push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                    }

                    freeOperand(array);
                    freeOperand(index);
                }
                break;
            case 0x34: /* caload */
                {
                    Operand *index = pop(cur_frame);
                    Operand *array = pop(cur_frame);

                    if (index->type == TYPE_NULL || array->type == TYPE_NULL) {
                        freeOperand(index);
                        freeOperand(array);
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
                    }
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " caload" ANSI_RESET " [%d] %s\n", index->val.vint,
                            printOpType(array->val.vref->data.array.type));
#endif
                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->data.array.len) {
                        freeOperand(index);
                        freeOperand(array);
                        return throw(thread, "java.lang.IndexOutOfBoundsException", NULL, pc);
                    }

                    if (array->val.vref->data.array.data[index->val.vint]) {
                        tmpOp.val.vint = array->val.vref->data.array.data[index->val.vint]->val.vchar;
                        push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                    } else {
                        tmpOp.val.vint = 0;
                        push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                    }

                    freeOperand(index);
                    freeOperand(array);
                }
                break;
            case 0x35: /* saload */
                {
                    Operand *index = pop(cur_frame);
                    Operand *array = pop(cur_frame);

                    if (index->type == TYPE_NULL || array->type == TYPE_NULL) {
                        freeOperand(index);
                        freeOperand(array);
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
                    }
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " saload" ANSI_RESET " [%d] %s\n", index->val.vint,
                            printOpType(array->val.vref->data.array.type));
#endif
                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->data.array.len) {
                        freeOperand(index);
                        freeOperand(array);
                        return throw(thread, "java.lang.IndexOutOfBoundsException", ERR_AT, pc);
                    }

                    if (array->val.vref->data.array.data[index->val.vint]) {
                        tmpOp.val.vint = array->val.vref->data.array.data[index->val.vint]->val.vshort;
                        push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                    } else {
                        tmpOp.val.vint = 0;
                        push(cur_frame, newOperand(TYPE_SHORT, &tmpOp.val.vint));
                    }

                    freeOperand(index);
                    freeOperand(array);
                }
                break;
            case 0x36: /* istore */
                {
                    tmp = code[++pc];
                    if (is_wide) {
                        tmp *= 256;
                        tmp += code[++pc];
                        is_wide = 0;
                    }
                    Operand *val = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " istore "ANSI_RESET" %s.%s => %d\n",
                            printOpType(val->type),
                            printOpValue(val),
                            tmp);
#endif
                    if(setLocal(cur_frame, tmp, val) == NULL) {
                        freeOperand(val);
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    }
                }
                break;
            case 0x37: /* lstore */
                {
                    tmp = code[++pc];
                    if (is_wide) {
                        tmp *= 256;
                        tmp += code[++pc];
                        is_wide = 0;
                    }
                    Operand *val = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " lstore "ANSI_RESET" %s.%s => %d\n",
                            printOpType(val->type),
                            printOpValue(val),
                            tmp);
#endif
                    if(setLocal(cur_frame, tmp, val) == NULL) {
                        freeOperand(val);
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    }
                }
                break;
            case 0x38: /* fstore */
                {
                    tmp = code[++pc];
                    if (is_wide) {
                        tmp *= 256;
                        tmp += code[++pc];
                        is_wide = 0;
                    }
                    Operand *val = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " fstore "ANSI_RESET"%s.%s => %d\n",
                            printOpType(val->type),
                            printOpValue(val),
                            tmp);
#endif
                    if(setLocal(cur_frame, tmp, val) == NULL) {
                        freeOperand(val);
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    }
                }
                break;
            case 0x39: /* dstore */
                {
                    tmp = code[++pc];
                    if (is_wide) {
                        tmp *= 256;
                        tmp += code[++pc];
                        is_wide = 0;
                    }
                    Operand *val = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " dstore "ANSI_RESET"%s.%s => %d\n",
                            printOpType(val->type),
                            printOpValue(val),
                            tmp);
#endif
                    if(setLocal(cur_frame, tmp, val) == NULL) {
                        freeOperand(val);
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    }
                }
                break;
            case 0x3a: /* astore */
                {
                    tmp = code[++pc];
                    if (is_wide) {
                        tmp *= 256;
                        tmp += code[++pc];
                        is_wide = 0;
                    }
                    Operand *val = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " astore "ANSI_RESET"%s.%s => %d\n",
                            printOpType(val->type),
                            printOpValue(val),
                            tmp);
#endif
                    if(setLocal(cur_frame, tmp, val) == NULL) {
                        freeOperand(val);
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    }
                }
                break;
            case 0x3b: /* istore_0 */
                {
                    Operand *a = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" istore_0 "  ANSI_RESET);
                    printf("<= %s %s\n", printOpType(a->type), printOpValue(a));
#endif
                    if(setLocal(cur_frame,0,a) == NULL) {
                        freeOperand(a);
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    }
                }
                break;
            case 0x3c: /* istore_1*/
                {
                    Operand *a = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " istore_1 "  ANSI_RESET);
                    printf("<= %s %s\n", printOpType(a->type), printOpValue(a));
#endif
                    if(setLocal(cur_frame,1,a) == NULL) {
                        freeOperand(a);
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    }
                }
                break;
            case 0x3d: /* istore_2*/
                {
                    Operand *a = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" istore_2 " ANSI_RESET);
                    printf("<= %s %s\n", printOpType(a->type), printOpValue(a));
#endif
                    if(setLocal(cur_frame,2,a) == NULL) {
                        freeOperand(a);
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    }
                }
                break;
            case 0x3e: /* istore_3*/
                {
                    Operand *a = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" istore_3 " ANSI_RESET);
                    printf("<= %s %s\n", printOpType(a->type), printOpValue(a));
#endif
                    if(setLocal(cur_frame,3,a) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;

            case 0x3f: /* lstore_0*/
                {
                    Operand *a = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" lstore_0 " ANSI_RESET);
                    printf("<= %s %s\n", printOpType(a->type), printOpValue(a));
#endif
                    if(setLocal(cur_frame,0,a) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x40: /* lstore_1*/
                {
                    Operand *a = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" lstore_1 " ANSI_RESET);
                    printf("<= %s %s\n", printOpType(a->type), printOpValue(a));
#endif
                    if(setLocal(cur_frame,1,a) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x41: /* lstore_2*/
                {
                    Operand *a = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" lstore_2 " ANSI_RESET);
                    printf("<= %s %s\n", printOpType(a->type), printOpValue(a));
#endif
                    if(setLocal(cur_frame,2,a) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x42: /* lstore_3*/
                {
                    Operand *a = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" lstore_3 " ANSI_RESET);
                    printf("<= %s %s\n", printOpType(a->type), printOpValue(a));
#endif
                    if(setLocal(cur_frame,3,a) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;

            case 0x43: /* fstore_0*/
                {
                    Operand *a = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" fstore_0 " ANSI_RESET);
                    printf("<= %s %s\n", printOpType(a->type), printOpValue(a));
#endif
                    if(setLocal(cur_frame,0,a) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x44: /* fstore_1*/
                {
                    Operand *a = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" fstore_1 " ANSI_RESET);
                    printf("<= %s %s\n", printOpType(a->type), printOpValue(a));
#endif
                    if(setLocal(cur_frame,1,a) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x45: /* fstore_2*/
                {
                    Operand *a = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" fstore_2 " ANSI_RESET);
                    printf("<= %s %s\n", printOpType(a->type), printOpValue(a));
#endif
                    if(setLocal(cur_frame,2,a) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x46: /* fstore_3*/
                {
                    Operand *a = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" fstore_3 " ANSI_RESET);
                    printf("<= %s %s\n", printOpType(a->type), printOpValue(a));
#endif
                    if(setLocal(cur_frame,3,a) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;

            case 0x47: /* dstore_0 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR "dstore_0\n" ANSI_RESET);
#endif
                    if(setLocal(cur_frame,0,pop(cur_frame)) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x48: /* dstore_1 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR "dstore_1\n" ANSI_RESET);
#endif
                    if(setLocal(cur_frame,1,pop(cur_frame)) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;

            case 0x49: /* dstore_2 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR "dstore_2\n" ANSI_RESET);
#endif
                    if(setLocal(cur_frame,2,pop(cur_frame)) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;

            case 0x4a: /* dstore_3 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR "dstore_3\n" ANSI_RESET);
#endif
                    if(setLocal(cur_frame,3,pop(cur_frame)) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x4b: /* astore_0 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" astore_0\n" ANSI_RESET);
#endif
                    if(setLocal(cur_frame,0,pop(cur_frame)) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x4c: /* astore_1 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" astore_1\n" ANSI_RESET);
#endif
                    if(setLocal(cur_frame,1,pop(cur_frame)) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x4d: /* astore_2 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" astore_2\n" ANSI_RESET);
#endif
                    if(setLocal(cur_frame,2,pop(cur_frame)) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x4e: /* astore_3 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" astore_3\n" ANSI_RESET);
#endif
                    if(setLocal(cur_frame,3,pop(cur_frame)) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x4f: /* iastore */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" iastore\n" ANSI_RESET);
#endif
                    Operand *value = pop(cur_frame);
                    Operand *index = pop(cur_frame);
                    Operand *array = pop(cur_frame);

                    if (index->type == TYPE_NULL || array->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->data.array.len)
                        return throw(thread, "java.lang.IndexOutOfBoundsException", ERR_AT, pc);

                    Operand *cur = array->val.vref->data.array.data[index->val.vint];
                    if (cur)
                        freeOperand(cur);

                    array->val.vref->data.array.data[index->val.vint] = value;

                    freeOperand(index);
                    freeOperand(array);
                }
                break;

            case 0x50: /* lastore */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" lastore\n" ANSI_RESET);
#endif
                    Operand *index = pop(cur_frame);
                    Operand *array = pop(cur_frame);

                    if (index->type == TYPE_NULL || array->type == TYPE_NULL) {
                        freeOperand(index); freeOperand(array);
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
                    }

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->data.array.len) {
                        freeOperand(index); freeOperand(array);
                        return throw(thread, "java.lang.IndexOutOfBoundsException", ERR_AT, pc);
                    }

                    Operand *value = pop(cur_frame);

                    if (value->type != array->type) {
                        freeOperand(index); freeOperand(array); freeOperand(value);
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    }

                    Operand *cur = array->val.vref->data.array.data[index->val.vint];
                    if (cur)
                        freeOperand(cur);

                    array->val.vref->data.array.data[index->val.vint] = value;

                    freeOperand(index);
                    freeOperand(array);
                }
                break;

            case 0x51: /* fastore */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" fastore\n" ANSI_RESET);
#endif
                    Operand *value = pop(cur_frame);
                    Operand *index = pop(cur_frame);
                    Operand *array = pop(cur_frame);

                    if (index->type == TYPE_NULL || array->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->data.array.len)
                        return throw(thread, "java.lang.IndexOutOfBoundsException", ERR_AT, pc);

                    Operand *cur = array->val.vref->data.array.data[index->val.vint];
                    if (cur)
                        freeOperand(cur);

                    array->val.vref->data.array.data[index->val.vint] = value;

                    freeOperand(index);
                    freeOperand(array);
                }
                break;

            case 0x52: /* dastore */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" dastore\n" ANSI_RESET);
#endif
                    Operand *value = pop(cur_frame);
                    Operand *index = pop(cur_frame);
                    Operand *array = pop(cur_frame);

                    if (index->type == TYPE_NULL || array->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->data.array.len)
                        return throw(thread, "java.lang.IndexOutOfBoundsException", ERR_AT, pc);

                    Operand *cur = array->val.vref->data.array.data[index->val.vint];
                    if (cur)
                        freeOperand(cur);

                    array->val.vref->data.array.data[index->val.vint] = value;

                    freeOperand(index);
                    freeOperand(array);
                }
                break;

            case 0x53: /* aastore */
                {
                    Operand *value = pop(cur_frame);
                    Operand *index = pop(cur_frame);
                    Operand *array = pop(cur_frame);

                    if (index->type == TYPE_NULL || array->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " aastore " ANSI_RESET "[%d]\n",
                            index->val.vint);
#endif
                    if (value->type != TYPE_NULL && array->val.vref->data.array.type != value->type) {
                        warnx("aastore: %d != %d", value->type, array->val.vref->data.array.type);
                        warnx("aastore: invalid type");
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    }

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->data.array.len)
                        return throw(thread, "java.lang.IndexOutOfBoundsException", ERR_AT, pc);

                    if (array->val.vref->data.array.data[index->val.vint])
                        freeOperand(array->val.vref->data.array.data[index->val.vint]);
                    array->val.vref->data.array.data[index->val.vint] = dupOperand(value);

                    freeOperand(array);
                    freeOperand(index);
                    freeOperand(value);
                }
                break;

            case 0x54: /* bastore */
                {
                    Operand *value = pop(cur_frame);
                    Operand *index = pop(cur_frame);
                    Operand *array = pop(cur_frame);

                    if (index->type == TYPE_NULL || array->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " bastore " ANSI_RESET "%s %s => [%d][%s]\n",
                            printOpType(value->type),
                            printOpValue(value),
                            index->val.vint,
                            printOpType(array->val.vref->data.array.type));
#endif
                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->data.array.len)
                        return throw(thread, "java.lang.IndexOutOfBoundsException", ERR_AT, pc);

                    if (value->type != TYPE_INT || (array->val.vref->data.array.type != TYPE_BYTE
                                && array->val.vref->data.array.type != TYPE_BOOL) )
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

                    tmpOp.val.vbyte = (int8_t)value->val.vint;

                    if (array->val.vref->data.array.data[index->val.vint])
                        freeOperand(array->val.vref->data.array.data[index->val.vint]);
                    array->val.vref->data.array.data[index->val.vint] =
                        newOperand(TYPE_BYTE, &tmpOp.val.vbyte);

                    freeOperand(array);
                    freeOperand(index);
                    freeOperand(value);
                }
                break;
            case 0x55: /* castore */
                {
                    Operand *value = pop(cur_frame);
                    Operand *index = pop(cur_frame);
                    Operand *array = pop(cur_frame);

                    if (index->type == TYPE_NULL || array->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " castore " ANSI_RESET "%s %s => [%d][%s]\n",
                            printOpType(value->type),
                            printOpValue(value),
                            index->val.vint,
                            printOpType(array->val.vref->data.array.type));
#endif
                    if (value->type != TYPE_INT || array->val.vref->data.array.type != TYPE_CHAR)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->data.array.len)
                        return throw(thread, "java.lang.IndexOutOfBoundsException", ERR_AT, pc);

                    tmpOp.val.vchar = (uint16_t)value->val.vint;

                    if (array->val.vref->data.array.data[index->val.vint])
                        freeOperand(array->val.vref->data.array.data[index->val.vint]);
                    array->val.vref->data.array.data[index->val.vint] = newOperand(TYPE_CHAR, &tmpOp.val.vchar);
                    freeOperand(array);
                    freeOperand(index);
                    freeOperand(value);
                }
                break;
            case 0x56: /* sastore */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" sastore\n" ANSI_RESET);
#endif
                    Operand *value = pop(cur_frame);
                    Operand *index = pop(cur_frame);
                    Operand *array = pop(cur_frame);

                    if (index->type == TYPE_NULL || array->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->data.array.len)
                        return throw(thread, "java.lang.IndexOutOfBoundsException", ERR_AT, pc);

                    Operand *cur = array->val.vref->data.array.data[index->val.vint];
                    if (cur)
                        freeOperand(cur);

                    array->val.vref->data.array.data[index->val.vint] = value;

                    freeOperand(index);
                    freeOperand(array);
                }
                break;

            case 0x57: /* pop */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " pop\n" ANSI_RESET);
#endif
                    Operand *op = pop(cur_frame);
                    freeOperand(op);
                }
                break;

            case 0x58: /* pop2 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " pop\n" ANSI_RESET);
#endif
                    Operand *op;
                    
                    for(int i = 0; i < 2; i++) {
                        op = pop(cur_frame);
                        freeOperand(op);
                    }
                }
                break;

            case 0x59: /* dup */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR" dup\n" ANSI_RESET);
#endif
                    Operand *cur = cur_frame->stack[cur_frame->sp-1];
                    Operand *new = dupOperand(cur);
                    push(cur_frame, new);
                }
                break;

            case 0x5a: /* dup_x1 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " dup_x1\n" ANSI_RESET);
#endif
                    Operand *val1 = pop(cur_frame);
                    Operand *val2 = pop(cur_frame);
                    push(cur_frame, dupOperand(val1));
                    push(cur_frame, val2);
                    push(cur_frame, val1);
                }
                break;

            case 0x5c: /* dup2 */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " dup2\n" ANSI_RESET);
#endif
                    Operand *val1 = pop(cur_frame);
                    if (val1->type == TYPE_DOUBLE || val1->type == TYPE_LONG) {
                        push(cur_frame, dupOperand(val1));
                        push(cur_frame, val1);
                    } else {
                        Operand *val2 = pop(cur_frame);
                        push(cur_frame, dupOperand(val2));
                        push(cur_frame, dupOperand(val1));
                        push(cur_frame, val2);
                        push(cur_frame, val1);
                    }
                }
                break;

            case 0x5f: /* swap */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " swap\n" ANSI_RESET);
#endif

                    Operand *val0 = pop(cur_frame);
                    Operand *val1 = pop(cur_frame);
                    push(cur_frame, val0);
                    push(cur_frame, val1);
                }
                break;
            case 0x60: /* iadd */
                {
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);

                    tmpOp.val.vint = one->val.vint + two->val.vint;
                    Operand *res = newOperand(TYPE_INT, &tmpOp.val.vint);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " iadd " ANSI_RESET "%d+%d=%d\n",
                            one->val.vint,
                            two->val.vint,
                            res->val.vint);
#endif
                    push(cur_frame, res);
                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0x61: /* ladd */
                {
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);

                    tmpOp.val.vlong = one->val.vlong + two->val.vlong;
                    Operand *res = newOperand(TYPE_LONG, &tmpOp.val.vlong);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " ladd " ANSI_RESET "%ld+%ld=%ld\n",
                            one->val.vlong,
                            two->val.vlong,
                            res->val.vlong);
#endif
                    push(cur_frame, res);
                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0x62: /* fadd */
                {
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);

                    tmpOp.val.vfloat = one->val.vfloat + two->val.vfloat;
                    Operand *res = newOperand(TYPE_FLOAT, &tmpOp.val.vfloat);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " fadd " ANSI_RESET "%f+%f=%f\n",
                            one->val.vfloat,
                            two->val.vfloat,
                            res->val.vfloat);
#endif
                    push(cur_frame, res);
                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0x63: /* dadd */
                {
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);

                    tmpOp.val.vdouble = one->val.vdouble + two->val.vdouble;
                    Operand *res = newOperand(TYPE_DOUBLE, &tmpOp.val.vdouble);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " dadd " ANSI_RESET "%f+%f=%f\n",
                            one->val.vdouble,
                            two->val.vdouble,
                            res->val.vdouble);
#endif
                    push(cur_frame, res);
                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0x64: /* isub */
                {
                    Operand *one = pop(cur_frame);
                    Operand *two = pop(cur_frame);

                    tmpOp.val.vint = two->val.vint - one->val.vint;
                    Operand *res = newOperand(TYPE_INT, &tmpOp.val.vint);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " isub " ANSI_RESET "%d-%d=%d\n",
                            two->val.vint,
                            one->val.vint,
                            res->val.vint);
#endif
                    push(cur_frame, res);
                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0x65: /* lsub */
                {
                    Operand *one = pop(cur_frame);
                    Operand *two = pop(cur_frame);

                    tmpOp.val.vlong = two->val.vlong - one->val.vlong;
                    Operand *res = newOperand(TYPE_LONG, &tmpOp.val.vlong);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " lsub " ANSI_RESET "%ld-%ld=%ld\n",
                            two->val.vlong,
                            one->val.vlong,
                            res->val.vlong);
#endif
                    push(cur_frame, res);
                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0x66: /* fsub */
                {
                    Operand *one = pop(cur_frame);
                    Operand *two = pop(cur_frame);

                    tmpOp.val.vfloat = two->val.vfloat - one->val.vfloat;
                    Operand *res = newOperand(TYPE_FLOAT, &tmpOp.val.vfloat);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " fsub " ANSI_RESET "%f-%f=%f\n",
                            two->val.vfloat,
                            one->val.vfloat,
                            res->val.vfloat);
#endif
                    push(cur_frame, res);
                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0x67: /* dsub */
                {
                    Operand *one = pop(cur_frame);
                    Operand *two = pop(cur_frame);

                    tmpOp.val.vdouble = two->val.vdouble - one->val.vdouble;
                    Operand *res = newOperand(TYPE_DOUBLE, &tmpOp.val.vdouble);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " dsub " ANSI_RESET "%f-%f=%f\n",
                            two->val.vdouble,
                            one->val.vdouble,
                            res->val.vdouble);
#endif
                    push(cur_frame, res);
                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0x68: /* imul */
                {
                    Operand *one = pop(cur_frame);
                    Operand *two = pop(cur_frame);

                    tmpOp.val.vint = one->val.vint * two->val.vint;
                    Operand *res = newOperand(TYPE_INT, &tmpOp.val.vint);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " imul " ANSI_RESET "%d*%d=%d\n",
                            one->val.vint,
                            two->val.vint,
                            res->val.vint);
#endif
                    push(cur_frame, res);
                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0x69: /* lmul */
                {
                    Operand *one = pop(cur_frame);
                    Operand *two = pop(cur_frame);

                    tmpOp.val.vlong = one->val.vlong * two->val.vlong;
                    Operand *res = newOperand(TYPE_LONG, &tmpOp.val.vlong);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " lmul " ANSI_RESET "%ld*%ld=%ld\n",
                            one->val.vlong,
                            two->val.vlong,
                            res->val.vlong);
#endif
                    push(cur_frame, res);
                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0x6a: /* fmul */
                {
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " fmul" ANSI_RESET "\n");
#endif
                    tmpOp.val.vfloat = one->val.vfloat * two->val.vfloat;
                    push(cur_frame, newOperand(TYPE_FLOAT, &tmpOp.val.vfloat));

                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0x6b: /* dmul */
                {
                    Operand *one = pop(cur_frame);
                    Operand *two = pop(cur_frame);

                    tmpOp.val.vdouble = one->val.vdouble * two->val.vdouble;
                    Operand *res = newOperand(TYPE_DOUBLE, &tmpOp.val.vdouble);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " dmul " ANSI_RESET "%f*%f=%f\n",
                            one->val.vdouble,
                            two->val.vdouble,
                            res->val.vdouble);
#endif
                    push(cur_frame, res);
                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0x6c: /* idiv */
                {
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);
                    tmpOp.val.vint = one->val.vint / two->val.vint;
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " idiv" ANSI_RESET " %d/%d=%d\n",
                            one->val.vint, two->val.vint,
                            tmpOp.val.vint);
#endif
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0x6d: /* ldiv */
                {
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);
                    tmpOp.val.vlong = one->val.vlong / two->val.vlong;
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " ldiv" ANSI_RESET " %ld/%ld=%ld\n",
                            one->val.vlong, two->val.vlong,
                            tmpOp.val.vlong);
#endif
                    push(cur_frame, newOperand(TYPE_LONG, &tmpOp.val.vlong));
                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0x6e: /* fdiv */
                {
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);
                    tmpOp.val.vfloat = one->val.vfloat / two->val.vfloat;
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " fdiv" ANSI_RESET " %f/%f=%f\n",
                            one->val.vfloat, two->val.vfloat,
                            tmpOp.val.vfloat);
#endif
                    push(cur_frame, newOperand(TYPE_LONG, &tmpOp.val.vfloat));
                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0x6f: /* ddiv */
                {
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);
                    tmpOp.val.vdouble = one->val.vdouble / two->val.vdouble;
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " ddiv" ANSI_RESET " %f/%f=%f\n",
                            one->val.vdouble, two->val.vdouble,
                            tmpOp.val.vdouble);
#endif
                    push(cur_frame, newOperand(TYPE_LONG, &tmpOp.val.vdouble));
                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0x70: /* irem */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " irem\n" ANSI_RESET);
#endif
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);

                    int32_t value1 = one->val.vint;
                    int32_t value2 = two->val.vint;

                    if (value2 == 0) {
                        return throw(thread, "java.lang.ArithmeticException", NULL, pc);
                    }

                    tmpOp.val.vint = value1 - (value1 / value2) * value2;

                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0x71: /* lrem */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " lrem\n" ANSI_RESET);
#endif
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);

                    int64_t value1 = one->val.vlong;
                    int64_t value2 = two->val.vlong;

                    if (value2 == 0) {
                        return throw(thread, "java.lang.ArithmeticException", NULL, pc);
                    }

                    tmpOp.val.vlong = value1 - (value1 / value2) * value2;

                    push(cur_frame, newOperand(TYPE_LONG, &tmpOp.val.vlong));
                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0x72: /* frem */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " frem\n" ANSI_RESET);
#endif
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);

                    float value1 = one->val.vfloat;
                    float value2 = two->val.vfloat;

                    if (fpclassify(value2) == FP_ZERO) {
                        return throw(thread, "java.lang.ArithmeticException", NULL, pc);
                    }

                    tmpOp.val.vfloat = fmodf(value1, value2);

                    push(cur_frame, newOperand(TYPE_FLOAT, &tmpOp.val.vlong));
                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0x73: /* drem */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " drem\n" ANSI_RESET);
#endif
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);

                    double value1 = one->val.vdouble;
                    double value2 = two->val.vdouble;

                    if (fpclassify(value2) == FP_ZERO) {
                        return throw(thread, "java.lang.ArithmeticException", NULL, pc);
                    }

                    tmpOp.val.vdouble = fmod(value1, value2);

                    push(cur_frame, newOperand(TYPE_DOUBLE, &tmpOp.val.vlong));
                    freeOperand(one);
                    freeOperand(two);
                }
                break;

            case 0x74: /* ineg */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR "ineg\n" ANSI_RESET);
#endif
                    Operand *one = pop(cur_frame);
                    tmpOp.val.vint = -one->val.vint;
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                    freeOperand(one);
                }
                break;
            case 0x75: /* lneg */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR "lneg\n" ANSI_RESET);
#endif
                    Operand *one = pop(cur_frame);
                    tmpOp.val.vlong = -one->val.vlong;
                    push(cur_frame, newOperand(TYPE_LONG, &tmpOp.val.vlong));
                    freeOperand(one);
                }
                break;
            case 0x76: /* fneg */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR "fneg\n" ANSI_RESET);
#endif
                    Operand *one = pop(cur_frame);
                    tmpOp.val.vfloat = -one->val.vfloat;
                    push(cur_frame, newOperand(TYPE_FLOAT, &tmpOp.val.vfloat));
                    freeOperand(one);
                }
                break;
            case 0x77: /* dneg */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR "dneg\n" ANSI_RESET);
#endif
                    Operand *one = pop(cur_frame);
                    tmpOp.val.vdouble = -one->val.vdouble;
                    push(cur_frame, newOperand(TYPE_DOUBLE, &tmpOp.val.vdouble));
                    freeOperand(one);
                }
                break;
            case 0x78: /* ishl */
                {
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " ishl\n" ANSI_RESET);
#endif
                    tmpOp.val.vint = one->val.vint << (two->val.vint & 0x1F);
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0x79: /* lshl */
                {
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " lshl\n" ANSI_RESET);
#endif
                    tmpOp.val.vlong = one->val.vlong << (two->val.vint & 0x3F);
                    push(cur_frame, newOperand(TYPE_LONG, &tmpOp.val.vlong));
                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0x7a: /* ishr */
                {
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " ishr\n" ANSI_RESET);
#endif
                    tmpOp.val.vint = one->val.vint >> (two->val.vint & 0x1F);
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));

                    freeOperand(one);
                    freeOperand(two);
                }
                break;

            case 0x7b: /* lshr */
                {
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " lshr\n" ANSI_RESET);
#endif
                    tmpOp.val.vlong = one->val.vlong >> (two->val.vint & 0x3F);
                    push(cur_frame, newOperand(TYPE_LONG, &tmpOp.val.vlong));

                    freeOperand(one);
                    freeOperand(two);
                }
                break;

            case 0x7c: /* iushr */
                {
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " iushr\n" ANSI_RESET);
#endif
                    tmpOp.val.vint = ((uint32_t)one->val.vint) >> (two->val.vint & 0x1F);
                    freeOperand(one);
                    freeOperand(two);

                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                }
                break;

            case 0x7d: /* lushr */
                {
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " lushr\n" ANSI_RESET);
#endif
                    tmpOp.val.vlong = ((uint64_t)one->val.vlong) >> (two->val.vint & 0x3F);
                    freeOperand(one);
                    freeOperand(two);

                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));

                }
                break;


            case 0x7e: /* iand */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " iand\n" ANSI_RESET);
#endif
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);

                    tmpOp.val.vint = one->val.vint & two->val.vint;
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));

                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0x7f: /* land */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " land\n" ANSI_RESET);
#endif
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);

                    tmpOp.val.vlong = one->val.vlong & two->val.vlong;
                    push(cur_frame, newOperand(TYPE_LONG, &tmpOp.val.vlong));

                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0x80: /* ior */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " ior\n" ANSI_RESET);
#endif
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);

                    tmpOp.val.vint = one->val.vint | two->val.vint;
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));

                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0x81: /* lor */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " lor\n" ANSI_RESET);
#endif
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);

                    tmpOp.val.vlong = one->val.vlong | two->val.vlong;
                    push(cur_frame, newOperand(TYPE_LONG, &tmpOp.val.vlong));

                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0x82: /* ixor */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " ixor\n" ANSI_RESET);
#endif
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);

                    tmpOp.val.vint = one->val.vint ^ two->val.vint;
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));

                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0x83: /* lxor */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " lxor\n" ANSI_RESET);
#endif
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);

                    tmpOp.val.vlong = one->val.vlong ^ two->val.vlong;
                    push(cur_frame, newOperand(TYPE_LONG, &tmpOp.val.vlong));

                    freeOperand(one);
                    freeOperand(two);
                }
                break;

            case 0x84: /* iinc */
                {
                    tmp = code[++pc];
                    if (is_wide) {
                        tmp *= 256;
                        tmp += code[++pc];
                    }
                    int16_t tmp2 = (int8_t)(code[++pc]);
                    if (is_wide) {
                        tmp2 *= 256;
                        tmp2 += code[++pc];
                        is_wide = false;
                    }
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " iinc " ANSI_RESET "#%d,%d\n", tmp, tmp2);
#endif
                    cur_frame->local[tmp]->val.vint += tmp2;
                }
                break;
            case 0x85: /* i2l */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " i2l\n" ANSI_RESET);
#endif
                    Operand *value = pop(cur_frame);
                    tmpOp.val.vlong = value->val.vint;
                    push(cur_frame, newOperand(TYPE_LONG, &tmpOp.val.vlong));
                    freeOperand(value);
                }
                break;
            case 0x86: /* i2f */
                {
                    Operand *value = pop(cur_frame);
                    tmpOp.val.vfloat = (float)(value->val.vint);
                    push(cur_frame, newOperand(TYPE_FLOAT, &tmpOp.val.vfloat));
                    freeOperand(value);
                }
                break;
            case 0x87: /* i2d */
                {
                    Operand *value = pop(cur_frame);
                    tmpOp.val.vdouble = (float)(value->val.vint);
                    push(cur_frame, newOperand(TYPE_DOUBLE, &tmpOp.val.vdouble));
                    freeOperand(value);
                }
                break;
            case 0x88: /* l2i */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " l2i " ANSI_RESET "\n");
#endif
                    Operand *value = pop(cur_frame);
                    tmpOp.val.vint = (int32_t)value->val.vlong;
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                    freeOperand(value);
                }
                break;
            case 0x89: /* l2f */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " l2f " ANSI_RESET "\n");
#endif
                    Operand *value = pop(cur_frame);
                    tmpOp.val.vfloat = (float)value->val.vlong;
                    push(cur_frame, newOperand(TYPE_FLOAT, &tmpOp.val.vdouble));
                    freeOperand(value);
                }
                break;
            case 0x8a: /* l2d */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " l2d " ANSI_RESET "\n");
#endif
                    Operand *value = pop(cur_frame);
                    tmpOp.val.vdouble = (double)value->val.vlong;
                    push(cur_frame, newOperand(TYPE_DOUBLE, &tmpOp.val.vdouble));
                    freeOperand(value);
                }
                break;
            case 0x8b: /* f2i */
                {
                    Operand *value = pop(cur_frame);
                    tmpOp.val.vint = (int32_t)value->val.vfloat;
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                    freeOperand(value);
                }
                break;
            case 0x8c: /* f2l */
                {
                    Operand *value = pop(cur_frame);
                    tmpOp.val.vint = (int64_t)value->val.vfloat;
                    push(cur_frame, newOperand(TYPE_LONG, &tmpOp.val.vlong));
                    freeOperand(value);
                }
                break;
            case 0x8d: /* f2d */
                {
                    Operand *value = pop(cur_frame);
                    tmpOp.val.vdouble = (double)value->val.vfloat;
                    push(cur_frame, newOperand(TYPE_DOUBLE, &tmpOp.val.vdouble));
                    freeOperand(value);
                }
                break;
            case 0x8e: /* d2i */
                {
                    Operand *value = pop(cur_frame);
                    tmpOp.val.vint = (int32_t)value->val.vdouble;
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                    freeOperand(value);
                }
                break;
            case 0x8f: /* d2l */
                {
                    Operand *value = pop(cur_frame);
                    tmpOp.val.vlong = (long)value->val.vdouble;
                    push(cur_frame, newOperand(TYPE_LONG, &tmpOp.val.vlong));
                    freeOperand(value);
                }
                break;
            case 0x90: /* d2f */
                {
                    Operand *value = pop(cur_frame);
                    tmpOp.val.vfloat = (float)value->val.vdouble;
                    push(cur_frame, newOperand(TYPE_FLOAT, &tmpOp.val.vfloat));
                    freeOperand(value);
                }
                break;
            case 0x91: /* i2b */
                {
                    Operand *val = pop(cur_frame);
                    tmpOp.val.vint = (uint8_t)(val->val.vint);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " i2b " ANSI_RESET "%d -> %d\n", val->val.vint,
                            tmpOp.val.vint);
#endif
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                    freeOperand(val);
                }
                break;
            case 0x92: /* i2c */
                {
                    Operand *val = pop(cur_frame);
                    tmpOp.val.vint = (uint16_t)(val->val.vint);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " i2c " ANSI_RESET "%d -> %u\n", val->val.vint,
                            tmpOp.val.vint);
#endif
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                    freeOperand(val);
                }
                break;
            case 0x93: /* i2s */
                {
                    Operand *val = pop(cur_frame);
                    tmpOp.val.vint = (int16_t)(val->val.vint);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " i2s " ANSI_RESET "%d -> %d\n", val->val.vint,
                            tmpOp.val.vint);
#endif
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                    freeOperand(val);
                }
                break;
            case 0x94: /* lcmp */
                {
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " lcmp " ANSI_RESET "%ld ? %ld\n",
                            one->val.vlong, two->val.vlong);
#endif
                    int32_t res = 0;

                    if (one->val.vlong > two->val.vlong) res = 1;
                    else if (one->val.vlong < two->val.vlong) res = -1;

                    push(cur_frame, newOperand(TYPE_INT, &res));

                    freeOperand(two);
                    freeOperand(one);
                }
                break;
            case 0x95: /* fcmpl */
            case 0x96: /* fcmpg */
                {
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " fcmp%c " ANSI_RESET "%lf ? %lf\n",
                            opc == 0x95 ? 'l' : 'g',
                            one->val.vfloat, two->val.vfloat);
#endif
                    int32_t res = 0;

                    if (isnan(one->val.vfloat) || isnan(two->val.vfloat))
                        res = opc == 0x96 ? 1 : -1;
                    else if (one->val.vfloat < two->val.vfloat)
                        res = 1;
                    else if (one->val.vfloat > two->val.vfloat)
                        res = -1;

                    push(cur_frame, newOperand(TYPE_INT, &res));

                    freeOperand(one);
                    freeOperand(two);
                }
                break;

            case 0x97: /* dcmpl */
            case 0x98: /* dcmpg */
                {
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " dcmp%c " ANSI_RESET "%lg ? %lg\n",
                            opc == 0x97 ? 'l' : 'g',
                            one->val.vdouble, two->val.vdouble);
#endif
                    int32_t res = 0;

                    if (isnan(one->val.vdouble) || isnan(two->val.vdouble))
                        res = opc == 0x98 ? 1 : -1;
                    else if (one->val.vdouble < two->val.vdouble)
                        res = 1;
                    else if (one->val.vdouble > two->val.vdouble)
                        res = -1;

                    push(cur_frame, newOperand(TYPE_INT, &res));

                    freeOperand(one);
                    freeOperand(two);
                }
                break;

            case 0x99: /* ifeq */
                {
                    const int16_t bra = readBranch(code, &pc, 3);
                    Operand *o = pop(cur_frame);


#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " ifeq" ANSI_RESET " %s == 0 ? goto %lu\n",
                            printOpValue(o),
                            pc + bra);
#endif
                    if (o->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);

                    switch(o->type)
                    {
                        case TYPE_BYTE:     if (o->val.vbyte == 0)      pc += bra; break;
                        case TYPE_SHORT:    if (o->val.vshort == 0)     pc += bra; break;
                        case TYPE_INT:      if (o->val.vint == 0)       pc += bra; break;
                        case TYPE_LONG:     if (o->val.vlong == 0)      pc += bra; break;
                        case TYPE_FLOAT:    if ((int32_t)(o->val.vfloat) == 0)  pc += bra; break;
                        case TYPE_CHAR:     if (o->val.vchar == 0)      pc += bra; break;
                        case TYPE_BOOL:     if (!o->val.vbool)          pc += bra; break;

                        default:
                                                errx(EXIT_FAILURE, "runCode: ifeq: can't process type %d", o->type);
                    }

                    freeOperand(o);
                }
                break;

            case 0x9a: /* ifne */
                {
                    const int16_t bra = readBranch(code, &pc, 3);
                    Operand *o = pop(cur_frame);

#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " ifne" ANSI_RESET " %s != 0 ? goto %lu\n",
                            printOpValue(o),
                            pc + bra);
#endif
                    if (o->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);

                    switch(o->type)
                    {
                        case TYPE_BYTE:     if (o->val.vbyte != 0)      pc += bra; break;
                        case TYPE_SHORT:    if (o->val.vshort != 0)     pc += bra; break;
                        case TYPE_INT:      if (o->val.vint != 0)       pc += bra; break;
                        case TYPE_LONG:     if (o->val.vlong != 0)      pc += bra; break;
                        case TYPE_BOOL:     if (o->val.vbool)           pc += bra; break;
                        case TYPE_CHAR:     if (o->val.vchar != 0)      pc += bra; break;

                        default:
                                                errx(EXIT_FAILURE, "runCode: ifne: can't process type %d", o->type);
                    }

                    freeOperand(o);
                }
                break;

            case 0x9b: /* iflt */
                {
                    const int16_t bra = readBranch(code, &pc, 3);
                    Operand *o = pop(cur_frame);

                    if (o->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " iflt" ANSI_RESET " %s < 0 ? goto %lu\n",
                            printOpValue(o),
                            pc + bra);
#endif

                    switch(o->type)
                    {
                        case TYPE_BYTE:     if (o->val.vbyte < 0)   pc += bra; break;
                        case TYPE_SHORT:    if (o->val.vshort < 0)  pc += bra; break;
                        case TYPE_INT:      if (o->val.vint < 0)    pc += bra; break;
                        case TYPE_LONG:     if (o->val.vlong < 0)   pc += bra; break;
                        case TYPE_CHAR:     if (o->val.vchar == 0)  pc += bra; break;

                        default:
                                                errx(EXIT_FAILURE, "runCode: iflt: can't process type %d", o->type);
                    }

                    freeOperand(o);
                }
                break;
            case 0x9c: /* ifge */
                {
                    const int16_t bra = readBranch(code, &pc, 3);
                    Operand *o = pop(cur_frame);

                    if (o->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " ifge" ANSI_RESET " %s >= 0 ? goto %lu\n",
                            printOpValue(o),
                            pc + bra);
#endif

                    switch(o->type)
                    {
                        case TYPE_BYTE:     if (o->val.vbyte >= 0)  pc += bra; break;
                        case TYPE_SHORT:    if (o->val.vshort >= 0) pc += bra; break;
                        case TYPE_INT:      if (o->val.vint >= 0)   pc += bra; break;
                        case TYPE_LONG:     if (o->val.vlong >= 0)  pc += bra; break;
                        case TYPE_FLOAT:    if (o->val.vfloat >= 0) pc += bra; break;
                        case TYPE_CHAR:     pc += bra; break;

                        default:
                                            errx(EXIT_FAILURE, "runCode: ifge: can't process type %d", o->type);
                    }

                    freeOperand(o);
                }
                break;
            case 0x9d: /* ifgt */
                {
                    const int16_t bra = readBranch(code, &pc, 3);
                    Operand *o = pop(cur_frame);

                    if (o->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " ifgt" ANSI_RESET " %s > 0 ? goto %lu\n",
                            printOpValue(o),
                            pc + bra);
#endif

                    switch(o->type)
                    {
                        case TYPE_BYTE:     if (o->val.vbyte > 0)   pc += bra; break;
                        case TYPE_SHORT:    if (o->val.vshort > 0)  pc += bra; break;
                        case TYPE_INT:      if (o->val.vint > 0)    pc += bra; break;
                        case TYPE_LONG:     if (o->val.vlong > 0)   pc += bra; break;
                        case TYPE_CHAR:     if (o->val.vchar > 0)   pc += bra; break;

                        default:
                                                errx(EXIT_FAILURE, "runCode: ifgt: can't process type %d", o->type);
                    }

                    freeOperand(o);
                }
                break;
            case 0x9e: /* ifle */
                {
                    const int16_t bra = readBranch(code, &pc, 3);
                    Operand *o = pop(cur_frame);

                    if (o->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " ifle" ANSI_RESET " %s <= 0 ? goto %lu\n",
                            printOpValue(o),
                            pc + bra);
#endif

                    switch(o->type)
                    {
                        case TYPE_BYTE:     if (o->val.vbyte <= 0)  pc += bra; break;
                        case TYPE_SHORT:    if (o->val.vshort <= 0) pc += bra; break;
                        case TYPE_INT:      if (o->val.vint <= 0)   pc += bra; break;
                        case TYPE_LONG:     if (o->val.vlong <= 0)  pc += bra; break;
                        case TYPE_CHAR:     if (o->val.vchar == 0)  pc += bra; break;

                        default:
                                                errx(EXIT_FAILURE, "runCode: ifle: can't process type %d", o->type);
                    }

                    freeOperand(o);
                }
                break;
            case 0x9f: /* if_icmpeq */
                {
                    const int16_t bra = readBranch(code, &pc, 3);

                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);

                    if (two->type == TYPE_NULL || one->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);

                    if (one->val.vint == two->val.vint)
                        pc += bra;
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " if_icmpne" ANSI_RESET " %d != %d ? goto %ld\n",
                            one->val.vint, two->val.vint,
                            pc + bra);
#endif
                    freeOperand(one);
                    freeOperand(two);

                }
                break;
            case 0xa0: /* if_icmpne */
                {
                    const int16_t bra = readBranch(code, &pc, 3);

                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);

                    if (two->type == TYPE_NULL || one->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);

                    if (one->val.vint != two->val.vint)
                        pc += bra;
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " if_icmpne" ANSI_RESET " %d != %d ? goto %ld\n",
                            one->val.vint, two->val.vint,
                            pc + bra);
#endif
                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0xa1: /* if_icmplt */
                {
                    const int16_t bra = readBranch(code, &pc, 3);

                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);

                    if (two->type == TYPE_NULL || one->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " if_icmplt" ANSI_RESET " %d < %d ? goto %ld\n",
                            one->val.vint, two->val.vint,
                            pc + bra);
#endif
                    if (one->val.vint < two->val.vint)
                        pc += bra;

                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0xa2: /* if_icmpge */
                {
                    const int16_t bra = readBranch(code, &pc, 3);
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);

                    if (two->type == TYPE_NULL || one->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " if_icmpge" ANSI_RESET " %d >= %d ? goto %ld\n",
                            one->val.vint, two->val.vint,
                            pc + bra);
#endif
                    if (one->val.vint >= two->val.vint)
                        pc += bra;

                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0xa3: /* if_icmpgt */
                {
                    const int16_t bra = readBranch(code, &pc, 3);
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);

                    if (two->type == TYPE_NULL || one->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " if_icmpgt" ANSI_RESET " %d > %d ? goto %ld\n",
                            one->val.vint, two->val.vint,
                            pc + bra);
#endif
                    if (one->val.vint > two->val.vint)
                        pc += bra;

                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0xa4: /* if_icmple */
                {
                    int16_t bra = readBranch(code, &pc, 3);

                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);

                    if (two->type == TYPE_NULL || one->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " if_icmple" ANSI_RESET " %d <= %d ? goto %lu\n",
                            one->val.vint, two->val.vint,
                            pc + bra);
#endif
                    if (one->val.vint <= two->val.vint)
                        pc += bra;

                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0xa5: /* if_acmpeq */
                {
                    int16_t bra = readBranch(code, &pc, 3);

                    Operand *one = pop(cur_frame);
                    Operand *two = pop(cur_frame);

                    if (two->type == TYPE_NULL || one->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " if_acmpeq" ANSI_RESET " ref == ref ? goto %lu\n",
                            pc + bra);
#endif
                    if (one->type == two->type && one->val.vref == two->val.vref)
                        pc += bra;

                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0xa6: /* if_acmpne */
                {
                    int16_t bra = readBranch(code, &pc, 3);

                    Operand *one = pop(cur_frame);
                    Operand *two = pop(cur_frame);

                    if (two->type == TYPE_NULL || one->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " if_acmpne" ANSI_RESET " ref != ref ? goto %lu\n",
                            pc + bra);
#endif
                    if (one->type == two->type && one->val.vref != two->val.vref)
                        pc += bra;

                    freeOperand(one);
                    freeOperand(two);
                }
                break;

            case 0xa7: /* goto */
                {
                    int16_t bra = readBranch(code, &pc, 3);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " goto " ANSI_RESET " %d:%lu\n", bra, pc + bra);
#endif
                    pc += bra;
                }
                break;

            case 0xa9: /* ret */
                {
                    tmp = code[++pc];
                    Operand *dest = getLocal(cur_frame, tmp);
                    if(dest->type != TYPE_RETADDR)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " ret " ANSI_RESET "%d\n", dest->val.vint);
#endif
                    /* TODO bounds check */
                    pc = dest->val.vint;
                }
                break;

            case 0xaa: /* tableswitch */
                {
                    const int64_t pc_save = pc++;
                    const int64_t pad = 4 - (pc % 4);
                    pc += pad;

                    const int32_t def = readInt(code, &pc);
                    const int32_t low = readInt(code, &pc);
                    const int32_t high = readInt(code, &pc);

                    Operand *v = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " tableswitch " ANSI_RESET "%d #%d->#%d on %d\n", def, low, high, v->val.vint);
#endif
                    //const int32_t num_offsets = high - low + 1;

                    int32_t index = v->val.vint;
                    if (index < low || index > high)
                        pc = pc_save + def; /* FIXME should this have +1 ? */
                    else {
                        index -= low;
                        pc += (index * 4);
                        pc = pc_save + readInt(code, &pc); /* FIXME should this have +1 ? */
                    }
                }
                break;
            case 0xab: /* lookupswitch */
                {
                    const int64_t pc_save = pc++;
                    const int64_t pad = 4 - (pc % 4);
                    pc += pad;
                    int32_t def = readInt(code, &pc);
                    const int32_t npairs = readInt(code, &pc);
                    Operand *v = pop(cur_frame);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " lookupswitch " ANSI_RESET "%d #%d == %d\n", def, npairs, v->val.vint);
#endif
                    bool found = false;
                    for(int i = 0; i < npairs; i++) {
                        const int32_t match = readInt(code, &pc);
                        const int32_t offset = readInt(code, &pc);
                        if (!found && match == v->val.vint)
                        {
                            found = true;
                            def = offset;
                        }
                    }
                    pc = pc_save + def - 1;
                }
                break;
            case 0xac: /* ireturn */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " ireturn\n" ANSI_RESET);
#endif
                    Operand *o = pop(cur_frame);
                    if (o == NULL) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    cur_frame->ret = o;
                    running = false;
                }
                break;
            case 0xad: /* lreturn */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " lreturn\n" ANSI_RESET);
#endif
                    Operand *o = pop(cur_frame);
                    if (o == NULL) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    cur_frame->ret = o;
                    running = false;
                }
                break;
            case 0xae: /* freturn */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR "freturn\n" ANSI_RESET);
#endif
                    Operand *o = pop(cur_frame);
                    if (o == NULL) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    cur_frame->ret = o;
                    running = false;
                }
                break;
            case 0xaf: /* dreturn */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR "dreturn\n" ANSI_RESET);
#endif
                    Operand *o = pop(cur_frame);
                    if (o == NULL) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    cur_frame->ret = o;
                    running = false;
                }
                break;
            case 0xb0: /* areturn */
                {
                    Operand *o = pop(cur_frame);
                    if (o == NULL) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " areturn " ANSI_RESET);
                    printf("%s %s\n", printOpType(o->type), printOpValue(o));
#endif
                    cur_frame->ret = o;
                    running = false;
                }
                break;
            case 0xb1: /* return */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " return\n" ANSI_RESET);
#endif
                    running = false;
                }
                break;
            case 0xb2: /* getstatic */
                {
                    tmpw = readShort(code, &pc);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " getstatic" ANSI_RESET " #%d ", tmpw);
#endif
                    field_info *f = NULL;
                    if ((ex_opr = getFieldInfo(thread, cls, tmpw, &f)) != NULL)
                        return ex_opr;
                    if (f == NULL)
                        errx(EXIT_FAILURE, "getstatic: can't find field");
#ifdef DEBUG_CODE
                    printf("got field_info '%s' '%s' ",
                            f->name->d.utf8.ascii,
                            f->descriptor->d.utf8.ascii);
#endif
                    if (f->static_operand) {
#ifdef DEBUG_CODE
                        printf("contents %s %s\n",
                                printOpType(f->static_operand->type),
                                printOpValue(f->static_operand));
#endif
                        if(!push(cur_frame, dupOperand(f->static_operand)))
                            return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    } else {
                        if(!push(cur_frame, newOperand(TYPE_NULL, NULL)))
                            return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    }
                }
                break;
            case 0xb3: /* putstatic */
                {
                    tmpw = readShort(code, &pc);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " putstatic" ANSI_RESET " #%d ", tmpw);
#endif
                    field_info *f = NULL;

                    if ((ex_opr = getFieldInfo(thread, cls, tmpw, &f)) != NULL)
                        return ex_opr;

                    if (f == NULL)
                        errx(EXIT_FAILURE, "putstatic: no field");
#ifdef DEBUG_CODE
                    printf("got field_info '%s' '%s'\n",
                            f->name->d.utf8.ascii,
                            f->descriptor->d.utf8.ascii);
#endif
                    Operand *value = pop(cur_frame);
                    if (value == NULL) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

                    if (f->static_operand)
                        freeOperand(f->static_operand);
                    f->static_operand = dupOperand(value);
                    freeOperand(value);
                }
                break;
            case 0xb4: /* getfield */
                {
                    tmpw = readShort(code, &pc);
                    field_info *fi = NULL;
                    if ((ex_opr = getFieldInfo(thread, cls, tmpw, &fi)) != NULL)
                        return ex_opr;
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " getfield" ANSI_RESET " #%d ", tmpw);
#endif
                    if (fi == NULL) {
                        errx(EXIT_FAILURE, "getfield: no field\n");
                        break;
                    }

                    Operand *o = pop(cur_frame);
                    if (o->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);

#ifdef DEBUG_CODE
                    printf("to: field_info '%s' '%s'\nto: %s %s\n",
                            fi->name->d.utf8.ascii,
                            fi->descriptor->d.utf8.ascii,
                            printOpType(o->type),
                            printOpValue(o)
                          );
#endif

                    Object *obj = o->val.vref;

                    Field *fld = NULL;
                    for (int i = 0; i < obj->data.obj.num_fields; i++)
                        if(fi == obj->data.obj.fields[i].field_info)
                            fld = &obj->data.obj.fields[i];

                    if (fld == NULL)
                        errx(EXIT_FAILURE, "getfield: Can't find field");

                    Operand *retop = (fld->op == NULL) ?
                        newOperand(TYPE_NULL, NULL) :
                        dupOperand(fld->op);

                    if(!push(cur_frame, retop))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

                    freeOperand((Operand *)o);
                }
                break;
            case 0xb5: /* putfield */
                {
                    /* TODO field #1 for inner classes is the parent class instance,
                     * this makes the child reference the parent and vice-versa
                     * meaning it is never freed*/
                    //tmpw = (uint16_t)(256U * code[pc+1] + code[pc+2]); pc += 2;
                    tmpw = readShort(code, &pc);
                    field_info *fi = NULL;
                    if ((ex_opr = getFieldInfo(thread, cls, tmpw,
                                    (field_info **)&fi)) != NULL)
                        return ex_opr;
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " putfield " ANSI_RESET " #%d ", tmpw);
#endif
                    if (fi == NULL) {
                        errx(EXIT_FAILURE, "putfield: no field");
                        break;
                    }
                    Operand *value = pop(cur_frame);
                    if (value == NULL) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
#ifdef DEBUG_CODE
                    printf(" from: %s %s", printOpType(value->type), printOpValue(value));
                    printf(" to  : %s %s\n",
                            fi->name->d.utf8.ascii,
                            fi->descriptor->d.utf8.ascii);
#endif
                    Operand *o = pop(cur_frame);
                    if (o->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
                    Object *obj = o->val.vref;

                    Field *fld = NULL;

                    if (o->type != TYPE_OREF) {
                        warnx("putfield: not OREF");
                        __asm__("int $3");
                    }

                    for (int i = 0; i < obj->data.obj.num_fields; i++) {
                        if(fi == obj->data.obj.fields[i].field_info)
                            fld = &obj->data.obj.fields[i];
                    }

                    if (fld == NULL) {
                        warnx("putfield: unknown field: %s %s",
                                fi->name->d.utf8.ascii, fi->descriptor->d.utf8.ascii);
                        __asm__("int $3");
                    }

                    if(fld->op) freeOperand(fld->op);
                    fld->op = dupOperand(value);
                    freeOperand(value);
                    freeOperand(o);
                }
                break;
            case 0xb6: /* invokevirtual */
                {
                    tmpw = readShort(code, &pc);
                    cp_Fieldref_info *mi = &cls->constant_pool[tmpw]->d.fr;
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " invokevirtual" ANSI_RESET " #%d ", tmpw);
                    printf("method_info %s %s %s\n",
                            mi->class->d.class.name->d.utf8.ascii,
                            mi->nametype->d.nt.name->d.utf8.ascii,
                            mi->nametype->d.nt.descriptor->d.utf8.ascii);
#endif
                    /* used for num meths only */
                    method_info *meth = findClassMethod(thread,
                            mi->class->d.class.name->d.utf8.ascii,
                            mi->nametype->d.nt.name->d.utf8.ascii,
                            mi->nametype->d.nt.descriptor->d.utf8.ascii);

                    if (meth == NULL)
                        return throw(thread, "java.lang.ClassNotFoundException",
                                mi->class->d.class.name->d.utf8.ascii, pc);

                    if (cur_frame->sp <= meth->desc.num_params)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

                    Operand *op_obj = cur_frame->stack[cur_frame->sp - meth->desc.num_params - 1];

                    if (op_obj->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException",
                                "invokevirtual: object is null: " ERR_AT, pc);

                    Object *this = op_obj->val.vref;
                    if (!this) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

                    meth = findMethodByClass(thread,
                            this->class,
                            mi->nametype->d.nt.name->d.utf8.ascii,
                            mi->nametype->d.nt.descriptor->d.utf8.ascii);

                    if (meth == NULL) {
                        warnx("findMethodByClass failed: %s %s %s",
                                this->class->this_class->d.class.name->d.utf8.ascii,
                                mi->nametype->d.nt.name->d.utf8.ascii,
                                mi->nametype->d.nt.descriptor->d.utf8.ascii);

                        return throw(thread, "java.lang.AbstractMethodError",
                                NULL, pc);
                    }

                    if (!this->class) {
                        Operand *ret = classInit(thread, (ClassFile *)this->class);
                        if (ret != NULL) return ret;
                    }

#ifdef DEBUG_CODE
                    printf("           [%s.%s%s] => %s.%s%s on %s\n",
                            mi->class->d.class.name->d.utf8.ascii,
                            mi->nametype->d.nt.name->d.utf8.ascii,
                            mi->nametype->d.nt.descriptor->d.utf8.ascii,
                            meth->class->this_class->d.class.name->d.utf8.ascii,
                            meth->name->d.utf8.ascii,
                            meth->descriptor->d.utf8.ascii,
                            this->class ?
                            this->class->this_class->d.class.name->d.utf8.ascii :
                            "(array)");
#endif

                    if ((ex_opr = invokeMethod(thread, meth, true, pc)) != NULL)
                        return ex_opr;

                }
                break;
            case 0xb7: /* invokespecial */
                {
                    tmpw = readShort(code, &pc);
                    cp_Fieldref_info *mi = &cls->constant_pool[tmpw]->d.fr;
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " invokespecial" ANSI_RESET " #%d ", tmpw);
                    printf("method_info special: %s:%s %s\n",
                            mi->class->d.class.name->d.utf8.ascii,
                            mi->nametype->d.nt.name->d.utf8.ascii,
                            mi->nametype->d.nt.descriptor->d.utf8.ascii);
#endif
                    method_info *meth = findClassMethod(thread,
                            mi->class->d.class.name->d.utf8.ascii,
                            mi->nametype->d.nt.name->d.utf8.ascii,
                            mi->nametype->d.nt.descriptor->d.utf8.ascii);

                    if (cur_frame->sp <= meth->desc.num_params)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

                    Operand *op_obj = cur_frame->stack[cur_frame->sp - meth->desc.num_params - 1];

                    if (op_obj->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException",
                                "invokespecial: object is null", pc);

                    Object *this = op_obj->val.vref;
                    if (!this) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

                    if (!this->class) {
                        Operand *ret = classInit(thread, (ClassFile *)this->class);
                        if (ret != NULL) return ret;
                    }

                    if (meth == NULL)
                        return throw(thread, "java.lang.AbstractMethodError", NULL, pc);
#ifdef DEBUG_CODE
                    printf("           [%s.%s%s] => %s.%s%s on %s\n",
                            mi->class->d.class.name->d.utf8.ascii,
                            mi->nametype->d.nt.name->d.utf8.ascii,
                            mi->nametype->d.nt.descriptor->d.utf8.ascii,
                            meth->class->this_class->d.class.name->d.utf8.ascii,
                            meth->name->d.utf8.ascii,
                            meth->descriptor->d.utf8.ascii,
                            this->class->this_class->d.class.name->d.utf8.ascii);
#endif

                    if ((ex_opr = invokeMethod(thread, meth, true, pc)) != NULL)
                        return ex_opr;
                }
                break;
            case 0xb8: /* invokestatic */
                {
                    tmpw = readShort(code, &pc);
                    cp_Fieldref_info *mi = &cls->constant_pool[tmpw]->d.fr;
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " invokestatic" ANSI_RESET " #%d ", tmpw);
                    printf("method_info static %s:%s %s\n",
                            mi->class->d.class.name->d.utf8.ascii,
                            mi->nametype->d.nt.name->d.utf8.ascii,
                            mi->nametype->d.nt.descriptor->d.utf8.ascii);
#endif
                    /* class containing static mehtod */
                    ClassFile *class = (ClassFile *)findClass(thread,
                            mi->class->d.class.name->d.utf8.ascii);

                    /* static method */
                    method_info *meth = findClassMethod(thread,
                            mi->class->d.class.name->d.utf8.ascii,
                            mi->nametype->d.nt.name->d.utf8.ascii,
                            mi->nametype->d.nt.descriptor->d.utf8.ascii);

                    if (meth == NULL)
                        return throw(thread, "java.lang.AbstractMethodError", ERR_AT, pc);

                    if(!class->isInit) {
                        Operand *ret = classInit(thread, (ClassFile *)class);
                        if (ret != NULL) return ret;
                    }

                    if ((ex_opr = invokeMethod(thread, meth, false, pc)) != NULL)
                        return ex_opr;
                }
                break;
            case 0xb9: /* invokeinterface */
                {
                    tmpw = readShort(code, &pc);
                    cp_Fieldref_info *mi = &cls->constant_pool[tmpw]->d.fr;
                    pc++; // skip first field, which has a value
                    pc++; // skip blank field
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " invokeinterface " ANSI_RESET " #%d,%d\n", tmpw, tmp);
                    printf("method_info interface %s:%s %s\n",
                            mi->class->d.class.name->d.utf8.ascii,
                            mi->nametype->d.nt.name->d.utf8.ascii,
                            mi->nametype->d.nt.descriptor->d.utf8.ascii);

#endif
                    /* interface method defintion (for param count) */
                    method_info *meth = findClassMethod(thread,
                            mi->class->d.class.name->d.utf8.ascii,
                            mi->nametype->d.nt.name->d.utf8.ascii,
                            mi->nametype->d.nt.descriptor->d.utf8.ascii);

                    if (meth == NULL) {
                        warnx("invokeinterface: %s.%s%s missing",
                                mi->class->d.class.name->d.utf8.ascii,
                                mi->nametype->d.nt.name->d.utf8.ascii,
                                mi->nametype->d.nt.descriptor->d.utf8.ascii);
                        return throw(thread, "java.lang.AbstractMethodError", ERR_AT, pc);
                    }

                    if (cur_frame->sp <= meth->desc.num_params)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

                    Operand *op_obj = cur_frame->stack[cur_frame->sp - meth->desc.num_params - 1];
                    if (op_obj->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);

                    Object *this = op_obj->val.vref;
                    if (!this) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

                    /* actual method instance in implementing class */
                    meth = findMethodByClass(thread,
                            this->class,
                            mi->nametype->d.nt.name->d.utf8.ascii,
                            mi->nametype->d.nt.descriptor->d.utf8.ascii);
                    if (meth == NULL)
                        return throw(thread, "java.lang.AbstractMethodError",
                                ERR_AT, pc);

#ifdef DEBUG_CODE
                    printf("actual method: %s.%s%s\n",
                            this->class->this_class->d.class.name->d.utf8.ascii,
                            meth->name->d.utf8.ascii,
                            meth->descriptor->d.utf8.ascii);
#endif
                    if (!this->class) {
                        Operand *ret = classInit(thread, (ClassFile *)this->class);
                        if (ret != NULL) return ret;
                    }
#ifdef DEBUG_CODE
                    printf("invoking\n");
#endif
                    if ((ex_opr = invokeMethod(thread, meth, true, pc)) != NULL)
                        return ex_opr;
                }
                break;
            case 0xbb: /* new */
                {
                    tmpw = readShort(code, &pc);
                    cp_Class_info *ci = &cls->constant_pool[tmpw]->d.class;
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " new " ANSI_RESET "#%d ", tmpw);
                    printf("class %s\n", ci->name->d.utf8.ascii);
#endif
                    ClassFile *tmpcls = findClass(thread, ci->name->d.utf8.ascii);
                    if (tmpcls == NULL)
                        return throw(thread, "java.lang.ClassNotFoundException", ERR_AT, pc);
                    Object *newObj = newObject(thread, tmpcls);
                    Operand *newOp = newOperand(TYPE_OREF, newObj);

                    if (newObj == NULL || newOp == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

                    if (!push(cur_frame, newOp))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0xbc: /* newarray */
                {
                    tmp = code[++pc];
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " newarray " ANSI_RESET "#%d \n", tmp);
                    dumpFrame(cur_frame);
#endif
                    Operand *cnt = pop(cur_frame);
                    if (cnt == NULL) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    if (cnt->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);

                    if (cnt->val.vint < 0) {
                        freeOperand(cnt);
                        return throw(thread, "java.lang.NegativeArraySizeException", ERR_AT, pc);
                    }

                    Object *arr = newArray(thread, atypemap[tmp], cnt->val.vint, NULL);
                    if (arr == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    if (!push(cur_frame, newOperand(TYPE_AREF, arr)))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    freeOperand(cnt);
                }
                break;
            case 0xbd: /* anewarray */
                {
                    tmpw = readShort(code, &pc);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " anewarray " ANSI_RESET "#%d \n", tmpw);
#endif
                    Operand *cnt = pop(cur_frame);
                    if (cnt == NULL) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    if (cnt->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);

                    if (cnt->val.vint < 0) {
                        freeOperand(cnt);
                        return throw(thread, "java.lang.NegativeArraySizeException", ERR_AT, pc);
                    }

                    // FIXME store class from cp[tmpw]
                    ClassFile *class = findClass(thread, cls->constant_pool[tmpw]->d.class.name->d.utf8.ascii);
                    if (class == NULL)
                        return throw(thread, "java.lang.ClassNotFoundException", ERR_AT, pc);

                    Object *arr = newArray(thread, TYPE_OREF, cnt->val.vint, class);
                    if (arr == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

                    if (!push(cur_frame, newOperand(TYPE_AREF, arr)))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    freeOperand(cnt);
                }
                break;
            case 0xbe: /* arraylength */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " arraylength " ANSI_RESET);
#endif
                    Operand *op_array = pop(cur_frame);
                    if (op_array == NULL) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    if (op_array->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);

                    Object *array = op_array->val.vref;
#ifdef DEBUG_CODE
                    printf("= %d\n", array->data.array.len);
#endif
                    if (!push(cur_frame, newOperand(TYPE_INT,
                                    &array->data.array.len)))
                        errx(EXIT_FAILURE, "arraylength: stack full");
                    freeOperand(op_array);
                }
                break;
            case 0xbf: /* athrow */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " athrow " ANSI_RESET);
#endif
                    Operand *ex = pop(cur_frame);
                    Operand *ex_opr = NULL;
                    Operand *tmpop = NULL;

                    if (ex == NULL) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    if (ex->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);

                    while(cur_frame->sp > 0 && (tmpop = pop(cur_frame)) != NULL)
                        freeOperand(tmpop);
                    //warnx("runCode: throw: %s", ex->val.vref->class->this_class->d.class.name->d.utf8.ascii);

                    method_info *mi = findMethodByClass(thread, ex->val.vref->class,
                            "fillInStackTrace", "()Ljava.lang.Throwable;");

                    push(cur_frame, dupOperand(ex));

                    if ((ex_opr = invokeMethod(thread, mi, true, pc)) != NULL)
                        return ex_opr;
                    else {
                        return ex;
                    }
                }
                break;
            case 0xc0: /* checkcast */
                {
                    tmpw = readShort(code, &pc);
                    cp_Class_info *ci = &cls->constant_pool[tmpw]->d.class;
                    Operand *ref = pop(cur_frame);
                    if (ref == NULL) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

                    /* val.vref check for casting null to something */
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " checkcast " ANSI_RESET "%s >= %s\n",
                            ref->val.vref ? ref->val.vref->class->this_class->d.class.name->d.utf8.ascii : "null",
                            ci->name->d.utf8.ascii);
#endif
                    if (!canCast(thread,
                                ref->val.vref ? ref->val.vref->class->this_class->d.class.name->d.utf8.ascii : NULL,
                                ci->name->d.utf8.ascii))
                        return throw(thread, "java.lang.ClassCastException", NULL, pc);
                    push(cur_frame, ref);
                }
                break;
            case 0xc1: /* instanceof */
                {
                    tmpw = readShort(code, &pc);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " instanceof\n" ANSI_RESET);
#endif
                    Operand *ref = pop(cur_frame);
                    if (ref == NULL) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    if (ref->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
                    cp_Class_info *ci = &cls->constant_pool[tmpw]->d.class;
                    tmpOp.val.vint = ref->val.vref->class == findClass(thread, ci->name->d.utf8.ascii);

                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));

                    freeOperand(ref);
                }
                break;
            case 0xc2: /* monitorenter */
                {
                    Operand *lock = pop(cur_frame);
                    if (lock->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " monitorenter " ANSI_RESET " %s %s\n",
                            printOpType(lock->type),
                            printOpValue(lock));
#endif
                    if (lock->type != TYPE_OREF && lock->type != TYPE_AREF)
                        errx(EXIT_FAILURE, "monitorenter on invalid type");

                    int rc;
                    char buf[BUFSIZ];

                    if ((rc = pthread_mutex_lock(&lock->val.vref->monitor)) != 0) {
                        strerror_r(rc, buf, sizeof(buf));
                        warnx("monitorenter: %s", buf);
                        return throw(thread, "java.lang.IllegalMonitorStateException", buf, pc);
                    }
                    // FIXME list of locks in case thread dies?
                    // FIXME if locked don't freeObject in gc
                    freeOperand(lock);
                }
                break;
            case 0xc3: /* monitorexit */
                {
                    Operand *lock = pop(cur_frame);
                    if (lock->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " monitorexit " ANSI_RESET " %s %s\n",
                            printOpType(lock->type),
                            printOpValue(lock));
#endif
                    if (lock->type != TYPE_OREF && lock->type != TYPE_AREF)
                        errx(EXIT_FAILURE, "monitorexit on invalid type");

                    int rc;
                    char buf[BUFSIZ];

                    if ((rc = pthread_mutex_unlock(&lock->val.vref->monitor)) != 0) {
                        strerror_r(rc, buf, sizeof(buf));
                        warnx("monitorexit: %s", buf);
                        freeOperand(lock);
                        return throw(thread, "java.lang.IllegalMonitorStateException", buf, pc);
                    }
                    // FIXME list of locks in case thread dies?
                    // FIXME if locked don't freeObject in gc
                    freeOperand(lock);
                }
                break;
            case 0xc4: /* wide   */
                {
                    is_wide = true;
                }
                break;
            case 0xc5: /* multianewarray */
                {
                    tmpw = readShort(code, &pc);
                    uint8_t dim  = code[++pc];
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " multianewarray " ANSI_RESET "#%d[%d] \n", tmpw, dim);
                    dumpFrame(cur_frame);
#endif

                    // FIXME store class from cp[tmpw]
                    ClassFile *class = findClass(thread, cls->constant_pool[tmpw]->d.class.name->d.utf8.ascii);
                    if (class == NULL)
                        return throw(thread, "java.lang.ClassNotFoundException", ERR_AT, pc);

                    /* need to pop tmp dimensions values off the op stack */
                    int32_t *sizes = calloc(dim + 1, sizeof(int32_t));
                    if (sizes == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

                    for(int off = 0; off < dim; off++) {
                        Operand *cnt = pop(cur_frame);
                        if (cnt == NULL) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                        if (cnt->type == TYPE_NULL) {
                            freeOperand(cnt);
                            free(sizes);
                            return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
                        }
                        if (cnt->val.vint < 0) {
                            freeOperand(cnt);
                            free(sizes);
                            return throw(thread, "java.lang.NegativeArraySizeException", ERR_AT, pc);
                        }
                        freeOperand(cnt);

                        sizes[off] = cnt->val.vint;
                    }

                    Object *arr = newMultiArray(thread, TYPE_OREF, sizes, dim, class);
                    if (arr == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    if (!push(cur_frame, newOperand(TYPE_AREF, arr)))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

                    /* FIXME do i need to free(sizes) ? */
                }
                break;

            case 0xc6: /* ifnull */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " ifnull\n" ANSI_RESET);
#endif
                    //int16_t bra = (int16_t)(256U * code[pc+1] + code[pc+2]); pc += 2;
                    //bra = (int16_t)(bra - 3);
                    const int16_t bra = readBranch(code, &pc, 3);
                    Operand *one = pop(cur_frame);
                    if (one == NULL) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    if (one->type == TYPE_NULL)
                        pc += bra;
                    freeOperand(one);
                }
                break;

            case 0xc7: /* ifnonnull */
                {
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " ifnonnull\n" ANSI_RESET);
#endif
                    const int16_t bra = readBranch(code, &pc, 3);
                    Operand *one = pop(cur_frame);
                    if (one == NULL) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    if (one->type != TYPE_NULL)
                        pc += bra;
                    freeOperand(one);
                }
                break;

            case 0xc8: /* goto_w */
                {
                    int32_t bra = readBranchWide(code, &pc, 5);
#ifdef DEBUG_CODE
                    printf(ANSI_INSTR " goto " ANSI_RESET " %d:%lu\n", bra, pc + bra);
#endif
                    pc += bra;
                }
                break;

            default:
                errx(EXIT_FAILURE,
                        ANSI_BRED "[%2ld] Unknown opcode 0x%02x\n" ANSI_RESET,
                        pc, opc);
                running = false;
                break;
        }
        if (running) pc++;
    }

    return NULL;
}

#ifdef DEBUG_CODE
void dumpFrame(const Frame *f)
{
    if (!f) {
        printf(" <<NULL>>\n");
        return;
    }

    printf("  sp: %d, num_local: %d, num_stack: %d\n",
            f->sp,
            f->num_local,
            f->num_stack);

    if (f->local) {
        printf("  local:\n");
        for (uint16_t i = 0; i < f->num_local; i++)
            if(f->local[i]) {
                printf("   [%d]: %s [%s]\n", i,
                        printOpType(f->local[i]->type),
                        printOpValue(f->local[i]));
                if (f->local[i]->type == TYPE_LONG ||
                        f->local[i]->type == TYPE_DOUBLE)
                    printf("   [%d]: --\n", i++);
            }
    } else
        printf("  local: <<none>>\n");

    if (f->num_stack) {
        if (f->sp) {
            printf("  stack:\n");
            for (uint16_t i = 0; i < f->sp; i++) {
                if (f->stack[i])
                    printf("   [%d]: %s [%s]\n", i,
                            printOpType(f->stack[i]->type),
                            printOpValue(f->stack[i]));
            }
        } else
            printf("  stack: <empty>\n");
    } else
        printf("  stack: <<none>>\n");
}
#endif

static Operand *invokeMethod(Thread *thread, const method_info *meth, bool withObject, int64_t old_pc)
{
    Frame *oldframe = thread->frame_free ? thread->stack[thread->frame_free-1] : NULL;
    uint16_t num_operands, num_locals;
    int64_t pc = 0;
    Code_attribute *code = NULL;
    Operand *throwOp = NULL;
    Object *th_obj = NULL;
	Frame *cur_frame = currentFrame(thread);

	assert(thread != NULL);
	assert(meth != NULL);

	if (withObject && (throwOp = cur_frame->stack[cur_frame->sp - meth->desc.num_params - 1])->type != TYPE_OREF) {
		printf("(invokeMethod [%s] %s.%s%s ",
            withObject ? "true": "false",
            meth->class->this_class->d.class.name->d.utf8.ascii,
            meth->name->d.utf8.ascii,
            meth->descriptor->d.utf8.ascii
          );
		printf("%s %s ", printOpType(throwOp->type), printOpValue(throwOp));
		printf("\n");
		abort();
		return throw(thread, "java.lang.VirtualMachineError", NULL, thread->pc);
	}


#ifdef DEBUG_CODE
    printf("(invokeMethod [%s] %s.%s%s)\n",
            withObject ? "true": "false",
            meth->class->this_class->d.class.name->d.utf8.ascii,
            meth->name->d.utf8.ascii,
            meth->descriptor->d.utf8.ascii
          );
#endif

    /* extract required Frame parameters for native/non-native methods */
    if ((meth->access_flags & (ACC_NATIVE|ACC_STATIC)) == (ACC_NATIVE|ACC_STATIC)) {
        if (!meth->nativeMethod.mstatic)
        {
            errx(EXIT_FAILURE, "Missing static native method in %s %s %s",
                    meth->class->this_class->d.class.name->d.utf8.ascii,
                    meth->name->d.utf8.ascii,
                    meth->descriptor->d.utf8.ascii);
        }
        num_operands = (uint16_t)(meth->desc.num_params + 13U); // FIXME this should be a max(return#, params#)
        num_locals = (uint16_t)(meth->desc.num_params * 2U);
    } else if (meth->access_flags & ACC_NATIVE) {
        if (!meth->nativeMethod.mvirtual)
            errx(EXIT_FAILURE, "Missing virtual native method in %s %s %s",
                    meth->class->this_class->d.class.name->d.utf8.ascii,
                    meth->name->d.utf8.ascii,
                    meth->descriptor->d.utf8.ascii);
        num_operands = (uint16_t)(meth->desc.num_params + 13U); // FIXME this should be a max(return#, params#)
        num_locals = (uint16_t)(meth->desc.num_params * 2U);
    } else {
        const attribute_info *const ai = findAttribute(meth->attributes, meth->attributes_count, "Code");
        if (!ai) {
            char buf[BUFSIZ];
            printAccessFlags(meth->access_flags, buf, sizeof(buf));
            warnx("Missing code attribute for method (%s %s %s [%s])",
                meth->class->this_class->d.class.name->d.utf8.ascii,
                meth->name->d.utf8.ascii,
                meth->descriptor->d.utf8.ascii,
                buf);
            exit(1);
            return throw(thread, "java.lang.VirtualMachineError", ERR_AT, old_pc);
        }

        if ((code = ai->d.code) == NULL)
            errx(EXIT_FAILURE, "Missing code for method (%s %s %s)",
                    meth->class->this_class->d.class.name->d.utf8.ascii,
                    meth->name->d.utf8.ascii,
                    meth->descriptor->d.utf8.ascii);

        /* this is +x to handle situations such as createClass() */
        num_operands = (uint16_t)(code->max_stack + 3U);
        num_locals = (uint16_t)(code->max_locals * 2U);
    }

    /* prepare and push the new frame */
    Frame *newframe = newFrame(num_operands, num_locals);
    if(newframe == NULL)
        errx(EXIT_FAILURE, "invokeMethod: newframe is NULL");
    newframe->num_stack = num_operands;

    if(thread->frame_free >= MAX_STACK)
        errx(EXIT_FAILURE, "invokeMethod: thread stack is full");

    thread->stack[thread->frame_free++] = newframe;
    const method_info *old_meth = thread->cur_method;
    thread->cur_method = meth;
    newframe->mi = meth;
    newframe->class = meth->class;

    /* "this" needs to be stack[0] */
    uint16_t local_count = 0;
    for (ssize_t i = 0; i < meth->desc.num_params; i++)
        if(meth->desc.params[i]->baseType == TYPE_LONG ||
                meth->desc.params[i]->baseType == TYPE_DOUBLE)
            local_count = (uint16_t)(local_count + 2U);
        else
            local_count++;

    local_count = (uint16_t)(local_count - (withObject ? 0 : 1));
    // was withobject ? 1 : 0;

    //uint16_t local_count = (uint16_t)(meth->desc.num_params - (withObject ? 0 : 1)); // was withobject ? 1 : 0;

    /* push parameters from the previous frame to this one */
    if (oldframe && newframe->local) {
        if (oldframe->sp < meth->desc.num_params) {
            warnx("invokeMethod: expected %d got %d: " ERR_AT,
                    meth->desc.num_params,
                    oldframe->sp);
            __asm__("int $3");
        }
        for (int i = 0; i < meth->desc.num_params; i++) {
            Operand *tmp = pop(oldframe);
            if(tmp->type == TYPE_LONG || tmp->type == TYPE_DOUBLE) {
                newframe->local[local_count--] = tmp;
            }
            newframe->local[local_count--] = tmp; // WAS ++
        }
        if (withObject) {
            setLocal(newframe, 0, pop(oldframe));
        }
    }


    /* invoke the method */
#ifdef DEBUG_CODE
    printf("(invoking method %s %s)\n", meth->name->d.utf8.ascii,
            meth->descriptor->d.utf8.ascii);
#endif
    if ((meth->access_flags & (ACC_NATIVE|ACC_STATIC)) == (ACC_NATIVE|ACC_STATIC)) {
#ifdef DEBUG_CODE
        printf(" (native+static %s)\n", meth->name->d.utf8.ascii);
#endif
        throwOp = meth->nativeMethod.mstatic(thread, meth->class);
    } else if (meth->access_flags & ACC_NATIVE) {
#ifdef DEBUG_CODE
        printf(" (native %s)\n", meth->name->d.utf8.ascii);
#endif
        if (!withObject)
            errx(EXIT_FAILURE, "invokeMethod: mvirtual but no withObject");
        if (!newframe->local || !newframe->local[0])
            errx(EXIT_FAILURE, "invokeMethod: missing local[0]");
        if (newframe->local[0]->type != TYPE_OREF && newframe->local[0]->type != TYPE_AREF)
            errx(EXIT_FAILURE, "invokeMethod: not an Object" ERR_AT);
        throwOp = meth->nativeMethod.mvirtual(thread, meth->class,
                newframe->local[0]->val.vref);
    } else {
#ifdef DEBUG_CODE
        printf(" (normal)\n");
#endif
        throwOp = runCode(thread, code, 0, &pc);
    }

redo:
#ifdef DEBUG_CODE
    //dumpFrame(newframe);
    printf(ANSI_BRED "(done method %s %s%s)"ANSI_RESET"\n",
            meth->name->d.utf8.ascii,
            meth->descriptor->d.utf8.ascii,
            throwOp ? " [throw Object]" : "");
#endif

    if (throwOp) {
        th_obj = throwOp->val.vref;
#ifdef DEBUG_CODE
        printf("(method threw - %s %s)\n", printObject(th_obj), th_obj->class->this_class->d.class.name->d.utf8.ascii);
#endif

        if (meth->code && meth->code->exception_table_length) {
            for (int i = 0; i < meth->code->exception_table_length; i++)
            {
                exception *ex = meth->code->exception_table[i];

                if(pc < ex->start_pc ||
                        pc > ex->end_pc ||
                        (ex->catch_type != NULL &&
                         strcmp(th_obj->class->this_class->d.class.name->d.utf8.ascii,
                             ex->catch_type->d.class.name->d.utf8.ascii)))
                    continue;

#ifdef DEBUG_CODE
                printf("(handler match)\n");
#endif
                while(newframe->sp) freeOperand(pop(newframe));
                push(newframe, newOperand(TYPE_OREF, th_obj));
                freeAndNullOperand(&throwOp); /* otherwise the old throw is never free'd */
                throwOp = runCode(thread, code, ex->handler_pc, &pc);
                goto redo;
            }
        }
        freeAndNullOperand(&throwOp);
    } else
        th_obj = NULL;

    /* if the method returns something, push it into the previous frame */
    if (oldframe && !th_obj) switch(meth->desc.ret.baseType) {
        case TYPE_VOID:
            break;
        default:
            {
                if (newframe->ret) {
#ifdef DEBUG_CODE
                    printf("(pushing return %s %s)\n",
                            printOpType(newframe->ret->type),
                            printOpValue(newframe->ret)
                          );
#endif
                    push(oldframe, dupOperand(newframe->ret));
                    freeAndNullOperand(&newframe->ret);
                } else push(oldframe, newOperand(TYPE_NULL, NULL));
            }
            break;
    } else if (newframe->ret) {
        warnx("newframe: ret is set?");
        __asm__("int $3");
    }

    /* pop off and free the new frame */
    thread->frame_free--;
    thread->stack[thread->frame_free] = NULL;
    thread->cur_method = old_meth;

    freeFrame(newframe);

#ifdef DEBUG_CODE
    if (old_meth) {
        printf(ANSI_BMAG "(back to %s.%s %s)\n" ANSI_RESET,
                old_meth->class->this_class->d.class.name->d.utf8.ascii,
                old_meth->name->d.utf8.ascii,
                old_meth->descriptor->d.utf8.ascii);
    } else
        printf(ANSI_BYEL "(no parent method)\n" ANSI_RESET);
#endif

    if (th_obj) {
#ifdef DEBUG_CODE
        printf("(returning with thrown object %s)\n", printObject(th_obj));
#endif
        return newOperand(TYPE_OREF, th_obj);
    } else
        return NULL;
}

static bool addClass(Thread *thread, ClassFile *cf)
{
#ifdef DEBUG
    printf("(addClass: %s)\n", cf->this_class->d.class.name->d.utf8.ascii);
#endif

    if (cf == NULL)
        errx(EXIT_FAILURE, "addClass: Attempting to add NULL class");

    uint16_t ptr = 0;

    if (isClassLoaded(thread, cf->this_class->d.class.name->d.utf8.ascii)) {
        warnx("addClass: %s [exists]", cf->this_class->d.class.name->d.utf8.ascii);
        return false;
    }

    JVM *jvm = (JVM *)thread->jvm;

    if (pthread_rwlock_wrlock(&jvm->rwlock))
        errx(EXIT_FAILURE, "addClass: pthread_rwlock_wrlock");

    for (; jvm->method_area[ptr] && ptr < MAX_METHOD; ptr++) ;

    if (ptr == MAX_METHOD)
        errx(EXIT_FAILURE, "addClass: method_area full");

    ((JVM *)jvm)->method_area[ptr] = cf;
    ((ClassFile *)cf)->heaploc = ptr;

    if (pthread_rwlock_unlock(&jvm->rwlock))
        errx(EXIT_FAILURE, "addClass: pthread_rwlock_unlock");

    //printf(ANSI_BYEL "addClass: %s\n" ANSI_RESET, cf->this_class->d.class.name->d.utf8.ascii);

    return true;
}

static long totalOperands = 0;

static Operand *dupOperand(const Operand *old)
{
    assert(old != NULL);
    assert(old->freed == 0);

    Operand *new = malloc(sizeof(Operand));
    if (!new) {
        warn("malloc");
        return NULL;
    }

    memcpy(new, old, sizeof(Operand));

    if (new->type == TYPE_OREF || new->type == TYPE_AREF) {
        assert(new->val.vref != NULL);
        new->val.vref->lock++; // lock-- in freeOperand
    }

    totalOperands++;
#ifdef DEBUG
//  printf("dupOperand: @%p %s %s -> @%p %s %s\n",
//          (void *)old, printOpType(old->type), printOpValue(old),
//          (void *)new, printOpType(new->type), printOpValue(new));
#endif
    return new;
}


static Operand *newOperand(uint8_t type, const void *value)
{
    Operand * ret = calloc(1, sizeof(Operand)); // making this malloc causes SEGV?? FIXME
    if (!ret) {
        warn("calloc");
        return NULL;
    }

    ret->type = type;

    if (value)
        switch (type)
        {
            case TYPE_BYTE:     ret->val.vbyte = *(int8_t *)value;      break;
            case TYPE_CHAR:     ret->val.vchar = *(uint16_t *)value;    break;
            case TYPE_DOUBLE:   ret->val.vdouble = *(double *)value;    break;
            case TYPE_FLOAT:    ret->val.vfloat = *(float *)value;      break;
            case TYPE_INT:      ret->val.vint = *(int32_t *)value;      break;
            case TYPE_LONG:     ret->val.vlong = *(int64_t *)value;     break;
            case TYPE_SHORT:    ret->val.vshort = *(int16_t *)value;    break;
            case TYPE_BOOL:     ret->val.vbool = *(bool *)value;        break;

            case TYPE_OREF:
            case TYPE_AREF:
                                ret->val.vref = (Object *)value;
                                ret->val.vref->lock++; // lock-- in freeOperand
                                break;
            default:
                                warnx("newOperand: no idea how to create type: %x", type);
                                abort();
                                break;
        }
    else
        switch (type)
        {
            case TYPE_NULL:
            case TYPE_VOID:
                break;
            default:
                warnx("newOperand: attempt to create %s with null value", printOpType(type));
                abort();
                break;
        }

    totalOperands++;

#ifdef DEBUG
//  printf("newOperand: @%p %s %s\n", (void *)ret, printOpType(ret->type), printOpValue(ret));
#endif

    return ret;
}

static uint32_t freeAndNullOperand(Operand **o)
{
    uint32_t ret = 9999;

    assert(o != NULL);
    if (*o) {
        ret = freeOperand(*o);
        *o = NULL;
    }

    return ret;
}

static uint32_t freeOperand(Operand *o)
{
    uint32_t ret = 99999;

    assert(o != NULL);

    if (o) {
#ifdef DEBUG
//      printf("freeOperand: @%p %s %s\n", (void *)o, printOpType(o->type), printOpValue(o));
#endif
        assert(o->freed == 0);
        switch(o->type)
        {
            case TYPE_AREF:
            case TYPE_OREF:
                if (o->val.vref) {
                    if (o->val.vref->lock == 0)
                        warnx("freeOperand: attempting to free an unlocked Object");
                    else
                        ret = --o->val.vref->lock;
                    o->val.vref = NULL;

                } else
                    ret = 999999;
                /* FALLTHRU */
            default:
                o->type = TYPE_NULL;
                o->freed = 1;
                free(o);
                totalOperands--;
                break;
        }
    }


    return ret;
}

static Operand *filedescriptor_sysClose(Thread *thread, ClassFile *cls,
        Object *this)
{
    Frame *cur_frame = currentFrame(thread);
    int ret,fd;

    if ( (ret = close(fd = cur_frame->local[1]->val.vint)) == -1 )
        warn("filedescriptor_sysClose: close: %d", fd);

    return NULL;
}

static Operand *file_access(Thread *thread, ClassFile *cls, Object *this)
{
    const Field *fld = NULL;
    if ((fld = findFieldByName(this, "pathname")) == NULL)
        return throw(thread, "java.lang.VirtualMachineError", NULL, thread->pc);

    const Object *pathname_str = fld->op->val.vref;
    if ((fld = findFieldByName(pathname_str, "string")) == NULL)
        return throw(thread, "java.lang.RuntimeException", NULL, thread->pc);

    const Object *pathname_ch = fld->op->val.vref;
    uint16_t *chars = NULL;
    uint32_t len = pathname_ch->data.array.len;

    if ((chars = calloc(len, sizeof(uint16_t))) == NULL)
        return throw(thread, "java.lang.RuntimeException", NULL, thread->pc);

    for (ssize_t i = 0; i < pathname_ch->data.array.len; i++)
        chars[i] = pathname_ch->data.array.data[i]->val.vchar;

    char *ascii = parseUtf16ToAscii(chars, len);
    free(chars);
    chars = NULL;


    fld = findFieldByName(this, "canRead");
    fld->op->val.vbool = access(ascii, R_OK) ? 0 : 1;

    fld = findFieldByName(this, "canExecute");
    fld->op->val.vbool = access(ascii, X_OK) ? 0 : 1;

    fld = findFieldByName(this, "canWrite");
    fld->op->val.vbool = access(ascii, W_OK) ? 0 : 1;

    return NULL;
}

static Operand *file_stat(Thread *thread, ClassFile *cls, Object *this)
{
    const Field *fld = NULL;
    if ((fld = findFieldByName(this, "pathname")) == NULL)
        return throw(thread, "java.lang.VirtualMachineError", NULL, thread->pc);

    const Object *pathname_str = fld->op->val.vref;
    if ((fld = findFieldByName(pathname_str, "string")) == NULL)
        return throw(thread, "java.lang.RuntimeException", NULL, thread->pc);

    const Object *pathname_ch = fld->op->val.vref;
    uint16_t *chars = NULL;
    uint32_t len = pathname_ch->data.array.len;

    if ((chars = calloc(len, sizeof(uint16_t))) == NULL)
        return throw(thread, "java.lang.RuntimeException", NULL, thread->pc);

    for (ssize_t i = 0; i < pathname_ch->data.array.len; i++)
        chars[i] = pathname_ch->data.array.data[i]->val.vchar;

    char *ascii = parseUtf16ToAscii(chars, len);
    free(chars);
    chars = NULL;

    struct stat sb;
    if (stat(ascii, &sb) == -1)
        return throw(thread, "java.lang.RuntimeException", NULL, thread->pc);

    if ((fld = findFieldByName(this, "isFile")) == NULL)
        return throw(thread, "java.lang.VirtualMachineError", NULL, thread->pc);
    fld->op->val.vbool = !S_ISDIR(sb.st_mode);

    if ((fld = findFieldByName(this, "isDirectory")) == NULL)
        return throw(thread, "java.lang.VirtualMachineError", NULL, thread->pc);
    fld->op->val.vbool = S_ISDIR(sb.st_mode);

    if ((fld = findFieldByName(this, "isHidden")) == NULL)
        return throw(thread, "java.lang.VirtualMachineError", NULL, thread->pc);
    fld->op->val.vbool = 0;

    if ((fld = findFieldByName(this, "lastModified")) == NULL)
        return throw(thread, "java.lang.VirtualMachineError", NULL, thread->pc);
    fld->op->val.vlong = sb.st_mtime * 1000L;

    if ((fld = findFieldByName(this, "length")) == NULL)
        return throw(thread, "java.lang.VirtualMachineError", NULL, thread->pc);
    fld->op->val.vlong = sb.st_size;

    return NULL;
}

static Operand *filedescriptor_sysOpen(Thread *thread, ClassFile *cls,
        Object *this)
{
    Frame *cur_frame = currentFrame(thread);
    int ret;

    if (cur_frame->local[1]->type != TYPE_OREF)
        return throw(thread, "java.lang.VirtualMachineError", NULL, thread->pc);

    const Object *str = cur_frame->local[1]->val.vref;
    const Field *fld = NULL;

    if ((fld = findFieldByName(str, "string")) == NULL) {
        return throw(thread, "java.lang.RuntimeException", NULL, thread->pc);
    }

    const Object *name = fld->op->val.vref;

    uint16_t *chars = NULL;
    uint32_t len = name->data.array.len;

    if ((chars = calloc(len, sizeof(uint16_t))) == NULL)
        return throw(thread, "java.lang.RuntimeException", NULL, thread->pc);

    for (ssize_t i = 0; i < name->data.array.len; i++)
        chars[i] = name->data.array.data[i]->val.vchar;

    char *ascii = parseUtf16ToAscii(chars, len);
    free(chars);
    chars = NULL;

    int wr = cur_frame->local[2]->val.vbool;

    if ((ret = open(ascii, wr ? O_WRONLY|O_APPEND : O_RDONLY)) == -1 ) {
        warn("filedescriptor_sysOpen: open: %s", ascii);
        free(ascii);
        return throw(thread, "java.io.IOException", NULL, thread->pc);
    }
    free(ascii);

    cur_frame->ret = newOperand(TYPE_INT, &ret);

    return NULL;
}

static Operand *fileinputstream_readint(Thread *thread, ClassFile *cls,
        Object *this)
{
    Frame *cur_frame = currentFrame(thread);

    const Field *fld = findFieldByName(this, "fdObj");
    if (fld == NULL) {
        warnx("fileoutputstream_readint: can't find fdObj");
        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, thread->pc);
    }

    const Object *fdObj = fld->op->val.vref;
    if (fdObj == NULL) {
        warnx("fileoutputstream_readint: fdObj is NULL");
        return throw(thread, "java.lang.IOException", NULL, thread->pc);
    }

    fld = findFieldByName(fdObj, "fd");
    const int fd = fld->op->val.vint;

    uint8_t r;
    int32_t a;
    ssize_t len;

    if ((len = read(fd, &r, 1)) == -1) {
        warn("fileoutputstream_readint: %d", fd);
        return throw(thread, "java.io.IOException", ERR_AT, thread->pc);
    }

    if (len == 0)
        a = -1;
    else
        a = r;

    cur_frame->ret = newOperand(TYPE_INT, &a);

    return NULL;
}

static Operand *fileoutputstream_writeb(Thread *thread, ClassFile *cls,
        Object *this)
{
    const Frame *cur_frame = currentFrame(thread);
    assert(cur_frame != NULL);
    
    const Field *fld = findFieldByName(this, "fdObj");
    if (fld == NULL) {
        warnx("fileoutputstream_writeb: can't find fdObj");
        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, thread->pc);
    }

    const Object *fdObj = fld->op->val.vref;
    if (fdObj == NULL) {
        warnx("fileoutputstream_writeb: fdObj is NULL");
        return throw(thread, "java.lang.IOException", NULL, thread->pc);
    }

    fld = findFieldByName(fdObj, "fd");
    assert(fld != NULL);
    const int fd = fld->op->val.vint;

    uint8_t a = (uint8_t)(cur_frame->local[1]->val.vint & 0xFF);
    if (write(fd, &a, 1) == -1)
        return throw(thread, "java.io.IOException", ERR_AT, thread->pc);

    return NULL;
}

static Operand *fileoutputstream_writebarray(Thread *thread, ClassFile *cls,
        Object *this)
{
    Frame *cur_frame = currentFrame(thread);
    assert(cur_frame != NULL);
    const Field *fld = findFieldByName(this, "fdObj");

    Object *array = NULL;
    Object *fdObj = NULL;
    int fd = -1;
    int8_t *buf = NULL;

    if (fld == NULL || fld->op == NULL || fld->op->type == TYPE_NULL)
        goto fail;

    fdObj = fld->op->val.vref;
    fld = findFieldByName(fdObj, "fd");
    assert(fld != NULL);

    fd = fld->op->val.vint;
    if (fd == -1 ||
            cur_frame->local[1] == NULL ||
            cur_frame->local[1]->type == TYPE_NULL) {
        warnx("fileoutputstream_writebarray: checks failed");
        goto fail;
    }

    array = cur_frame->local[1]->val.vref;
    if (array->data.array.len == 0) {
        warnx("fileoutputstream_writebarray: trying to write 0 length array");
        return NULL;
    }

    if ((buf = malloc(array->data.array.len)) == NULL) {
        warn("malloc");
        goto fail;
    }

    for (uint32_t i = 0; i < array->data.array.len; i++)
    {
        Operand *op = array->data.array.data[i];
        if (!op)
            buf[i] = 0;
        else {
            if (op->type != TYPE_BYTE) {
                warnx("fileoutputstream_writebarray: not TYPE_BYTE or NULL: %d [%d/%d]",
                        op ? op->type : -1,
                        i,
                        array->data.array.len);
                goto fail;
            }
            buf[i] = op->val.vbyte;
        }
    }
    if (write(fd, buf, array->data.array.len) == -1) {
        warn("fileoutputstream_writebarray");
        goto fail;
    }

    free(buf);
    return NULL;
fail:
    warnx("fileoutputstream_writebarray");
    if (buf) free(buf);
    return throw(thread, "java.io.IOException", ERR_AT, thread->pc);
}

/* NIO native structures */

struct nio_filechannel_native {
	int fd;
};

static Operand *nio_filechannel_init(Thread *thread, Object *this)
{
	Frame *cur_frame;
	struct nio_filechannel_native *native;
	const Field *fld;
	Object *fdObj = NULL;
	int fd = -1;
	Operand *ret;

	cur_frame = currentFrame(thread);
	assert(cur_frame != NULL);

	fld = findFieldByName(this, "fdObj");
	assert(fld != NULL && fld->op != NULL);
	if (fld->op->type == TYPE_NULL)
		goto fail;

	fdObj = fld->op->val.vref;
	fld = findFieldByName(this, "fd");
	assert(fld != NULL && fld->op != NULL);

	fd = fld->op->val.vint;
	if (fd == -1)
		goto fail;

	if ((native = this->data.obj.native_data = calloc(1, sizeof(struct nio_filechannel_native))) == NULL) {
		warn("nio_filechannel_init: native_data: calloc");
		goto fail;
	}

	this->data.obj.native_free = free;

	const method_info *mi_put;
	mi_put = findMethodByClass(thread, this->class, "put", "([B)Ljava.nio.ByteBuffer;");

	const method_info *mi_remaining;
	mi_remaining = findMethodByClass(thread, this->class, "remaining", "(V)I");

	push(cur_frame, newOperand(TYPE_OREF, this));
	ret = invokeMethod(thread, mi_remaining, true, thread->pc);

	if (ret->type != TYPE_INT)
		goto fail;

	/* in java we would do:
	 *
	 * int fd = fdObj.fd;
	 * byte[] tmp = new byte[remaining()];
	 * int rd = read(fd, tmp, tmp.length); // read(2)
	 * if (rd == -1) return -1;
	 * if (rd == 0)  return 0;
	 * put(rd == tmp.length ? tmp : Arrays.copyOf(tmp, rd));
	 *
	 */

	/* read the stuff into something? */
	/* invoke put ? */
	/* maybe if the ByteBuffer is 'direct' then cheat ? */

	return ret;
fail:
	if (ret)
		freeAndNullOperand(&ret);
	return throw(thread, "java.io.IOException", ERR_AT, thread->pc);
}

static Operand *nio_implfilechannel_read(Thread *thread, ClassFile *cls,
		Object *this)
{
	Operand *ret = NULL;

	if (!this->data.obj.native_data)
		if ((ret = nio_filechannel_init(thread, this)) != NULL)
			return ret;
}

static Object *getClassObject(Thread *thread, ClassFile *cls)
{
    int rc;
    char buf[BUFSIZ];
    Object *ret = NULL;

    if (cls->class_object) return cls->class_object;

    if ((rc = pthread_rwlock_wrlock(&cls->rwlock)) != 0) {
        strerror_r(rc, buf, sizeof(buf));
        return NULL;
    }

    if ((ret = cls->class_object)) goto done;

    ret = cls->class_object = createClass(thread, cls);

done:
    pthread_rwlock_unlock(&cls->rwlock);
    return ret;
}

static Operand *javalangmath_sin(Thread *thread, ClassFile *cls)
{
    Frame *cur_frame = currentFrame(thread);
    const Operand *a = cur_frame->local[0];

    double r = sin(a->val.vdouble);

    cur_frame->ret = newOperand(TYPE_DOUBLE, &r);

    return NULL;
}

static Operand *javalangmath_sinh(Thread *thread, ClassFile *cls)
{
    Frame *cur_frame = currentFrame(thread);
    const Operand *a = cur_frame->local[0];

    double r = sinh(a->val.vdouble);

    cur_frame->ret = newOperand(TYPE_DOUBLE, &r);

    return NULL;
}

static Operand *javalangmath_cos(Thread *thread, ClassFile *cls)
{
    Frame *cur_frame = currentFrame(thread);
    const Operand *a = cur_frame->local[0];

    double r = cos(a->val.vdouble);

    cur_frame->ret = newOperand(TYPE_DOUBLE, &r);

    return NULL;
}

static Operand *javalangmath_cosh(Thread *thread, ClassFile *cls)
{
    Frame *cur_frame = currentFrame(thread);
    const Operand *a = cur_frame->local[0];

    double r = cosh(a->val.vdouble);

    cur_frame->ret = newOperand(TYPE_DOUBLE, &r);

    return NULL;
}

static Operand *javalangmath_sqrt(Thread *thread, ClassFile *cls)
{
    Frame *cur_frame = currentFrame(thread);
    const Operand *a = cur_frame->local[0];

    double r = sqrt(a->val.vdouble);

    cur_frame->ret = newOperand(TYPE_DOUBLE, &r);

    return NULL;
}

static Operand *javalangmath_atan2(Thread *thread, ClassFile *cls)
{
    Frame *cur_frame = currentFrame(thread);
    const Operand *b = cur_frame->local[1];
    const Operand *a = cur_frame->local[0];

    double r = atan2(a->val.vdouble, b->val.vdouble);

    cur_frame->ret = newOperand(TYPE_DOUBLE, &r);

    return NULL;
}

static Operand *javalangmath_atan(Thread *thread, ClassFile *cls)
{
    Frame *cur_frame = currentFrame(thread);
    const Operand *a = cur_frame->local[0];

    double r = atan(a->val.vdouble);

    cur_frame->ret = newOperand(TYPE_DOUBLE, &r);

    return NULL;
}

static Operand *javalangmath_acos(Thread *thread, ClassFile *cls)
{
    Frame *cur_frame = currentFrame(thread);
    const Operand *a = cur_frame->local[0];

    double r = acos(a->val.vdouble);

    cur_frame->ret = newOperand(TYPE_DOUBLE, &r);

    return NULL;
}

static Operand *javalangmath_asin(Thread *thread, ClassFile *cls)
{
    Frame *cur_frame = currentFrame(thread);
    const Operand *a = cur_frame->local[0];

    double r = asin(a->val.vdouble);

    cur_frame->ret = newOperand(TYPE_DOUBLE, &r);

    return NULL;
}

static Operand *javalangmath_log(Thread *thread, ClassFile *cls)
{
    Frame *cur_frame = currentFrame(thread);
    const Operand *a = cur_frame->local[0];

    double r = log(a->val.vdouble);

    cur_frame->ret = newOperand(TYPE_DOUBLE, &r);

    return NULL;
}

static Operand *javalangmath_log10(Thread *thread, ClassFile *cls)
{
    Frame *cur_frame = currentFrame(thread);
    const Operand *a = cur_frame->local[0];

    double r = log10(a->val.vdouble);

    cur_frame->ret = newOperand(TYPE_DOUBLE, &r);

    return NULL;
}

static Operand *javalangmath_log1p(Thread *thread, ClassFile *cls)
{
    Frame *cur_frame = currentFrame(thread);
    const Operand *a = cur_frame->local[0];

    double r = log1p(a->val.vdouble);

    cur_frame->ret = newOperand(TYPE_DOUBLE, &r);

    return NULL;
}


static Operand *javalangmath_exp(Thread *thread, ClassFile *cls)
{
    Frame *cur_frame = currentFrame(thread);
    Operand *a = cur_frame->local[0];

    double r = exp(a->val.vdouble);

    cur_frame->ret = newOperand(TYPE_DOUBLE, &r);

    return NULL;
}

static Operand *javalangmath_hypot(Thread *thread, ClassFile *cls)
{
    Frame *cur_frame = currentFrame(thread);
    Operand *b = cur_frame->local[1];
    Operand *a = cur_frame->local[0];

    double r = hypot(a->val.vdouble, b->val.vdouble);

    cur_frame->ret = newOperand(TYPE_DOUBLE, &r);

    return NULL;
}

static Operand *javalangdouble_doubleToRawLongBits(Thread *thread,
        ClassFile *cls)
{
    Frame *cur_frame = currentFrame(thread);
    Operand *a = cur_frame->local[0];

    double val = a->val.vdouble;
    long ret = (long)val;
    cur_frame->ret = newOperand(TYPE_LONG, &ret);
    return NULL;
}

static Operand *javalangfloat_floatToRawIntBits(Thread *thread,
        ClassFile *cls)
{
    Frame *cur_frame = currentFrame(thread);
    Operand *a = cur_frame->local[0];

    float val = a->val.vfloat;
    int ret = (int)val;
    cur_frame->ret = newOperand(TYPE_INT, &ret);
    return NULL;
}

static Operand *javalangdouble_longBitsToDouble(Thread *thread,
        ClassFile *cls)
{
    Frame *cur_frame = currentFrame(thread);
    Operand *a = cur_frame->local[0];

    long val = a->val.vlong;
    double ret = (double)val;
    cur_frame->ret = newOperand(TYPE_DOUBLE, &ret);
    return NULL;
}

static Operand *javalangfloat_intBitsToFloat(Thread *thread,
        ClassFile *cls)
{
    Frame *cur_frame = currentFrame(thread);
    Operand *a = cur_frame->local[0];

    int val = a->val.vint;
    float ret = (float)val;
    cur_frame->ret = newOperand(TYPE_FLOAT, &ret);
    return NULL;
}


static Operand *javalangobject_getClass(Thread *thread, ClassFile *cls,
        Object *this)
{
    Frame *cur_frame = currentFrame(thread);
    //printf("getClass: %s @ %p\n", this->class->this_class->d.class.name->d.utf8.ascii, this->class);
    cur_frame->ret = newOperand(TYPE_OREF, getClassObject(thread,
                (ClassFile *)this->class));
    //printf("getClass: ret: %p\n", cur_frame->ret->val.vref);

    return NULL;
}

static Operand *javalangobject_hashCode(Thread *thread, ClassFile *cls,
        Object *this)
{
    Frame *cur_frame = currentFrame(thread);
    int32_t hash = (int32_t)((int64_t)this);
    cur_frame->ret = newOperand(TYPE_INT, &hash);
    return NULL;
}

static Operand *javalangthread_nativeStart(Thread *thread, ClassFile *cls,
        Object *this)
{
    //Frame *cur_frame = currentFrame(thread);
    //Operand *op_th = cur_frame->local[0];
    Thread *newth = newThread((JVM *)thread->jvm, this);
    startThread(newth);
    return NULL;
}

static Operand *javalangthread_sleep(Thread *thread, ClassFile *cls)
{
    Frame *cur_frame = currentFrame(thread);

    const long millis = cur_frame->local[0]->val.vlong;
    const int nanos = cur_frame->local[1]->val.vint;

    const struct timespec rqtp = {
        .tv_sec = millis / 1000,
        .tv_nsec = (1000000L * (millis % 1000)) + nanos
    };

    int rc;

    if ((rc = nanosleep(&rqtp, NULL))) {
        err(EXIT_FAILURE, "javalangthread_sleep: nanosleep");
    }

    return NULL;
}

static Operand *javalangthread_currentThread(Thread *thread, ClassFile *cls)
{
    Frame *cur_frame = currentFrame(thread);

    if (pthread_rwlock_rdlock(&thread->rwlock))
        err(EXIT_FAILURE, "pthread_rwlock_rdlock");

    push(cur_frame, newOperand(TYPE_OREF, thread->thread));

    pthread_rwlock_unlock(&thread->rwlock);
    return NULL;
}

static Operand *javalangreflectarray_newInstance(Thread *thread, ClassFile *cls)
{
    return NULL;
}

static Operand *javalangsystem_exit(Thread *thread, ClassFile *cls)
{
    Frame *cur_frame = currentFrame(thread);
    Operand *rc_op = pop(cur_frame);
    if (rc_op) {
        const int rc = rc_op->val.vint;
        freeAndNullOperand(&rc_op);
        exit(rc); // FIXME exit other threads?
    } else
        exit(EXIT_SUCCESS);
}

static Operand *javalangsystem_currentTimeMillis(Thread *thread, ClassFile *cls)
{
    Frame *cur_frame = currentFrame(thread);

    struct timeval tv;

    if (gettimeofday(&tv, NULL))
        err(EXIT_FAILURE, ERR_AT "gettimeofday");

    long ret = tv.tv_sec * 1000;
    ret += (tv.tv_usec / 1000);

    cur_frame->ret = newOperand(TYPE_LONG, &ret);

    return NULL;
}

static Operand *javalangclassloader_findClass(Thread *thread, ClassFile *cls, Object *this)
{
    Frame *cur_frame = currentFrame(thread);
    const Field *fld = NULL;
    Object *str = getLocal(cur_frame, 1)->val.vref;

    if ((fld = findFieldByName(str, "string")) == NULL) {
        warnx("Cannot find string");
        return NULL;
    }

    const Object *name = fld->op->val.vref;
    uint16_t *chars = NULL;

    if ((chars = calloc(name->data.array.len, sizeof(uint16_t))) == NULL)
        return NULL;

    for (ssize_t i = 0; i < name->data.array.len; i++)
        chars[i] = name->data.array.data[i]->val.vchar;

    char *ascii = parseUtf16ToAscii(chars, name->data.array.len);
    free(chars);
    chars = NULL;

    for(size_t i = 0; i < strlen(ascii); i++)
        if(ascii[i] == '.') ascii[i]='/';

    const ClassFile *cf = findClass(thread, ascii);
    free(ascii);
    ascii = NULL;

    if (cf) {
        cur_frame->ret = newOperand(TYPE_OREF, getClassObject(thread,
                    (ClassFile *)cf));
    } else {
        cur_frame->ret = newOperand(TYPE_NULL, NULL);
    }

    return NULL;
}

static Operand *javalangclassloader_findLoadedClass(Thread *thread,
        ClassFile *cls, Object *this)
{
    Frame *cur_frame = currentFrame(thread);
    const Field *fld = NULL;
    Object *str = getLocal(cur_frame, 1)->val.vref;

    if ((fld = findFieldByName(str, "string")) == NULL) {
        warnx("Cannot find string");
        return NULL;
    }

    const Object *name = fld->op->val.vref;
    uint16_t *chars = NULL;

    if ((chars = calloc(name->data.array.len, sizeof(uint16_t))) == NULL)
        return NULL;

    for (ssize_t i = 0; i < name->data.array.len; i++)
        chars[i] = name->data.array.data[i]->val.vchar;

    char *ascii = parseUtf16ToAscii(chars, name->data.array.len);
    free(chars);
    chars = NULL;

    const ClassFile *cf = findClass2(thread, ascii, false);
    free(ascii);
    ascii = NULL;

    if (cf) {
        cur_frame->ret = newOperand(TYPE_OREF, getClassObject(thread,
                    (ClassFile *)cf));
    } else {
        cur_frame->ret = newOperand(TYPE_NULL, NULL);
    }

    return NULL;
}

static Operand *javalangthrowable_fillInStackTrace(Thread *thread,
        ClassFile *cls, Object *this)
{
    Frame *cur_frame = currentFrame(thread);

    ClassFile *ste_c = findClass(thread, "java.lang.StackTraceElement");
    if (ste_c == NULL)
        return NULL;

    Field *fld = NULL;

    if ((fld = findFieldByName(this, "stackTrace")) == NULL)
        return NULL;

    freeAndNullOperand(&fld->op);

    Object *trace = newArray(thread, TYPE_OREF, thread->frame_free, ste_c);
    fld->op = newOperand(TYPE_AREF, trace);

    int arrpos = 0;
    method_info *mi = findMethodByClass(thread, ste_c, "<init>",
            "(Ljava.lang.String;Ljava.lang.String;Ljava.lang.String;I)V");
    Operand *ex_opr;
    Object *str = NULL;

    for (ssize_t sp = thread->frame_free - 2; sp >= 2; sp--, arrpos++)
    {
        Frame *fr = thread->stack[sp];
        if (fr == NULL) {
            warnx("fr == NULL @ %ld", sp);
            continue;
        }

        Object *ste_o = newObject(thread, ste_c);
        if (!push(cur_frame, newOperand(TYPE_OREF, ste_o))) {
            warnx("unable to push: " ERR_AT);
            return NULL;
        }

        /* declaringClass */
        if ((ex_opr = newString(thread,
                        fr->class ?
                        fr->class->this_class->d.class.name->d.utf8.ascii :
                        "unknown",
                        &str)) != NULL)
            return ex_opr;
        if (!push(cur_frame, newOperand(TYPE_OREF, str))) {
            warnx("unable to push: " ERR_AT);
            return NULL;
        }

        /* methodName */
        if ((ex_opr = newString(thread,
                        fr->mi ? fr->mi->name->d.utf8.ascii : "unknown",
                        &str)) != NULL)
            return ex_opr;
        if (!push(cur_frame, newOperand(TYPE_OREF, str))) {
            warnx("unable to push: " ERR_AT);
            return NULL;
        }

        /* fileName */
        if ((ex_opr = newString(thread,
                        (fr->class && fr->class->srcfile) ?
                        fr->class->srcfile->d.utf8.ascii :
                        "unknown",
                        &str)) != NULL)
            return ex_opr;
        if (!push(cur_frame, newOperand(TYPE_OREF, str))) {
            warnx("unable to push: " ERR_AT);
            return NULL;
        }

        int32_t line = -1;

        if (fr->mi && fr->mi->code && fr->mi->code->lineno ) {
            const Code_attribute *code = fr->mi->code;
            for (ssize_t i = 0; i < code->lineno->line_number_table_length; i++)
            {
                const line_number_t *li = &code->lineno->line_number_table[i];
                if (fr->pc >= li->start_pc && fr->pc < li->end_pc) {
                    line = li->line_number;
                    break;
                }
            }
        }

        if (line == -1) {
            warnx("No linenumber for %d: mi:%s code:%s lineno:%s", fr->pc,
                    fr->mi ? "true" : "false",
                    (fr->mi && fr->mi->code) ? "true" : "false",
                    (fr->mi && fr->mi->code && fr->mi->code->lineno) ? "true" : "false");
        }

        if (!push(cur_frame, newOperand(TYPE_INT, &line))) {
            warnx("unable to push: " ERR_AT);
            return NULL;
        }

        if ((ex_opr = invokeMethod(thread, mi, true, 0)) != NULL)
            return ex_opr;

        trace->data.array.data[arrpos] = newOperand(TYPE_OREF, ste_o);

    }

    cur_frame->ret = newOperand(TYPE_OREF, this);
    return NULL;
}

#if 0
static const internalClass tmp_Object = {
    .name = "java.lang.Object",
    .access = ACC_PUBLIC,
    .methods = {
        {.name = NULL}
    },
    .super = "java.lang.Object"
};

static const internalClass tmp_Class = {
    .name = "java.lang.Class",
    .access = ACC_PUBLIC,
    .methods = {
        {.name = NULL}
    },
    .super = "java.lang.Object"
};

static const internalClass tmp_String = {
    .name = "java.lang.String",
    .access = ACC_PUBLIC,
    .methods = {
        {.name = NULL}
    },
    .super = "java.lang.Object"
};
#endif

static internalClass java_lang_Throwable = {
    .name = "java.lang.Throwable",
    .methods = {
        { .name = "fillInStackTrace", .desc = "()Ljava.lang.Throwable;", .meth.mvirtual = javalangthrowable_fillInStackTrace },
        {.name = NULL}
    }
};

static internalClass java_lang_Object = {
    .name = "java.lang.Object",
    .methods = {
        { .name = "hashCode", .desc = "()I",                .meth.mvirtual = javalangobject_hashCode },
        { .name = "getClass", .desc = "()Ljava.lang.Class;",.meth.mvirtual = javalangobject_getClass },
        {.name = NULL}
    }
};

static internalClass java_io_FileOutputStream = {
    .name = "java.io.FileOutputStream",
    .methods = {
        { .name = "write", .desc = "(I)V",  .meth.mvirtual = fileoutputstream_writeb },
        { .name = "write", .desc = "([B)V", .meth.mvirtual = fileoutputstream_writebarray },
        {.name = NULL}
    }
};

static internalClass java_io_FileInputStream = {
    .name = "java.io.FileInputStream",
    .methods = {
        { .name = "read", .desc = "()I",    .meth.mvirtual = fileinputstream_readint },
        {.name = NULL}
    }
};

static internalClass java_io_File = {
    .name = "java.io.File",
    .methods = {
        { .name = "stat", .desc = "()V", .meth.mvirtual = file_stat },
        { .name = "access", .desc = "()V", .meth.mvirtual = file_access },
        {.name = NULL}
    }
};

static internalClass java_io_FileDescriptor = {
    .name = "java.io.FileDescriptor",
    .methods = {
        { .name = "sysClose", .desc = "(I)V",                   .meth.mvirtual = filedescriptor_sysClose },
        { .name = "sysOpen",  .desc = "(Ljava.lang.String;Z)I", .meth.mvirtual = filedescriptor_sysOpen },
        {.name = NULL}
    }
};

static internalClass java_lang_Thread = {
    .name = "java.lang.Thread",
    .methods = {
        { .name = "nativeStart",    .desc = "()V",                  .meth.mvirtual = javalangthread_nativeStart },
        { .name = "sleep",          .desc = "(JI)V",                .meth.mstatic = javalangthread_sleep },
        { .name = "currentThread",  .desc = "()Ljava.lang.Thread;", .meth.mstatic = javalangthread_currentThread },
        {.name = NULL}
    }
};

static internalClass java_lang_reflect_Array = {
    .name = "java.lang.reflect/Array",
    .methods = {
        { .name = "newInstance", .desc = "(Ljava.lang.Class;[I)Ljava.lang.Object;", .meth.mstatic = javalangreflectarray_newInstance },
        {.name = NULL}
    }
};

static internalClass java_lang_System = {
    .name = "java.lang.System",
    .methods = {
        { .name = "exit",               .desc = "(I)V", .meth.mstatic = javalangsystem_exit },
        { .name = "currentTimeMillis",  .desc = "()J",  .meth.mstatic = javalangsystem_currentTimeMillis },
        {.name = NULL}
    }
};

static internalClass java_lang_ClassLoader = {
    .name = "java.lang.ClassLoader",
    .methods = {
        { .name = "findLoadedClass",    .desc = "(Ljava.lang.String;)Ljava.lang.Class;", .meth.mvirtual = javalangclassloader_findLoadedClass },
        { .name = "findClass",          .desc = "(Ljava.lang.String;)Ljava.lang.Class;", .meth.mvirtual = javalangclassloader_findClass },
        {.name = NULL}
    }
};

static internalClass java_lang_Double = {
    .name = "java.lang.Double",
    .methods = {
        { .name = "longBitsToDouble",   .desc = "(J)D", .meth.mstatic = javalangdouble_longBitsToDouble },
        { .name = "doubleToRawLongBits",.desc = "(D)J", .meth.mstatic = javalangdouble_doubleToRawLongBits },
        {.name=NULL}
    }
};

static internalClass java_lang_Float = {
    .name = "java.lang.Float",
    .methods = {
        { .name = "intBitsToFloat",    .desc = "(I)F", .meth.mstatic = javalangfloat_intBitsToFloat    },
        { .name = "floatToRawIntBits", .desc = "(F)I", .meth.mstatic = javalangfloat_floatToRawIntBits },
        {.name=NULL}
    }
};

static internalClass java_lang_Math = {
    .name = "java.lang.Math",
    .methods = {
        { .name = "atan2",  .desc = "(DD)D",.meth.mstatic = javalangmath_atan2 },
        { .name = "hypot",  .desc = "(DD)D",.meth.mstatic = javalangmath_hypot },
        { .name = "acos",   .desc = "(D)D", .meth.mstatic = javalangmath_acos },
        { .name = "asin",   .desc = "(D)D", .meth.mstatic = javalangmath_asin },
        { .name = "atan",   .desc = "(D)D", .meth.mstatic = javalangmath_atan },
        { .name = "atan2",  .desc = "(D)D", .meth.mstatic = javalangmath_atan2 },
        { .name = "cos",    .desc = "(D)D", .meth.mstatic = javalangmath_cos },
        { .name = "cosh",   .desc = "(D)D", .meth.mstatic = javalangmath_cosh },
        { .name = "sin",    .desc = "(D)D", .meth.mstatic = javalangmath_sin },
        { .name = "sinh",   .desc = "(D)D", .meth.mstatic = javalangmath_sinh },
        { .name = "exp",    .desc = "(D)D", .meth.mstatic = javalangmath_exp },
        { .name = "log",    .desc = "(D)D", .meth.mstatic = javalangmath_log },
        { .name = "log10",  .desc = "(D)D", .meth.mstatic = javalangmath_log10 },
        { .name = "log1p",  .desc = "(D)D", .meth.mstatic = javalangmath_log1p },
        { .name = "sqrt",   .desc = "(D)D", .meth.mstatic = javalangmath_sqrt },
        {.name=NULL}
    }
};

static internalClass java_nio_ImplFileChannel = {
	.name = "java.nio.ImplFileChannel",
	.methods = {
		{ .name = "read", .desc = "(Ljava.nio.ByteBuffer;J)J", .meth.mvirtual = nio_implfilechannel_read     },
		//{ .name = "read", .desc = "(Ljava.nio.ByteBuffer;)I",  .meth.mvirtual = nio_implfilechannel_read_pos },
		{.name=NULL}
	}
};

#if 0
ClassFile *buildInternalClass(internalClass *ic)
{
    uint16_t cp_ptr = 1;

    cp_info *tmp_utf8 = NULL;
    cp_info *tmp_class = NULL;
    const char *super = NULL;
    field_info *tmp_fi = NULL;
    method_info *tmp_mi = NULL;
    cp_info **tmp_cp = NULL;

    ClassFile *ret = calloc(1, sizeof(ClassFile));
    if (ret == NULL) goto fail;

    ret->access_flags = ic->access;
    if ((ret->constant_pool = calloc(1000, sizeof(cp_info *))) == NULL) goto fail;

    if ((tmp_utf8 = calloc(1, sizeof(cp_info))) == NULL) goto fail;
    tmp_utf8->tag = CONSTANT_Utf8;
    tmp_utf8->d.utf8.length = (uint16_t)(strlen(ic->name));
    if ((tmp_utf8->d.utf8.bytes = calloc(1, tmp_utf8->d.utf8.length + 1U)) == NULL) goto fail;

    strcpy((char *)tmp_utf8->d.utf8.bytes, ic->name);
    fixupCpUtf8(tmp_utf8);

    if ((tmp_class = calloc(1, sizeof(cp_info))) == NULL) goto fail;
    tmp_class->tag = CONSTANT_Class;
    tmp_class->d.class.name = tmp_utf8;

    ret->this_class_idx = cp_ptr;
    ret->this_class = tmp_class;
    ret->constant_pool[cp_ptr++] = tmp_class;
    ret->constant_pool[cp_ptr++] = tmp_class->d.class.name;

    if ((ret->methods = calloc(MAX_METHODS, sizeof(method_info *))) == NULL)
        goto fail;

    if (ic->super == NULL && strcmp(ic->name, "java.lang.Object")) {
        super = strdup("java.lang.Object");
    } else
        super = strdup(ic->super);

    if (super)
        ret->super_name = strdup(super);

    for (int i = 0; i < MAX_METHODS && ic->methods[i].name; i++)
    {
        tmp_mi = calloc(1, sizeof(method_info));
        if (tmp_mi == NULL) goto fail;

        tmp_mi->access_flags = ic->methods[i].access;

        cp_info *name = calloc(1, sizeof(cp_info));
        name->d.utf8.length = (uint16_t)strlen(ic->methods[i].name);
        name->tag = CONSTANT_Utf8;
        if ((name->d.utf8.bytes = calloc(1, name->d.utf8.length + 1U)) == NULL) goto fail;
        if ((tmp_mi->name = name) == NULL) goto fail;
        strcpy((char *)name->d.utf8.bytes, ic->methods[i].name);

        cp_info *descriptor = calloc(1, sizeof(cp_info));
        descriptor->d.utf8.length = (uint16_t)strlen(ic->methods[i].desc);
        descriptor->tag = CONSTANT_Utf8;
        if ((name->d.utf8.bytes = calloc(1, descriptor->d.utf8.length + 1U)) == NULL) goto fail;
        if ((tmp_mi->descriptor = descriptor) == NULL) goto fail;
        strcpy((char *)descriptor->d.utf8.bytes, ic->methods[i].desc);

        tmp_mi->name_index = cp_ptr;
        ret->constant_pool[cp_ptr++] = tmp_mi->name;

        tmp_mi->descriptor_index = cp_ptr;
        ret->constant_pool[cp_ptr++] = tmp_mi->descriptor;

        fixupCpUtf8(name);
        fixupCpUtf8(descriptor);

        if (tmp_mi->access_flags & ACC_STATIC) {
            if ((tmp_mi->nativeMethod.mstatic = ic->methods[i].meth.mstatic) != NULL)
                tmp_mi->access_flags |= ACC_NATIVE;
        } else {
            if ((tmp_mi->nativeMethod.mvirtual = ic->methods[i].meth.mvirtual) != NULL)
                tmp_mi->access_flags |= ACC_NATIVE;
        }

        tmp_mi->class = ret;

        if (!parseMethodDescriptor(&tmp_mi->desc, ic->methods[i].desc)) goto fail;

        ret->methods[ret->methods_count++] = tmp_mi;
        tmp_mi = NULL;
    }

    if ((ret->fields = calloc(MAX_FIELDS, sizeof(field_info *))) == NULL)
        goto fail;


    for (int i = 0; i < MAX_FIELDS && ic->fields[i].name; i++)
    {
        tmp_fi = calloc(1, sizeof(field_info));
        if (tmp_fi == NULL) goto fail;

        tmp_fi->class = ret;
        tmp_fi->access_flags = ic->fields[i].access;

        cp_info *name = calloc(1, sizeof(cp_info));
        name->tag = CONSTANT_Utf8;
        name->d.utf8.length = (uint16_t)strlen(ic->fields[i].name);
        if ((tmp_fi->name = name) == NULL) goto fail;
        if ((name->d.utf8.bytes = calloc(1, name->d.utf8.length + 1U)) == NULL) goto fail;
        strcpy((char *)name->d.utf8.bytes, ic->fields[i].name);

        cp_info *descriptor = calloc(1, sizeof(cp_info));
        descriptor->tag = CONSTANT_Utf8;
        descriptor->d.utf8.length = (uint16_t)strlen(ic->fields[i].desc);
        if ((tmp_fi->descriptor = descriptor) == NULL) goto fail;
        if ((descriptor->d.utf8.bytes = calloc(1, descriptor->d.utf8.length + 1U)) == NULL) goto fail;
        strcpy((char *)descriptor->d.utf8.bytes, ic->fields[i].desc);

        tmp_fi->name_index = cp_ptr;
        ret->constant_pool[cp_ptr++] = tmp_fi->name;

        tmp_fi->descriptor_index = cp_ptr;
        ret->constant_pool[cp_ptr++] = tmp_fi->descriptor;

        fixupCpUtf8(name);
        fixupCpUtf8(descriptor);

        if (parseParamDescriptor(&tmp_fi->desc, ic->fields[i].desc) == NULL) goto fail;

        ret->fields[ret->fields_count++] = tmp_fi;
        tmp_fi = NULL;
    }

    ret->constant_pool_count = cp_ptr;
    tmp_cp = realloc(ret->constant_pool, sizeof(cp_info *) * (cp_ptr + 1U));
    if (tmp_cp == NULL) {
        warn("buildInternalClass");
        goto fail;
    }
    ret->constant_pool = tmp_cp;
    return ret;
fail:
    if (ret) {
        if (tmp_fi) {
            free(tmp_fi);
            tmp_fi = NULL;
        }
        if (tmp_mi) {
            free(tmp_mi);
            tmp_mi = NULL;
        }
        free(ret);
        ret = NULL;
    }
    warnx("Failed to process internalClass");
    return NULL;
}
#endif

#ifdef DEBUG
static void dumpConstantPool(const int me, const cp_info *cp)
{
    printf(" [%2d]: ", me);

    switch(cp->tag)
    {
        case CONSTANT_Class:
            {
                printf("class:  name[%d]="ANSI_BRED"%s"ANSI_RESET,
                        cp->d.class.name_index,
                        cp->d.class.name->d.utf8.ascii
                        //cp->d.class.original_name
                        );
            }
            break;
        case CONSTANT_Fieldref:
            {
                const cp_Fieldref_info *ent = &cp->d.fr;
                printf("fieldr: class[%d]=%s nametype[%d]=%s %s",
                        ent->class_index,
                        ent->class->d.class.name->d.utf8.ascii,
                        ent->name_and_type_index,
                        ent->nametype->d.nt.name->d.utf8.ascii,
                        ent->nametype->d.nt.descriptor->d.utf8.ascii
                      );
            }
            break;
        case CONSTANT_Methodref:
            {
                const cp_Fieldref_info *ent = &cp->d.fr;
                printf("methdr: class[%d]=%s nametype[%d]='%s %s'",
                        ent->class_index,
                        ent->class->d.class.name->d.utf8.ascii,
                        ent->name_and_type_index,
                        ent->nametype->d.nt.name->d.utf8.ascii,
                        ent->nametype->d.nt.descriptor->d.utf8.ascii
                      );
            }
            break;
        case CONSTANT_NameAndType:
            {
                const cp_NameAndType_info *ent = &cp->d.nt;
                printf("nm&typ: name[%d]='%s' descriptor[%d]='%s'",
                        ent->name_index,
                        ent->name->d.utf8.ascii,
                        //ent->original_name,
                        ent->descriptor_index,
                        ent->descriptor->d.utf8.ascii
                        //,ent->original_descriptor
                      );
            }
            break;
        case CONSTANT_String:
            {
                const cp_String_info *ent = &cp->d.str;
                printf("string: utf8[%d]='%s'", ent->string_index,
                        ent->string->d.utf8.ascii);
            }
            break;
        case CONSTANT_Utf8:
            {
                const cp_Utf8_info *ent = &cp->d.utf8;
                printf("utf8:   bytes='%s'", ent->ascii);
            }
            break;
        case CONSTANT_Integer:
            {
                const cp_Integer_info *ent = &cp->d.tint;
                printf("int:    c_int='%d'", ent->c_int);
            }
            break;
        default:
            printf("Unknown tag %d", cp->tag);
    }

    printf("\n");
}

static enum std_attr_e lookupPredefinedAttribute(const char *name)
{
    for (int i = 0; predefinedAttrs[i].name; i++)
    {
        if (!strcmp(name, predefinedAttrs[i].name))
            return predefinedAttrs[i].attr;
    }
    return ATTR_INVALID;
}

static void dumpAttribute(const attribute_info *ai)
{
    int val;
    printf("attr=%s ", ai->name ? ai->name->d.utf8.ascii : "<<missing name>>");
    switch((val = lookupPredefinedAttribute(ai->name->d.utf8.ascii)))
    {
        case ATTR_CODE:
            printf("max_stack=%d max_locals=%d code_length=%d attributes=%d exceptions=%d",
                    ai->d.code->max_stack,
                    ai->d.code->max_locals,
                    ai->d.code->code_length,
                    ai->d.code->attributes_count,
                    ai->d.code->exception_table_length
                  );

            if(ai->d.code->exception_table_length) {
                printf("\n");
                for (int i = 0; i < ai->d.code->exception_table_length; i++)
                {
                    const exception *ex = ai->d.code->exception_table[i];
                    printf("   [%2d]: start_pc=%d end_pc=%d handler_pc=%d "
                            "type=%s\n", i,
                            ex->start_pc,
                            ex->end_pc,
                            ex->handler_pc,
                            ex->catch_type ? ex->catch_type->d.class.name->d.utf8.ascii :
                            "<none");
                }
            }
            //printf("\n"); disasm(ai->d.code->code, ai->d.code->code_length);
            break;
        case ATTR_INNERCLASSES:
            printf("number_of_classes=%d\n", ai->d.inner->number_of_classes);
            for (int i = 0; i < ai->d.inner->number_of_classes; i++)
            {
                char buf[BUFSIZ];
                printAccessFlags(ai->d.inner->classes[i]->inner_class_access_flags, buf, BUFSIZ);
                printf("   [%2d]: inner_cls_idx=%d outer_cls_idx=%d "
                        "inner_name_idx=%d access=%s\n",
                        i,
                        ai->d.inner->classes[i]->inner_class_info_index,
                        ai->d.inner->classes[i]->outer_class_info_index,
                        ai->d.inner->classes[i]->inner_name_index,
                        buf);
            }
            return;
        case ATTR_INVALID:
            printf("<<ATTR_INVALID[%d] not supported>>", val);
            break;
        case ATTR_EXCEPTIONS:
            printf("number=%d\n", ai->d.exception->number_of_exceptions);
            for (int i = 0; i < ai->d.exception->number_of_exceptions; i++)
            {
                printf("   [%d]: exception=%s\n", i,
                        ai->d.exception->exception_index_table[i]->d.class.name->d.utf8.ascii
                        //ai->d.exception->exception_index_table[i]->d.class.original_name
                        );
            }
            break;
        case ATTR_CONSTANT_VALUE:
            printf("ref=%d %s %s",
                    ai->d.constant->constantvalue_index,
                    printOpType(ai->d.constant->op.type),
                    printOpValue(&ai->d.constant->op)
                  );
            break;
        case ATTR_STACKMAPTABLE:
        case ATTR_BOOTSTRAPMETHODS:
        case ATTR_SOURCEFILE:
        case ATTR_LINENUMBERTABLE:
        default:
            printf("<<ATTR [%s] not implemented [%d]>>", ai->name->d.utf8.ascii, val);
            break;
    }
    printf("\n");
}

static void dumpField(const int me, const field_info *fi)
{
    char buf[BUFSIZ];

    printf(" [%d]: ", me);
    printAccessFlags(fi->access_flags, buf, BUFSIZ);
    printf("access=%s name=%s descriptor=%s\n",
            buf,
            fi->name->d.utf8.ascii,
            fi->descriptor->d.utf8.ascii
          );

    for (int i = 0; i < fi->attributes_count; i++)
    {
        printf("  [%d]: ", i);
        dumpAttribute(fi->attributes[i]);
    }
}

static void dumpMethod(const int me, method_info *mi)
{
    printf(" [%2d]: ", me);
    char buf[BUFSIZ];
    printAccessFlags(mi->access_flags, buf, BUFSIZ);
    printf("access=%s name="ANSI_BRED"%s"ANSI_RESET" descriptor="ANSI_BMAG"%s"ANSI_RESET"\n",
            buf,
            mi->name->d.utf8.ascii,
            //mi->original_name,
            mi->descriptor->d.utf8.ascii
            //mi->original_descriptor
          );

    for (int i = 0; i < mi->attributes_count; i++)
    {
        printf("  [%2d]: ", i);
        dumpAttribute(mi->attributes[i]);
    }
}
#endif

/* aquires jvm->rwlock */
static bool findHeapSlot(const Thread *const thread, Object *const obj)
{
    JVM *const jvm = (JVM *)thread->jvm;
    bool ret = false;
    int rc;
    char buf[BUFSIZ];

    if ((rc = pthread_rwlock_wrlock(&jvm->rwlock))) {
        strerror_r(rc, buf, sizeof(buf));
        errx(EXIT_FAILURE, "findHeapSlot: pthread_rwlock_wrlock: %s", buf);
    }

    for (uint16_t i = 0; i < jvm->maxHeap; i++)
        if (jvm->heap[i] == NULL) {
            jvm->heap[i] = obj;
            obj->heaploc = i;

            ret = true;

            break;
        }
    
    if (!ret && jvm->maxHeap < MAX_HEAP) {
        Object **newheap;
        uint16_t newMaxHeap = (uint16_t)((jvm->maxHeap + 128U > MAX_HEAP) ? 
                MAX_HEAP : (jvm->maxHeap + 128U));

        if ((newheap = realloc(jvm->heap, sizeof(Object *) * newMaxHeap)) != NULL) {
            jvm->heap = newheap;

            for (uint16_t i = jvm->maxHeap; i < newMaxHeap; i++)
                jvm->heap[i] = NULL;

            jvm->heap[jvm->maxHeap] = obj;
            obj->heaploc = jvm->maxHeap;
            jvm->maxHeap = newMaxHeap;

            ret = true;
        } else /* realloc */
            err(EXIT_FAILURE, "findHeapSlot: calloc");
    }

    if ((rc = pthread_rwlock_unlock(&jvm->rwlock))) {
        strerror_r(rc, buf, sizeof(buf));
        errx(EXIT_FAILURE, "findHeapSlot: pthread_rwlock_unlock: %s", buf);
    }

    return ret;
}

// FIXME common code with newObject

static Object *newMultiArray(Thread *thread, uint8_t type, int32_t *sizes,
        uint8_t dim, const ClassFile *clsType)
{
    Object *ret = NULL;
    int32_t size = 0;
    char buf[BUFSIZ] = {0};
    //long nowt = 0;
    int rc;
    bool pti = false;

    if (type == TYPE_NULL) {
        warnx("newMultiArray: type is NULL");
        goto fail;
    }

    if ((ret = calloc(1, sizeof(Object)))== NULL) {
        warn("newMultiArray: calloc");
        goto fail;
    }

    if ((rc = pthread_mutex_init(&ret->monitor, NULL)) != 0) {
        strerror_r(rc, buf, sizeof(buf));
        warnx("newMultiArray: pthread_mutex_init: %s", buf);
        goto fail;
    }

    pti = true;

    if (!thread) {
        ret->heaploc = -1;
        warnx("newMultiArray: no thread");
        goto fail;
    } else if (!findHeapSlot(thread, ret)) {
        warnx("newMultiArray: findHeapSlot");
        goto fail;
    }

    ret->type = OBJ_ARRAY;
    ret->data.array.type = type;

    for (ssize_t i = 0; i < dim; i++) {
        strcat(buf, "[");
        if (sizes[i] > (1<<30) || sizes[i] < 0) {
            warnx("newMultiArray: invalid size[%lu]=%d", i, sizes[i]);
            goto fail;
        }
        size += sizes[i];
    }

    buf[dim] = (char)(type);
    buf[dim+1] = '\0';

    if (type == TYPE_OREF) {
        strcat(buf, clsType->this_class->d.class.name->d.utf8.ascii);
        strcat(buf, ";");
    } else if(type == TYPE_AREF)
        abort();

    if (isClassLoaded(thread, buf))
        ret->class = findClass(thread, buf);
    else {
        ret->class_obj = createClassForArray(thread, type, dim, clsType); // fixme
        ret->class = NULL;
    }

    ret->data.array.len = size;
    ret->data.array.dimensions = dim;
    ret->data.array.sizes = sizes;

    if (size > 0) {
        if ((ret->data.array.data = calloc(size, sizeof(Operand *))) == NULL) {
            warn("newMultiArray: calloc");
            goto fail;
        }

        /*
           if ( (type != TYPE_AREF) && (type != TYPE_OREF) && (type != TYPE_VOID) && (type != TYPE_NULL) )
           for (ssize_t i = 0; i < size; i++)
           ret->data.array.data[i] = newOperand(type, &nowt);
           */
    }
#ifdef DEBUG_CODE
    printf("newMultiArray: %s %s [",
            printOpType(type),
            clsType ?  clsType->this_class->d.class.name->d.utf8.ascii : "<n/a>");
    for (ssize_t i = 0; i < dim ; i++)
        printf("%d%s",sizes[i], i < dim-1 ? "," : "" );
    printf("]\n");
#endif
    ret->className = strdup(clsType ? clsType->this_class->d.class.name->d.utf8.ascii : "<none>");
    return ret;

fail:
    free(sizes);
    sizes = NULL;
    if (ret) {
        if (pti)
            pthread_mutex_destroy(&ret->monitor);

        if (thread && ret->heaploc != - 1) {
            if (pthread_rwlock_wrlock(&((JVM *)thread->jvm)->rwlock))
                err(EXIT_FAILURE, "newMultiArray: wrlock");
            ((JVM *)thread->jvm)->heap[ret->heaploc] = NULL;
            if (pthread_rwlock_unlock(&((JVM *)thread->jvm)->rwlock))
                err(EXIT_FAILURE, "newMultiArray: unlock");
        }
        free(ret);
        ret = NULL;
    }
    return NULL;

}

static Object *newArray(Thread *thread, uint8_t type, int32_t size,
        const ClassFile *clsType)
{
    //if (size == 0)
    //  warnx("array %c is 0 length for clsType:%s", type, clsType == NULL ? "(null)" : 
    //          clsType->this_class->d.class.name->d.utf8.ascii);

    int32_t *sizes = calloc(2, sizeof(uint32_t));
    if (sizes == NULL) {
        warn("newArray: calloc");
        return NULL;
    }

    sizes[0] = size;
    sizes[1] = 0;

    return newMultiArray(thread, type, sizes, 1U, clsType);
}

static uint16_t getSuperFieldCount(const Thread *thread, const ClassFile *const cls)
{
    assert(thread->jvm->VM == true);
    assert(cls != NULL);

    //printf("getSuperFieldCount: %s\n", cls->this_class->name->ascii);
    if(cls == NULL)
        warnx("getSuperFieldCount: class is null");
    else if(cls->super_class)
        return (uint16_t)(cls->fields_count + getSuperFieldCount(thread,
                    findClass((Thread *)thread, cls->super_name)));
    else
        return cls->fields_count;
    return 0;
}

static Object *newObject(Thread *thread, const ClassFile *const cls)
{
    uint32_t fldptr = 0;
    const ClassFile *super = cls;
    Field *fields = NULL;
    uint16_t field_count = 0;
    bool pti = false;
    int rc;
    char buf[BUFSIZ];

    assert(cls != NULL);
    assert(thread != NULL);

    if (cls == NULL) {
        warnx("newObject: NULL class");
        return NULL;
    }

    Object *ret = calloc(1, sizeof(Object));
    if (ret == NULL) {
        warn("newObject: calloc");
        goto fail;
    }

    if ((rc = pthread_mutex_init(&ret->monitor, NULL)) != 0) {
        strerror_r(rc, buf, sizeof(buf));
        warnx("newObject: unable to create mutex: %s", buf);
        goto fail;
    }

    pti = true;

    if (!thread)
        ret->heaploc = -1;
    else if (!findHeapSlot(thread, ret)) {
        warnx("newObject: no heapslot");
        __asm__("int $3");
        goto fail;
    }

    ret->type = OBJ_CLASS;
    ret->class = cls;

    field_count = getSuperFieldCount(thread, cls);

    if ((fields = ret->data.obj.fields = calloc(field_count + 1U,
                    sizeof(Field))) == NULL) {
        warn("newObject: fields calloc");
        goto fail;
    }

    ret->data.obj.num_fields = field_count;


    while(super)
    {
        for (int i = 0; i < super->fields_count; fldptr++, i++)
        {
            uint32_t nowt = 0;

            fields[fldptr].field_info = super->fields[i];
            switch(super->fields[i]->desc.baseType)
            {
                case TYPE_OREF:
                case TYPE_AREF:
                    fields[fldptr].op = newOperand(TYPE_NULL, NULL);
                    break;
                default:
                    if (super->fields[i]->desc.array)
                        fields[fldptr].op = newOperand(TYPE_NULL, NULL);
                    else
                        fields[fldptr].op =
                            newOperand(super->fields[i]->desc.baseType, &nowt);
                    break;
            }
        }

        if (super->super_class) {
            super = findClass(thread, super->super_name);
        } else
            super = NULL;
    }

#ifdef DEBUG_CODE
    printf("newObject: OBJ_CLASS heap_id=%d class=%s\n",
            ret->heaploc,
            cls->this_class->d.class.name->d.utf8.ascii);
#endif

    if ((ret->className = strdup(cls->this_class->d.class.name->d.utf8.ascii)) == NULL)
        warn("newObject: className strdup");

    return ret;
fail:
    warnx("Unable to create Object from %s",
            cls->this_class->d.class.name->d.utf8.ascii);
    if (ret) {
        if (pti) {
            pthread_mutex_destroy(&ret->monitor);
        }
        if (ret->type == OBJ_CLASS) {
            if (ret->data.obj.fields) {
                free(ret->data.obj.fields);
                ret->data.obj.fields = NULL;
            }
        }
        if (thread && ret->heaploc != -1) {
            if (pthread_rwlock_wrlock(&((JVM *)thread->jvm)->rwlock))
                err(EXIT_FAILURE, "newObject: wrlock");
            ((JVM *)thread->jvm)->heap[ret->heaploc] = NULL;
            if (pthread_rwlock_unlock(&((JVM *)thread->jvm)->rwlock))
                err(EXIT_FAILURE, "newObject: unlock");
        }
        free(ret);
        ret = NULL;
    }
    return NULL;
}

#ifdef DEBUG
void dumpClass(const ClassFile *cf)
{
    char buf[BUFSIZ];

    printf( "Class: %s\n", 
            cf->this_class->d.class.name->d.utf8.ascii
            //cf->canonical_name,
            //cf->original_name
            );
    printAccessFlags(cf->access_flags, buf, BUFSIZ);
    printf( "       version:%d.%d access_flags:%s super:%s this_class_idx:%d\n",
            cf->major_version,
            cf->minor_version,
            buf,
            cf->super_class ? cf->super_class->d.class.name->d.utf8.ascii : "(none)",
            cf->this_class_idx
          );
    printf( "Constant Pool [%2d]:\n", cf->constant_pool_count);
    for (int i = 1; i < cf->constant_pool_count; i++)
    {
        dumpConstantPool(i, cf->constant_pool[i]);
        if (cf->constant_pool[i]->tag == CONSTANT_Double ||
                cf->constant_pool[i]->tag == CONSTANT_Long)
            i++;
    }
    printf( "Interfaces:\n");
    for (int i = 0; i < cf->interfaces_count; i++)
    {
        printf(" [%2d]\n", i);
    }
    printf( "Fields: [%2d]\n", cf->fields_count);
    for (int i = 0; i < cf->fields_count; i++)
    {
        dumpField(i, cf->fields[i]);
    }
    printf( "Methods: [%2d]\n", cf->methods_count);
    for (int i = 0; i < cf->methods_count; i++)
    {
        dumpMethod(i, cf->methods[i]);
    }
    printf( "Attributes: [%2d]\n", cf->attributes_count);
    for (int i = 0; i < cf->attributes_count; i++)
    {
        printf(" [%2d]: ", i);
        dumpAttribute(cf->attributes[i]);
    }
    printf("\n");
}
#endif

static const char *search[] = {
    "cp",
    "cp/java",
    "cp/java/lang",
    "cp/java/lang/annotation",
    "cp/java/lang/reflect",
    "cp/java/math"
    "cp/java/io",
    "cp/java/nio",
    "cp/java/nio/channels",
    "cp/java/text",
    "cp/java/util",
    "cp/fail",
    "test",
    NULL
};

static ClassFile *loadClass(Thread *const thread, const char *restrict sfile)
{
    ClassFile *ret;
    FILE *fp;
    char buf[BUFSIZ];
    char file[BUFSIZ];

    const char *start;

    assert(sfile != NULL);
    assert(*sfile != 0);

#ifdef DEBUG
    printf("(loadClass: %s)\n", sfile);
#endif

    if ((fp = fopen(sfile, "r")) != NULL) {
#ifdef DEBUG
        printf("(loadClass trying.0: %s direct found)\n", sfile);
#endif
        goto found;
    }

    if ((start = strstr(sfile, ".class")) == NULL)
        start = (char *)(sfile + strlen(sfile));

    assert(start != NULL);

    if ( (start - sfile) >= BUFSIZ  || (start - sfile) <= 1 ) {
        warnx("loadClass: file name malformed, too short or too long");
        return NULL;
    }

    file[start - sfile] = '\0';
    start--;

    while(start >= sfile)
    {
        file[start - sfile] = (*start == '.' ? '/' : *start);
        start--;
    }

#ifdef DEBUG
    printf("(loadClass normalised to: %s)\n", file);
#endif

    if (strchr(file, '/')) {
        if(snprintf(buf, sizeof(buf), "cp/%s%s", file,
                    strstr(file, ".class") != NULL ? "" : ".class") > (int)sizeof(buf)) {
            STRING_TOO_LONG();
        }
#ifdef DEBUG
    printf(" (loadClass trying.1: %s)\n", buf);
#endif
        fp = fopen(buf, "r");

    } else {

        /* this is a bit of a mess as .2 .3 .4 are (rarely? never?) used and 
         * should be instead made to use the classpath to search */
        snprintf(buf, sizeof(buf), "%s", file);

#ifdef DEBUG
    printf(" (loadClass trying.2: %s)\n", buf);
#endif
        if ((fp = fopen(buf, "r")) == NULL) {
            for (int i = 0; search[i]; i++)
            {
                if(snprintf(buf, sizeof(buf), "%s/%s",
                            search[i], file) > (int)sizeof(buf)) {
                    STRING_TOO_LONG();
                }
#ifdef DEBUG
    printf(" (loadClass trying.3: %s)\n", buf);
#endif
                if ((fp = fopen(buf, "r")) != NULL) break;

                if(snprintf(buf, sizeof(buf), "%s/%s.class", search[i],
                            file) > (int)sizeof(buf)) {
                    STRING_TOO_LONG();
                }
#ifdef DEBUG
    printf(" (loadClass trying.4: %s)\n", buf);
#endif
                if ((fp = fopen(buf, "r")) != NULL) break;

            }
        }
    }

    if (fp == NULL) {
        warnx("loadClass: no such class: %s", buf);
        return NULL;
    }

found:
    ret = parseClass(thread, fp);
    if (!ret)
        warnx("loadClass: parseClass failed for %s", buf);
    fclose(fp);

#ifdef DEBUG
    //if (ret)
    //  dumpClass(ret);
#endif

    return ret;
}

static void freeAttribute(attribute_info *const ai)
{
    const struct predefinedAttribute *pa;
    bool found = false;

    for (int i = 0; ((pa = &predefinedAttrs[i])->name) != NULL; i++)
    {
        if (!ai->name || !ai->name->d.utf8.ascii ||
                strcmp(ai->name->d.utf8.ascii, pa->name)) continue;

        switch (pa->attr)
        {
            case ATTR_LINENUMBERTABLE:
                found = true;
                if (ai->d.lineno->line_number_table)
                    free(ai->d.lineno->line_number_table);
                free((void *)ai->d.lineno);
                ai->d.lineno = NULL;
                break;
            case ATTR_SOURCEFILE:
                found = true;
                free((void *)ai->d.srcfile);
                ai->d.srcfile = NULL;
                break;
            case ATTR_CODE:
                found = true;
                if(ai->d.code->code) {
                    free((uint8_t *)ai->d.code->code);
                    ai->d.code->code = NULL;
                }
                if(ai->d.code->attributes) {
                    for (int j = 0; j < ai->d.code->attributes_count; j++)
                    {
                        if (ai->d.code->attributes[j]) {
                            freeAttribute((attribute_info *)ai->d.code->attributes[j]);
                            ai->d.code->attributes[j] = NULL;
                        }
                    }
                    free(ai->d.code->attributes);
                    ai->d.code->attributes = NULL;
                }
                if(ai->d.code->exception_table) {
                    for (int j = 0; j < ai->d.code->exception_table_length; j++)
                    {
                        if (ai->d.code->exception_table[j]) {
                            free((void *)ai->d.code->exception_table[j]);
                            ai->d.code->exception_table[j] = NULL;
                        }
                    }
                    free(ai->d.code->exception_table);
                    ai->d.code->exception_table = NULL;
                }
                free((void *)ai->d.code);
                ai->d.code = NULL;
                break;
            case ATTR_CONSTANT_VALUE:
                found = true;
                if (ai->d.constant->entry) switch(ai->d.constant->entry->tag)
                {
                    case CONSTANT_String:
                        if (ai->d.constant->op.val.vref)
                            ai->d.constant->op.val.vref->lock--;
                        break;
                    default:
                        /* do nothing */
                        break;
                }
                free((void *)ai->d.constant);
                ai->d.constant = NULL;
                break;
            case ATTR_STACKMAPTABLE:
                found = true;
                if (ai->d.stackmap->entries)
                {
                    for (int j = 0; j < ai->d.stackmap->number_of_entries; j++)
                    {
                        switch(ai->d.stackmap->entries[j]->type)
                        {
                            case FT_APPEND:
                                {
                                    if (ai->d.stackmap->entries[j]->i.af.locals)
                                    {
                                        for (int k = 0; k < ai->d.stackmap->entries[j]->i.af.local_num; k++)
                                        {
                                            if (ai->d.stackmap->entries[j]->i.af.locals[k]) {
                                                free(ai->d.stackmap->entries[j]->i.af.locals[k]);
                                                ai->d.stackmap->entries[j]->i.af.locals[k] = NULL;
                                            }
                                        }
                                        free(ai->d.stackmap->entries[j]->i.af.locals);
                                        ai->d.stackmap->entries[j]->i.af.locals = NULL;
                                    }
                                }
                                break;
                            case FT_FULL_FRAME:
                                {
                                    if (ai->d.stackmap->entries[j]->i.ff.locals)
                                    {
                                        for (int k = 0; k < ai->d.stackmap->entries[j]->i.ff.number_of_locals; k++)
                                        {
                                            if (ai->d.stackmap->entries[j]->i.ff.locals[k]) {
                                                free(ai->d.stackmap->entries[j]->i.ff.locals[k]);
                                                ai->d.stackmap->entries[j]->i.ff.locals[k] = NULL;
                                            }
                                        }
                                        free(ai->d.stackmap->entries[j]->i.ff.locals);
                                        ai->d.stackmap->entries[j]->i.ff.locals = NULL;
                                    }
                                    if (ai->d.stackmap->entries[j]->i.ff.stack)
                                    {
                                        for (int k = 0; k < ai->d.stackmap->entries[j]->i.ff.number_of_stack_items; k++)
                                        {
                                            if (ai->d.stackmap->entries[j]->i.ff.stack[k]) {
                                                free(ai->d.stackmap->entries[j]->i.ff.stack[k]);
                                                ai->d.stackmap->entries[j]->i.ff.stack[k] = NULL;
                                            }
                                        }
                                        free(ai->d.stackmap->entries[j]->i.ff.stack);
                                        ai->d.stackmap->entries[j]->i.ff.stack = NULL;
                                    }
                                }
                                break;
                            case FT_SAME:
                            case FT_SAME_LOCALS_1_STACK_ITEM:
                            case FT_CHOP:
                            case FT_SAME_FRAME_EXTENDED:
                            default:
                                break;
                            case FT_SAME_LOCALS_1_STACK_ITEM_EXTENDED:
                                warnx("freeAttribute: StackMapTable: freeframe type not suppored");
                                break;
                            case FT_NULL:
                                warnx("freeAttribute: StackMapTable: FT_NULL");
                                break;
                        }
                        free(ai->d.stackmap->entries[j]);
                        ai->d.stackmap->entries[j] = NULL;
                    }
                    free(ai->d.stackmap->entries);
                    ai->d.stackmap->entries = NULL;
                }
                free((void *)ai->d.stackmap);
                ai->d.stackmap = NULL;
                break;
            case ATTR_INNERCLASSES:
                found = true;
                if (ai->d.inner->classes) {
                    for (int j = 0; j < ai->d.inner->number_of_classes; j++)
                    {
                        if(ai->d.inner->classes[j])
                            free(ai->d.inner->classes[j]);
                        ai->d.inner->classes[j] = NULL;
                    }
                    free(ai->d.inner->classes);
                    ai->d.inner->classes = NULL;
                }
                free((void *)ai->d.inner);
                ai->d.inner = NULL;
                break;
            case ATTR_EXCEPTIONS:
                found = true;
                if (ai->d.exception->exception_index_table)
                {
                    free(ai->d.exception->exception_index_table);
                    ai->d.exception->exception_index_table = NULL;
                }
                free((void *)ai->d.exception);
                ai->d.exception = NULL;
                break;
            case ATTR_BOOTSTRAPMETHODS:
            case ATTR_INVALID:
                found = true;
                break;
            default:
                break;
        }

        if(found) break;
    }

    free(ai);
}

#ifdef DEBUG_CODE
const char *printObject(const Object *const o)
{
    static char buf[BUFSIZ] = {0};

    snprintf(buf, sizeof(buf), "OBJ_%s lock=%d heap_id=%d ",
            o->type == OBJ_CLASS ? "CLASS" : "ARRAY",
            o->lock,
            o->heaploc);

    //return buf;

    FILE *const bfp = fmemopen(buf, sizeof(buf), "w");

    fprintf(bfp, "OBJ_%s lock=%d heap_id=%d ",
            o->type == OBJ_CLASS ? "CLASS" : "ARRAY",
            o->lock,
            o->heaploc);

    switch(o->type)
    {
        case OBJ_ARRAY:
            fprintf(bfp, "%s[%d]",
                    printOpType(o->data.array.type),
                    o->data.array.len);
            break;
        case OBJ_CLASS:
            break;
        default:
            errx(EXIT_FAILURE, "printObject: unknown type: %d", o->type);
    }

    fclose(bfp);
    return buf;
}
#endif

/* caller must have a wrlock on jvm */
static bool freeObject(JVM *const jvm, Object *restrict const o)
{
    if (o->type == OBJ_CLASS && o->class->isInnerClass) {
        if (o->lock > 1)
            return false;
    } else if (o->lock)
        return false;

    if ((pthread_mutex_trylock(&o->monitor) != 0))
        return false;

    switch (o->type)
    {
        case OBJ_CLASS:
            {
                if (o->data.obj.fields) {
                    for (int i = 0; i < o->data.obj.num_fields; i++)
                    {
                        Operand *tmpO;
                        if ( (tmpO = o->data.obj.fields[i].op) != NULL ) {
#if 0
                            printf(" (freeField %d %s ", i, printOpType(tmpO->type));
                            if (tmpO->type == TYPE_OREF)
                                printf("%s", printOpValue(tmpO));
                            printf(")\n");
#endif
                            freeAndNullOperand(&tmpO);
                        }
                        o->data.obj.fields[i].op = NULL;
                    }
                    free(o->data.obj.fields);
                    o->data.obj.fields = NULL;
                }

				if (o->data.obj.native_data) {
					assert(o->data.obj.native_free != NULL);
					o->data.obj.native_free(o->data.obj.native_data);
					o->data.obj.native_data = NULL;
				}
            }
            break;
        case OBJ_ARRAY:
            {
                int count = 0;
                for(uint32_t i = 0; i < o->data.array.len; i++) {
                    if(!o->data.array.data[i])
                        continue;
                    count++;
#if 0
                    int lock =
#endif
                        freeAndNullOperand(&o->data.array.data[i]);
#if 0
                    if (lock != -1)
                        printf(" (freeObject [OBJ_ARRAY] [%d/%d] lock.now=%d\n", i, o->data.array.len, lock);
#endif
                    o->data.array.data[i] = NULL;
                }
                //printf("Freeing Array [%d]\n", count);
                free(o->data.array.data);
                o->data.array.data = NULL;
                free(o->data.array.sizes);
                o->data.array.sizes = NULL;
            }
            break;
        default:
            warnx("freeObject: unknown type %d", o->type);
    }

    if (o->heaploc != -1)
        jvm->heap[o->heaploc] = NULL;

    pthread_mutex_unlock(&o->monitor);
    pthread_mutex_destroy(&o->monitor);

    if (o->className) {
        free(o->className);
    }
    free(o);
    return true;
}

/* caller must have wrlock on jvm */
static void freeClass(JVM *const jvm, ClassFile *restrict const cf)
{
    char buf[BUFSIZ] = {0};
    int rc;

    if ((rc = pthread_rwlock_wrlock(&cf->rwlock))) {
        strerror_r(rc, buf, sizeof(buf));
        warnx("freeClass: pthread_rwlock_wrlock: %s", buf);
        return;
    }

    if (cf->class_object) {
        // lock++ in createClass
        cf->class_object->lock--;
        freeObject(jvm, cf->class_object);
        cf->class_object = NULL;
    }

    if (cf->fields) {
        for (int i = 0; i < cf->fields_count; i++)
        {
            field_info *fi = (field_info *)cf->fields[i];
            if (!fi) continue;

            if (fi->attributes) {
                for (int j = 0; j < fi->attributes_count; j++)
                {
                    attribute_info *ai = (attribute_info *)fi->attributes[j];
                    if (!ai) continue;

                    freeAttribute(ai);
                    fi->attributes[j] = NULL;
                }
                free(fi->attributes);
                fi->attributes = NULL;
            }

            freeAndNullOperand(&fi->static_operand);

            if(fi->desc.className) {
                free((void *)fi->desc.className);
                fi->desc.className = NULL;
            }
            if (fi->desc.orig_className) {
                free((void *)fi->desc.orig_className);
                fi->desc.orig_className = NULL;
            }

            free(fi);
            cf->fields[i] = NULL;
        }
        free(cf->fields);
        cf->fields = NULL;
    }

    if (cf->methods) {
        for (int i = 0; i < cf->methods_count; i++)
        {
            method_info *mi = (method_info *)cf->methods[i];
            if (!mi) continue;

            if (mi->attributes) {
                for (int j = 0; j < mi->attributes_count; j++)
                {
                    attribute_info *ai = (attribute_info *)mi->attributes[j];
                    if (!ai) continue;

                    freeAttribute(ai);
                    mi->attributes[j] = NULL;
                }
                free(mi->attributes);
                mi->attributes = NULL;
            }

            if (mi->desc.params) {
                for (int j = 0; j < mi->desc.num_params; j++)
                {
                    if(mi->desc.params[j]) {
                        if (mi->desc.params[j]->className) {
                            free((void *)mi->desc.params[j]->className);
                            mi->desc.params[j]->className = NULL;
                        }
                        if (mi->desc.params[j]->orig_className) {
                            free((void *)mi->desc.params[j]->orig_className);
                            mi->desc.params[j]->orig_className = NULL;
                        }
                        free((void *)mi->desc.params[j]);
                    }
                }
                free(mi->desc.params);
                mi->desc.params = NULL;
            }

            if (mi->desc.ret.className)
                free((void *)mi->desc.ret.className);

            if (mi->desc.ret.orig_className)
                free((void *)mi->desc.ret.orig_className);

//          if (mi->original_name)
//              free((void *)mi->original_name);

//          if (mi->original_descriptor)
//              free((void *)mi->original_descriptor);

            free(mi);
            cf->methods[i] = NULL;
        }
        free(cf->methods);
        cf->methods = NULL;
    }

    if (cf->attributes) {
        for (int i = 0; i < cf->attributes_count; i++)
        {
            attribute_info *ai = (attribute_info *)cf->attributes[i];
            if (!ai) continue;

            freeAttribute(ai);
            cf->attributes[i] = NULL;
        }
        free(cf->attributes);
        cf->attributes = NULL;
    }

    if (cf->constant_pool) {
        for (int i = 0; i < cf->constant_pool_count; i++)
        {
            cp_info *cp = (cp_info *)cf->constant_pool[i];
            if (!cp) continue;
            cf->constant_pool[i] = NULL;

            switch(cp->tag)
            {
                case CONSTANT_NameAndType:
                    {
                        //cp_NameAndType_info *nt = &cp->d.nt;
//                      if (nt->original_name)
//                          free((void *)nt->original_name);
//                      if (nt->original_descriptor)
//                          free((void *)nt->original_descriptor);
                    }
                    break;
                case CONSTANT_Class:
                    {
                        //cp_Class_info *ci = &cp->d.class;
//                      if (ci->original_name)
//                          free((char *)ci->original_name);
//                      ci->original_name = NULL;
                    }
                    break;
                case CONSTANT_String:
                    {
                        cp_String_info *str = &cp->d.str;
                        if (str->intern) {
                            // lock++ in fixupCpString
                            ((Object *)str->intern)->lock--;
                            freeObject(jvm, (Object *)str->intern);
                            str->intern = NULL;
                        }
                    }
                    break;
                case CONSTANT_Long:
                case CONSTANT_Double:
                    i++;
                    break;
                case CONSTANT_Utf8:
                    {
                        cp_Utf8_info *utf8 = &cp->d.utf8;
                        if (utf8->utf16) free(utf8->utf16);
                        utf8->utf16 = NULL;
                        if (utf8->ascii) free(utf8->ascii);
                        utf8->ascii = NULL;
                        if (utf8->bytes) free(utf8->bytes);
                        utf8->bytes = NULL;
                    }
                    break;
                default:
                    break;
            }
            free(cp);
        }
        free(cf->constant_pool);
        cf->constant_pool = NULL;
    }

    if (cf->super_name) {
        free((char *)cf->super_name);
        cf->super_name = NULL;
    }

//  if (cf->canonical_name) {
//      free((char *)cf->canonical_name);
//      cf->canonical_name = NULL;
//  }

//  if (cf->original_name) {
//      free((char *)cf->original_name);
//      cf->original_name = NULL;
//  }

    if (cf->interfaces) {
        free(cf->interfaces);
        cf->interfaces = NULL;
    }

    if (cf->interface_class) {
        free(cf->interface_class);
        cf->interface_class = NULL;
    }

    if (cf->heaploc != -1)
        jvm->method_area[cf->heaploc] = NULL;

    pthread_rwlock_unlock(&cf->rwlock);
    pthread_rwlock_destroy(&cf->rwlock);

    free(cf);
}

static void freeThread(Thread *restrict const t)
{
#ifdef DEBUG
//  printf("(freeThread %p)\n", (void *)t);
#endif
    if (t->thread) {
        t->thread->lock--; // lock++ in newThread
        assert(t->thread->lock == 0);
    }

    /* checking for missing frames (probably a bug) or the initial frame (not a bug) */
    for (uint16_t i = 0; i < t->frame_free; i++)
        if (t->stack[i]) {
            freeFrame(t->stack[i]);
            t->stack[i] = NULL;
        }

    free(t);
}

#if 0
static void printThreadState(th_state_en st, char *buf, int size)
{
    const char *tmp = NULL;
    switch(st)
    {
        case TH_NEW:        tmp = "TH_NEW";     break;
        case TH_RUNNING:    tmp = "TH_RUNNING"; break;
        case TH_DEAD:       tmp = "TH_DEAD";    break;
        case TH_FINISHED:   tmp = "TH_FINISHED";break;
        default: tmp = "<<unknown>>"; break;
    }
    snprintf(buf, size, "%s", tmp);
}
#endif

static JVM *jvm = NULL;

static void freeJVM(JVM *restrict const j)
{
    int rc;
    char buf[BUFSIZ] = {0};

    int count;

    const struct timespec rqtp = {
        .tv_sec = 0,
        .tv_nsec = 50000000L,
    };

    do {
        nanosleep(&rqtp, NULL);
        count = 0;

        if ((rc = pthread_rwlock_wrlock(&j->rwlock))) {
            strerror_r(rc, buf, sizeof(buf));
            if (rc != EDEADLK)
                errx(EXIT_FAILURE, "freeJVM: wrlock: %s", buf);
            else
                warnx("freeJVM: wrlock: %s", buf);
        }

        Operand *ex_op = NULL;

        for (uint16_t id = 0; id < MAX_THREADS; id++) {
            if (j->threads[id]) {
                if (j->threads[id]->state == TH_FINISHED) {
                    pthread_join(j->threads[id]->pthread, (void **)&ex_op);
                    j->threads[id]->state = TH_DEAD;
                    freeThread(j->threads[id]);
                    j->threads[id] = NULL;
                    if (ex_op) {
                        const Object *ex_obj = ex_op->val.vref;
                        if (ex_op->type == TYPE_OREF && ex_obj && ex_obj->class) {
                            warnx("Uncaught exception %s",
                                    ex_obj->class->this_class->d.class.name->d.utf8.ascii);
                        } else {
                            warnx("Uncaught exception (null)");
                        }
                        freeAndNullOperand(&ex_op);
                    }
                } else if (j->threads[id]->state == TH_NEW) {
                    if (j->threads[id]->pthread)
                        pthread_join(j->threads[id]->pthread, (void **)&ex_op);
                    j->threads[id]->state = TH_DEAD;
                    freeThread(j->threads[id]);
                    j->threads[id] = NULL;
                } else if (j->threads[id]->state == TH_RUNNING)
                    count++;
            }
        }

        if(pthread_rwlock_unlock(&j->rwlock))
            err(EXIT_FAILURE, "freeJVM: pthread_rwlock_unlock");
    } while(count > 0);

    pthread_rwlock_destroy(&clslock);

    for (ssize_t i = 0; i < num_ary_maps; i++) {
        if (ary_maps[i].object->lock > 0)
            ((Object *)ary_maps[i].object)->lock--; // lock++ createClassForArray
        free((char *)ary_maps[i].canonical);
        ary_maps[i].canonical = NULL;
        ary_maps[i].object = NULL;
    }

    free(ary_maps);
    ary_maps = NULL;

    for (uint16_t i = 0; i < MAX_METHOD; i++)
    {
        if (j->method_area[i]) {
            freeClass(j, (ClassFile *)j->method_area[i]);
            j->method_area[i] = NULL;
        }
    }

    int found;
    for (int k = 0; k < 30; k++)
    {
        found = 0;
        for (uint16_t i = 0; i < j->maxHeap; i++)
        {
            if (j->heap[i]) {
                found++;
                if (freeObject(j, j->heap[i]))
                    j->heap[i] = NULL;
            }
        }
    }

    if (found) {
        warnx("%d objects still remain: totalOperands=%ld", found, totalOperands);
        for (uint16_t i = 0; i < j->maxHeap; i++)
        {
            if (!j->heap[i]) continue;
            Object *o = j->heap[i];
            warnx("object: heap[%d=%d] OBJ_%s lock=%d class=%s",
                    o->heaploc,
                    i,
                    o->type == OBJ_CLASS ? "CLASS" : "ARRAY",
                    o->lock,
                    o->className);
        }
    }


    if (jvm == j) jvm = NULL;
    pthread_rwlock_unlock(&j->rwlock);
    pthread_rwlock_destroy(&j->rwlock);
    free(j->heap);
    free(j);
}


static void cleanup(void)
{
    if (jvm) {
        freeJVM(jvm);
        jvm = NULL;
    }
    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
}

static ClassFile *processNatives(Thread *const thread, const internalClass *const ic)
{
    ClassFile *cf = NULL;

    if (!ic || !ic->name) {
        warnx("processNatives: missing name");
        goto fail;
    }

    if ((cf = (ClassFile *)findClass(thread, ic->name)) == NULL) {
        warnx("processNatives: unable to load class: %s", ic->name);
        goto fail;
    }

    for (uint16_t i = 0; i < MAX_METHODS; i++)
    {
        if (!ic->methods[i].name) continue;

        if (ic->methods[i].name == NULL || ic->methods[i].desc == NULL) {
            warnx("processNatives: missing name/desc in %s[%d]", ic->name, i);
            continue;
        }

        const method_info *const mi = findMethodByClass(thread, cf,
                ic->methods[i].name, ic->methods[i].desc);
        if (mi == NULL) {
            warnx("processNatives: no such method %s:%s", ic->methods[i].name,
                    ic->methods[i].desc);
            continue;
        }

        if (!(mi->access_flags & ACC_NATIVE)) {
            warnx("processNatives: method %s:%s is not ACC_NATIVE",
                    ic->methods[i].name, ic->methods[i].desc);
            continue;
        }

        if (mi->access_flags & ACC_STATIC)
            ((method_info *)mi)->nativeMethod.mstatic =
                ic->methods[i].meth.mstatic;
        else
            ((method_info *)mi)->nativeMethod.mvirtual =
                ic->methods[i].meth.mvirtual;
#ifdef DEBUG
        printf("processNatives: patched in %s:%s\n", ic->methods[i].name,
                ic->methods[i].desc);
#endif
    }

    return cf;
fail:
    warnx("processNatives: unable to process: %s", ic ? ic->name : "null");
    return NULL;
}


static Thread *newThread(JVM *const jvm, Object *const thread)
{
    //printf("newThread: start\n");
    Thread *ret = NULL;
    int rc;
    char buf[BUFSIZ] = {0};

    if ((ret = calloc(1, sizeof(Thread))) == NULL) goto fail;
    ret->jvm = jvm;

    if (thread) {
        ret->thread = thread;
        ret->thread->lock++; // lock-- in freeThread
    } else if (jvm->VM) {
        if ((ret->thread = newObject(ret, findClass(ret,
                            "java.lang.Thread"))) == NULL) goto fail;
        ret->thread->lock++; // lock-- in freeThread
    } else // jvm->VM == false
        ret->thread = NULL;


    if ((rc = pthread_rwlock_init(&ret->rwlock, NULL))) {
        strerror_r(rc, buf, sizeof(buf));
        errx(EXIT_FAILURE, "newThread: pthread_rwlock_init: %s", buf);
    }

    if ((rc = pthread_rwlock_wrlock(&ret->rwlock))) {
        strerror_r(rc, buf, sizeof(buf));
        errx(EXIT_FAILURE, "newThread: pthread_rwlock_wrlock(thread): %s", buf);
    }

    if ((rc = pthread_rwlock_wrlock(&jvm->rwlock))) {
        strerror_r(rc, buf, sizeof(buf));
        errx(EXIT_FAILURE, "newThread: pthread_rwlock_wrlock(jvm): %s", buf);
    }

    uint16_t th_id;

    for (th_id = 0; th_id < MAX_THREADS; th_id++) {
        if (jvm->threads[th_id] == NULL) break;
    }

    if (th_id == MAX_THREADS) {
        //pthread_rwlock_unlock(&ret->rwlock);
        //pthread_rwlock_unlock(&jvm->rwlock);
        warnx("newThread: no free threads");
        goto fail;
    }

    jvm->threads[th_id] = ret;
    ret->state = TH_NEW;
    ret->thread_id = th_id;
    //printf("newThread: created with thread_id=%d\n", ret->thread_id);

    pthread_rwlock_unlock(&jvm->rwlock);
    pthread_rwlock_unlock(&ret->rwlock);
    return ret;
fail:
    if (ret) {
        pthread_rwlock_unlock(&jvm->rwlock);
        pthread_rwlock_unlock(&ret->rwlock);
        pthread_rwlock_destroy(&ret->rwlock);
        if (ret->thread && ret->thread->lock)
            ret->thread->lock--; // lock++ here
        free(ret);
    }
    return NULL;
}

/* self->frame[0] is what was passed to startThread (stack based)
 * self->Frame[1] is the args to be used here (local based)
 */
static void *threadinit(void *arg)
{
    Thread *const self = (Thread *)arg;

    //printf("inside threadinit\n");

    int rc;
    char buf[BUFSIZ] = {0};
    if ((rc = pthread_rwlock_wrlock(&self->rwlock))) {
        strerror_r(rc, buf, sizeof(buf));
        warnx("threadinit: pthread_rwlock_wrlock: %s", buf);
        return NULL;
    }
    Frame *const cur_frame = currentFrame(self);
    if (cur_frame == NULL)
        errx(EXIT_FAILURE, "threadInit: no current frame");

    const bool isStatic = self->cur_method->access_flags & ACC_STATIC;

    if (!isStatic) {
        Operand *this = getLocal(cur_frame, 0);
        push(cur_frame, dupOperand(this));
        clearAndFreeLocal(cur_frame, 0);
    }

    // FIXME this doesn't work with double's
    /* check is this the right order of arguments for the stack? */
    uint16_t local = isStatic ? 0 : 1;

    for (int i = 0; i < self->cur_method->desc.num_params; i++)
    {
        Operand *op = NULL;
        op = getLocal(cur_frame, local);
        if (op == NULL) {
            warnx("threadinit: %d local is null", local);
            __asm__("int $3");
            continue;
        } else if (!push(cur_frame, op = dupOperand(op))) {
            warnx("threadinit: failed to push local[%d]", local);
            freeAndNullOperand(&op);
        }
        const uint16_t oldlocal = local;
        if (op && (op->type == TYPE_LONG || op->type == TYPE_DOUBLE))
            local++;
        local++;
        clearAndFreeLocal(cur_frame,oldlocal);
    }

    self->state = TH_RUNNING;
    pthread_rwlock_unlock(&self->rwlock);

    Operand *ret = NULL;
    if (isStatic) {
        //printf("inside threadinit: about to invoke static\n");
        ret = invokeMethod(self, self->cur_method, false, 0);
    } else {
        //printf("inside threadinit: about to invoke non-static\n");
        ret = invokeMethod(self, self->cur_method, true, 0);
    }

    //printf("inside threadinit: returned\n");
    if (ret) {
        while (cur_frame->sp)
            freeOperand(pop(cur_frame));

        Operand *const dup = dupOperand(ret);
        if (!push(cur_frame, dup)) {
            freeAndNullOperand((Operand **)&dup);
            warnx("threadinit: failed to push thrown object: %d/%d", cur_frame->sp, cur_frame->num_stack);
        } else {
            method_info *mi = findMethodByClass(self,
                    ((Operand *)ret)->val.vref->class,
                    "printStackTrace", "()V");
            freeAndNullOperand(&ret);
            if (mi)
                ret = invokeMethod(self, mi, true, 0);
        }
    }

    if ((rc = pthread_rwlock_wrlock(&self->rwlock))) {
        strerror_r(rc, buf, sizeof(buf));
        warnx("threadinit: pthread_rwlock_wrlock: %s", buf);
        return ret;
    }

    self->state = TH_FINISHED;
    pthread_rwlock_unlock(&self->rwlock);
    return ret;
}

static Operand *startThread(Thread *const thread)
{
    Frame *const cur_frame = currentFrame(thread);
    int rc;
    char buf[BUFSIZ] = {0};

    if ((rc = pthread_rwlock_wrlock(&thread->rwlock))) {
        strerror_r(rc, buf, sizeof(buf));
        errx(EXIT_FAILURE, "startThread: wrlock: %s", buf);
    }
    const Object *th_obj = thread->thread;

    if (thread->cur_method == NULL) {
        method_info *meth = findMethodByClass(thread,
                th_obj->class, "run", "()V");
        thread->cur_method = meth;
    }

    Frame *first = newFrame(
            (uint16_t)(thread->cur_method->code->max_stack + 3U),
            (uint16_t)(umax(thread->cur_method->desc.num_params, thread->cur_method->code->max_locals) + 2U)
            );
    if (first == NULL || !first->local) {
        errx(EXIT_FAILURE, "startThread: unable to create frame");
    }
    thread->stack[thread->frame_free++] = first;

    first->local[0] = newOperand(TYPE_OREF, th_obj);

    if (thread->cur_method->desc.num_params > first->num_local)
        errx(EXIT_FAILURE, "more parameters than local slots");

    if (cur_frame) for (uint16_t i = 0; i < thread->cur_method->desc.num_params; i++)
    {
        // FIXME: (check it) doesn't work for long/double
        first->local[i + 1] = pop(cur_frame);
    }

    if ((rc = pthread_create(&thread->pthread, NULL, threadinit, thread))) {
        strerror_r(rc, buf, sizeof(buf));
        errx(EXIT_FAILURE, "startThread: pthread_create: %s", buf);
    }

    if (pthread_rwlock_unlock(&thread->rwlock))
        warnx("startThread: pthread_rwlock_unlock");

    const struct timespec rqtp = {
        .tv_sec = 0,
        .tv_nsec = 100000000L,
    };

    nanosleep(&rqtp, NULL);
    return NULL;
}

int main(const int ac, const char *const av[])
{
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    if (ac != 3) {
        fprintf(stderr, "Missing arguments\nUsage: fail-jvm CLASSFILE MAINCLASSNAME\n");
        exit(EXIT_FAILURE);
    }

    if ((jvm = calloc(1, sizeof(JVM))) == NULL)
        err(EXIT_FAILURE, "boot: calloc(jvm)");

    if ((jvm->heap = calloc(DEF_HEAP, sizeof(Object *))) == NULL)
        err(EXIT_FAILURE, "boot: calloc(jvm->heap)");

    jvm->maxHeap = DEF_HEAP;
    jvm->VM = false;

    int rc;

    if ((rc = pthread_rwlock_init(&jvm->rwlock, NULL)))
        errx(EXIT_FAILURE, "boot: JVM.rwlock: %s", strerror(rc));

    if ((rc = pthread_rwlock_init(&clslock, NULL)))
        errx(EXIT_FAILURE, "boot: clslock: %s", strerror(rc));

    atexit(cleanup);

    ClassFile *cf = NULL;
    Thread *parent = NULL;

    if ((parent = newThread(jvm, NULL)) == NULL)
        errx(EXIT_FAILURE, "boot: newThread");

    if (parent->thread)
        errx(EXIT_FAILURE, "boot: thread not null");

    Frame *cur_frame = parent->stack[parent->frame_free++] = newFrame(3, 2);

    ClassFile *cls_String, *cls_Class, *cls_Object, *cls_ClassLoader;

    /* load classes with circular references */
    if ((cls_Object = loadClass(parent, "java.lang.Object")) == NULL) errx(1, "unable to load java.lang.Object");
    if ((cls_String = loadClass(parent, "java.lang.String")) == NULL) errx(1, "unable to load java.lang.String");
    if ((cls_Class = loadClass(parent, "java.lang.Class")) == NULL) errx(1, "unable to load java.lang.Class");
    if ((cf = loadClass(parent, "java.lang.Thread")) == NULL) errx(1, "unable to load java.lang.Thread");
    if ((cls_ClassLoader = loadClass(parent, "java.lang.ClassLoader")) == NULL) errx(1, "unable to load java.lang.ClassLoader");

    jvm->VM = true;

#ifdef DEBUG
    printf("boot: VM now enabled\n");
#endif

    cls_Class->isLinked = true;
    cls_String->isLinked = true;
    cls_ClassLoader->isLinked = true;

    linkClass(parent, cls_Object);

    cls_Class->isLinked = false;
    linkClass(parent, cls_Class);

    cls_String->isLinked = false;
    linkClass(parent, cls_String);

    cls_ClassLoader->isLinked = false;
    linkClass(parent, cls_ClassLoader);

#ifdef DEBUG
    printf("boot: linking early classes\n");
#endif
    /* ensure getClass() works on them */
    for (uint16_t i = 0; i < MAX_METHOD; i++)
    {
        ClassFile *const ret = (ClassFile *)jvm->method_area[i];
        if (!ret)
            continue;
        if (!ret->isLinked)
            linkClass(parent, ret);
    }

#ifdef DEBUG
    printf("boot: patching natives\n");
#endif
    /* patch natives */
    cf = processNatives(parent, &java_lang_Object);
    if (cf == NULL) exit(1);
    cf = processNatives(parent, &java_io_FileOutputStream);
    if (cf == NULL) exit(1);
    cf = processNatives(parent, &java_io_FileInputStream);
    if (cf == NULL) exit(1);
    cf = processNatives(parent, &java_io_File);
    if (cf == NULL) exit(1);
    cf = processNatives(parent, &java_lang_Thread);
    if (cf == NULL) exit(1);
    cf = processNatives(parent, &java_lang_Throwable);
    if (cf == NULL) exit(1);
    cf = processNatives(parent, &java_lang_System);
    if (cf == NULL) exit(1);
    cf = processNatives(parent, &java_lang_reflect_Array);
    if (cf == NULL) exit(1);
    cf = processNatives(parent, &java_lang_ClassLoader);
    if (cf == NULL) exit(1);
    cf = processNatives(parent, &java_io_FileDescriptor);
    if (cf == NULL) exit(1);

    /* load primitive classes */
    for (ssize_t i = 0; primitiveClassMap[i].c_name; i++)
    {
        ClassFile *cls = findClass(parent, primitiveClassMap[i].c_name);
        if (cls == NULL) {
            warnx("boot: unable to load %s", primitiveClassMap[i].c_name);
            continue;
        }
        if (!cls->isInit) {
            if (classInit(parent, (ClassFile *)cls))
                errx(EXIT_FAILURE, "boot: error thrown in %s", primitiveClassMap[i].c_name);
        }

        field_info *fi = findClassFieldForClass(parent, cls, "TYPE");
        if (fi == NULL) {
            warnx("boot: field TYPE missing for %s", primitiveClassMap[i].c_name);
            continue;
        }

        if (fi->static_operand == NULL || fi->static_operand->type == TYPE_NULL) {
            warnx("boot: field TYPE NULL for %s", primitiveClassMap[i].c_name);
            continue;
        }

        primitiveClassMap[i].class = fi->static_operand->val.vref;
    }

    jvm->primUp = true;

    if ((parent->thread = newObject(parent, findClass2(parent, "java.lang.Thread", false))) == NULL)
        errx(EXIT_FAILURE, "boot: unable to create Thread Object");
    parent->thread->lock++;

    cf = processNatives(parent, &java_lang_Math);
    if (cf == NULL) exit(1);
    cf = processNatives(parent, &java_lang_Double);
    if (cf == NULL) exit(1);
    cf = processNatives(parent, &java_lang_Float);
    if (cf == NULL) exit(1);

    cf = loadClass(parent, av[1]);

//#ifdef DEBUG
    printf("boot: VM loaded\n");
//#endif
    if (!cf) errx(EXIT_FAILURE, "boot: no main class");
    linkClass(parent, cf);

    const char *const clsname = av[2];
    const method_info *const m_Main = findClassMethod(parent, clsname, "main", "([Ljava.lang.String;)V");
    if (m_Main == NULL)
        errx(EXIT_FAILURE, "boot: Cannot find method main in class %s", clsname);

    parent->cur_method = m_Main;

    push(cur_frame, newOperand(TYPE_AREF,
                newArray(parent, TYPE_OREF, 0,
                    findClass(parent, "java.lang.String"))));

#ifdef DEBUG
    printf("\n\n\n\n\n");
#endif

    startThread(parent);

    const struct timespec rqtp = {
        .tv_sec = 2,
        .tv_nsec = 50000000L,
    };

    nanosleep(&rqtp, NULL);
    exit(1);
//#ifdef DEBUG
    printf("\n\nMain finished\n\n");
//#endif
}
