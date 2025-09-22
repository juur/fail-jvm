#ifndef _FAIL_JVM_H
#define _FAIL_JVM_H

#define _XOPEN_SOURCE 700

#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>

/* macros etc */
#define COLOUR
#ifdef COLOUR
#define ANSI_BRED   "\x1b[1;31m"
#define ANSI_INSTR  "\x1b[1;32m"
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

#define CLASSFILE_MAGIC (0xcafebabe)

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

/* invoke types */

typedef enum {
    INVOKE_VIRTUAL = 0x1,
    INVOKE_STATIC = 0x2,
    INVOKE_SPECIAL = 0x3,
    INVOKE_INTERFACE = 0x4
} invoke_t;

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

/*
 * Constant Pool
 */

/* value of .tag */

#define CONSTANT_Class              ((uint8_t)7)
#define CONSTANT_Fieldref           ((uint8_t)9)
#define CONSTANT_Methodref          ((uint8_t)10)
#define CONSTANT_InterfaceMethodref ((uint8_t)11)
#define CONSTANT_String             ((uint8_t)8)
#define CONSTANT_Integer            ((uint8_t)3)
#define CONSTANT_Float              ((uint8_t)4)
#define CONSTANT_Long               ((uint8_t)5)
#define CONSTANT_Double             ((uint8_t)6)
#define CONSTANT_NameAndType        ((uint8_t)12)
#define CONSTANT_Utf8               ((uint8_t)1)
#define CONSTANT_MethodHandle       ((uint8_t)15)
#define CONSTANT_MethodType         ((uint8_t)16)
#define CONSTANT_InvokeDynamic      ((uint8_t)18)

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

} cp_NameAndType_info;

typedef struct _cp_InvokeDynamic_info {
    uint16_t    bootstrap_method_attr_index;
    uint16_t    name_and_type_index;

    struct _cp_info *nametype;
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
    struct _Object  *intern;
    struct _cp_info *string;
} cp_String_info;

typedef struct _cp_Class_info {
    uint16_t    name_index;

    struct _cp_info *name;
} cp_Class_info;


/* Actually covers both Fieldref_info and Methodref_info */
typedef struct _cp_Fieldref_info {
    uint16_t    class_index;
    uint16_t    name_and_type_index;

    struct _cp_info     *class;
    struct _cp_info     *nametype;

    /* invokestatic et al cache these */
    struct _method_info *meth_cache;
    struct _method_info *meth_cache_static;
    struct _method_info *meth_cache2;
    struct _method_info *meth_cache_special;
} cp_Fieldref_info;

typedef struct _cp_info {
    uint8_t tag;
    bool dyn;

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
    };/* d; */
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
    const enum std_attr_e   attr;
    const char             *const name;
    const size_t            size;
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
    cp_info         *inner_name;

    struct _ClassFile   *inner_class;
} inner_class_def;

typedef struct _InnerClasses_attribute {
    uint16_t          number_of_classes;
    inner_class_def **classes;
} InnerClasses_attribute;

typedef struct _SourceFile_attribute {
    uint16_t   sourcefile_index;
    cp_info   *name;
} SourceFile_attribute;

typedef struct _Exceptions_attribute {
    uint16_t    number_of_exceptions;
    cp_info   **exception_index_table;
} Exceptions_attribute;

typedef union _OperandValue {
    struct _Object  *vref;
    int64_t          vlong;
    double           vdouble;
    float            vfloat;
    int32_t          vint;
    uint32_t         vretaddr;
    int16_t          vshort;
    uint16_t         vchar;
    int8_t           vbyte;
    bool             vbool;
} OperandValue;

typedef struct _Operand {
    OperandValue val;
    uint8_t      type;
    bool         freed;
} Operand;

typedef struct _ConstantValue_attribute {
    cp_info  *entry;
    Operand   op;
    uint16_t  constantvalue_index;
} ConstantValue_attribute;

typedef struct _bootstrap_argument {
    cp_info  *arg;
    uint16_t  arg_idx;
} bootstrap_argument;

typedef struct _bootstrap_method {
    bootstrap_argument    *bootstrap_arguments;

    uint16_t     bootstrap_method_ref;
    uint16_t     num_bootstrap_arguments;
} bootstrap_method;

typedef struct _BootstrapMethods_attribute {
    bootstrap_method    **bootstrap_methods;
    uint16_t              num_bootstrap_methods;
} BootstrapMethods_attribute;

typedef struct _exception {
    cp_info *catch_type;

    uint16_t start_pc;
    uint16_t end_pc;
    uint16_t handler_pc;
    uint16_t catch_type_idx;
} exception;

struct _attribute_info;

typedef struct _Code_attribute {
    uint16_t    max_stack;
    uint16_t    max_locals;
    uint32_t    code_length;
    uint16_t    exception_table_length;
    uint16_t    attributes_count;
    uint8_t    *code;
    exception **exception_table;

    struct _method_info        *method;
    struct _attribute_info    **attributes;
    LineNumberTable_attribute  *lineno;
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
        Code_attribute             *code;
        Exceptions_attribute       *exception;
        StackMapTable_attribute    *stackmap;
        ConstantValue_attribute    *constant;
        InnerClasses_attribute     *inner;
        SourceFile_attribute       *srcfile;
        LineNumberTable_attribute  *lineno;
        BootstrapMethods_attribute *bootstrap;
        void                       *data;
    } d;

    cp_info *name;

    /* must be last for unknown attributes */
    uint8_t  info[];
} attribute_info;

/* descriptors */

typedef struct _des_Field {
    uint8_t  baseType;
    uint8_t  array;
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
    cp_info          *descriptor;

    attribute_info   *code_attr;

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
    pthread_rwlock_t rwlock;

    cp_info     **constant_pool;
    cp_info     **interface_class;
    field_info  **fields;
    method_info **methods;
    cp_info      *this_class;
    cp_info      *super_class;
    cp_info      *srcfile;
    const char   *super_name;
    uint16_t     *interfaces;
    method_info  *init;

    attribute_info **attributes;
    struct _Object  *class_object;

    int32_t       heaploc;
    uint32_t      magic;

    uint16_t      minor_version;
    uint16_t      major_version;
    uint16_t      constant_pool_count;
    uint16_t      access_flags;

    uint16_t      this_class_idx;
    uint16_t      super_class_idx;
    uint16_t      interfaces_count;
    uint16_t      fields_count;

    uint16_t      methods_count;
    uint16_t      attributes_count;

    bool          isInit;
    bool          isLinked;
    bool          isInnerClass;
    bool          isDynamic;
} ClassFile;

/* Object */

typedef struct _Field {
    Operand             *op;
    field_info  *field_info;
} Field;

/* Object.type */
#define OBJ_CLASS   ((uint8_t)1)
#define OBJ_ARRAY   ((uint8_t)2)

typedef struct _Object {
    uint8_t  type;
    uint32_t lock;
    int32_t  heaploc;

    pthread_mutex_t  monitor;
    /* class->class_object in here is the java.lang.Class returned by getClass()
     * class is the in-memory wrapper
     * class->this_class is the constantpool entry
     */
    const ClassFile *class;
    //struct _Object  *class_obj;
    bool freed;

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
            const char       *className;
        } array;
    };/* data; */
} Object;

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
    struct _JVM     *jvm;
    method_info     *cur_method;

    uint32_t         pc;
    uint32_t         frame_free;
    pthread_t        pthread;
    uint16_t         thread_id;
    th_state_en      state;
    pthread_rwlock_t rwlock;
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
    intStaticMethod mstatic;
    intMethod       mvirtual;
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
    ClassFile   *class_file;
    //Object      *object;
} ary_map_t;


#endif
