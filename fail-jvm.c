#define _XOPEN_SOURCE 700
#if defined(DEBUG) || defined (DEBUG_CODE)
#define NDEBUG
#else
#undef NDEBUG
#endif

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define ERR_AT  __FILE__ ":" TOSTRING(__LINE__)
#define STRING_TOO_LONG() warnx("snprintf truncation:" ERR_AT)

#ifdef DEBUG_CODE
#define dbg_cprintf(...) printf(__VA_ARGS__)
#else
#define dbg_cprintf(...)
#endif

#ifdef DEBUG
#define dbg_printf(...) printf(__VA_ARGS__)
#else
#define dbg_printf(...)
#endif

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
#include <ctype.h>
#include <fcntl.h>

#include "fail-jvm.h"

/*
 * Forward function declarations
 */

#ifdef DEBUG
__attribute__((nonnull)) static void dumpAttribute(const attribute_info *ai);
#endif
__attribute__((nonnull)) static Object *getClassObject(Thread *, ClassFile *, bool);
__attribute__((nonnull)) static int32_t getLine(const Frame *fr);
__attribute__((nonnull)) static bool linkCpString(Thread *, cp_info *);
__attribute__((nonnull)) static bool fixupCpString(Thread *, cp_info *, cp_info **, uint16_t);
__attribute__((nonnull)) static bool linkClass(Thread *, ClassFile *);
__attribute__((nonnull)) static Operand *newStringFromChar(Thread *, const Object *, Object **);
__attribute__((nonnull)) static field_info *findClassFieldForClass(Thread *, ClassFile *, const char *);
__attribute__((nonnull)) static bool isClassLoaded(Thread *, const char *);
__attribute__((nonnull)) static void freeClass(JVM *jvm, ClassFile *cf);
__attribute__((nonnull)) static bool freeObject(JVM *jvm, Object *o);
__attribute__((nonnull)) static Operand *newString(Thread *, const char *, Object **);
__attribute__((nonnull)) static bool fixupCpUtf8(cp_info *);
__attribute__((nonnull(1,2))) static ClassFile *findClass2(Thread *, const char *, bool, Object *);
__attribute__((nonnull(1))) static Operand *throw(Thread *, const char *, const char *, int64_t pc);
__attribute__((nonnull)) static bool freeThread(Thread *restrict const t);
__attribute__((nonnull)) static Operand *startThread(Thread *);
__attribute__((malloc(freeThread,1),nonnull(1))) static Thread *newThread(JVM *, Object *);
__attribute__((nonnull(1,2))) static ClassFile *loadClass(Thread *, const char *, Object *);
__attribute__((nonnull(1))) static Object *newArray(Thread *, uint8_t, int32_t, const ClassFile *);
__attribute__((malloc(freeObject,2),nonnull(1,3))) static Object *newMultiArray(Thread *thread, uint8_t type, int32_t *sizes, uint8_t dim, const ClassFile *clsType);
__attribute__((malloc(freeObject,2))) static Object *newObject(Thread *, const ClassFile *const);
__attribute__((nonnull)) static uint32_t freeOperand(Operand *);
__attribute__((malloc(freeOperand,1))) static Operand *dupOperand(const Operand *);
__attribute__((malloc(freeOperand,1))) static Operand *newOperand(uint8_t, const void *);
__attribute__((nonnull(1,2))) static bool addClass(Thread *, ClassFile *, Object *);
__attribute__((nonnull(1,2))) static ClassFile *findClass(Thread *, const char *, Object *);
__attribute__((nonnull)) static method_info *findClassMethod(Thread *, const char *, const char *, const char *, invoke_t);
__attribute__((nonnull(1,3,4))) static method_info *findMethodByClass(Thread *, const ClassFile *, const char *, const char *, invoke_t);
__attribute__((nonnull)) static Operand *classInit(Thread *, ClassFile *cls);
__attribute__((nonnull)) static uint32_t freeAndNullOperand(Operand **o);
__attribute__((nonnull)) static Operand *invokeMethod(Thread *, method_info *, bool, int64_t, invoke_t);
__attribute__((nonnull(1,2))) static Object *createClass(Thread *, ClassFile *, Object *);
__attribute__((nonnull)) static bool getObjectFieldValue(const Object *obj, const char *name, void *out);
__attribute__((nonnull(1))) static ClassFile *createClassForArray(Thread *thread, const uint8_t type, const uint8_t dim, const ClassFile *compClass);
__attribute__((malloc(freeClass,2),nonnull)) static ClassFile *parseClass(Thread *, FILE *, const char *, Object *);

#ifdef DEBUG
static void dumpClass(const ClassFile *);
static const char *printObject(const Object *o);
#endif
#ifdef DEBUG
static void dumpFrame(const Frame *f);
static void dumpStack(const Frame *frame, const char *op);
static void disasm(const uint8_t *const code, uint32_t len);
#endif


/*
 * Constants
 */

#ifdef DEBUG
static const char *const invokeTypes[] = {
    [INVOKE_VIRTUAL]   = "INVOKE_VIRTUAL",
    [INVOKE_STATIC]    = "INVOKE_STATIC",
    [INVOKE_INTERFACE] = "INVOKE_INTERFACE",
    [INVOKE_SPECIAL]   = "INVOKE_SPECIAL",
    NULL,
};
#endif

static const struct predefinedAttribute predefinedAttrs[9] = {
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

static const char *const search[] = {
    "java",
    "java/lang",
    "java/lang/annotation",
    "java/lang/reflect",
    "java/math"
    "java/io",
    "java/nio",
    "java/nio/channels",
    "java/text",
    "java/util",
    "fail",
    "test",
    NULL
};

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


#if 0
static ClassFile *buildInternalClass(internalClass *ic);
#endif

/* global variables */

static uint16_t framecounter = 0;

static const char *opt_classpath = "cp";

static prim_class_map_t primitiveClassMap[10] = {
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
 * Inline function defintions
 */

static inline unsigned int umax(unsigned int a, unsigned int b)
{
    return a < b ? b : a;
}

__attribute__((nonnull))
static inline Frame *currentFrame(Thread * thread)
{
    if (thread->frame_free == 0) return NULL;
    return thread->stack[thread->frame_free-1];
}

__attribute__((nonnull))
static inline const char *getClassName(const cp_info *const cls) {
    if (cls->class.name != NULL && cls->class.name->utf8.ascii != NULL)
        return cls->class.name->utf8.ascii;
    return "<null>";
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
        case TYPE_OREF:     return "oreference (L)"; break;
        case TYPE_AREF:     return "areference ([)"; break;
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
static const char *printOpValue(const Operand *op);

__attribute__((nonnull))
static char *printArray(const Object *o)
{
    static char buf[BUFSIZ/2];
    FILE *bfp = fmemopen(buf, sizeof(buf), "w");

    for(uint32_t i = 0; i < o->array.len; i++)
    {
        if (o->array.data[i])
            fprintf(bfp, "%s", printOpValue(o->array.data[i]));
        if (o->array.type != TYPE_CHAR && i + 1 < o->array.len)
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
        case TYPE_OREF:     snprintf(buf, sizeof(buf), "%s @ %p",
                                    (op->val.vref->class && op->val.vref->class->this_class->class.name )
                                    ? getClassName(op->val.vref->class->this_class)
                                    : "<null class name>",
                                    (void *)op->val.vref); break;
        case TYPE_AREF:     snprintf(buf, sizeof(buf), "%s:%s",
                                    printOpType(op->val.vref->array.type),
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

#ifdef DEBUG
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

__attribute__((nonnull))
static inline bool push(Frame *const frame, const Operand *op)
{
    assert(op->type != 0);

    if (frame->sp >= frame->num_stack)
        return false;

    frame->stack[frame->sp++] = (Operand *)op;
    return true;
}

__attribute__((pure, nonnull))
static inline Operand *peeknth(const Frame *const frame, int pos)
{
    if (frame->sp == 0)
        return NULL;
    if ((frame->sp - pos - 1) > frame->sp)
        return NULL;

    return frame->stack[frame->sp - pos - 1];
}

__attribute__((pure, nonnull))
static inline Operand *peek(const Frame *const frame)
{
    if (frame->sp == 0)
        return NULL;

    return frame->stack[frame->sp - 1];
}

__attribute__((nonnull))
static inline Operand *pop(Frame *frame)
{
    assert(frame->sp > 0);

    Operand *ret = (Operand *)frame->stack[--frame->sp];
    frame->stack[frame->sp] = NULL;

    return ret;
}

__attribute__((pure))
static inline Operand *getLocal(const Frame *const frame, uint16_t slot)
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

__attribute__((nonnull))
static bool read4(FILE *const f, uint32_t *const dst)
{
    if ( (fread(dst, 1, sizeof(uint32_t), f)) != sizeof(uint32_t))
        return false;

    *dst = ntohl(*dst);
    return true;
}

__attribute__((nonnull))
static bool read2(FILE *const f, uint16_t *const dst)
{
    if ( (fread(dst, 1, sizeof(uint16_t), f)) != sizeof(uint16_t)) return false;
    *dst = ntohs(*dst);
    return true;
}

__attribute__((nonnull))
static bool read1(FILE *const f, uint8_t *const dst)
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

__attribute__((nonnull))
static cp_info *readConstantPool(FILE *const f)
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
            if (!read2(f, &ret->class.name_index)) goto fail;
            break;

        case CONSTANT_Fieldref:
        case CONSTANT_Methodref:
        case CONSTANT_InterfaceMethodref:
            if ((ret = calloc(1, sizeof(cp_info))) == NULL) return NULL;
            if (!read2(f, &ret->fr.class_index)) goto fail;
            if (!read2(f, &ret->fr.name_and_type_index))
                goto fail;
            break;

        case CONSTANT_String:
            if ((ret = calloc(1, sizeof(cp_info))) == NULL)
                return NULL;
            if (!read2(f, &ret->str.string_index)) goto fail;
            break;

        case CONSTANT_InvokeDynamic:
            if ((ret = calloc(1, sizeof(cp_info))) == NULL)
                return NULL;
            if (!read2(f, &ret->id.bootstrap_method_attr_index)) goto fail;
            if (!read2(f, &ret->id.name_and_type_index)) goto fail;
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
                ret->tdouble.c_double = val;
            }
            break;

        case CONSTANT_Long:
            if ((ret = calloc(1, sizeof(cp_info))) == NULL) return NULL;
            if (!read4(f, &tmp)) goto fail;
            ret->tlong.c_long = ((int64_t)tmp) << 32;
            if (!read4(f, &tmp)) goto fail;
            ret->tlong.c_long += tmp;
            break;

        case CONSTANT_Utf8:
            if (!read2(f, &tmp16)) goto fail;
            if ((ret = calloc(1, sizeof(cp_info))) == NULL)
                return NULL;
            ret->utf8.length = tmp16;
            //if (tmp16 == 0) {
            //  warnx("constant pool is zero length");
            //}
            if ((ret->utf8.bytes = calloc(tmp16 + 1U, sizeof(uint8_t))) == NULL)
                goto fail;
            if ((fread(ret->utf8.bytes, 1, tmp16, f)) != tmp16)
                goto fail;
            fixupCpUtf8(ret);
            break;

        case CONSTANT_NameAndType:
            if ((ret = calloc(1, sizeof(cp_info))) == NULL)
                return NULL;
            if (!read2(f, &ret->nt.name_index)) goto fail;
            if (!read2(f, &ret->nt.descriptor_index)) goto fail;
            break;

        case CONSTANT_MethodHandle:
            if ((ret = calloc(1, sizeof(cp_info))) == NULL)
                return NULL;
            if (!read1(f, &ret->mh.reference_kind))
                goto fail;
            if (!read2(f, &ret->mh.reference_index))
                goto fail;
            break;

        case CONSTANT_MethodType:
            if ((ret = calloc(1, sizeof(cp_info))) == NULL)
                return NULL;
            if (!read2(f, &ret->mt.descriptor_index))
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
                ret->tfloat.c_float = tmpf;
            }
            break;

        case CONSTANT_Integer:
            if ((ret = calloc(1, sizeof(cp_info))) == NULL) return NULL;
            if (!read4(f, &tmp)) goto fail;
            ret->tint.c_int = (int32_t)tmp;

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

__attribute__((nonnull))
static void freeFrame(Frame *f)
{
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

__attribute__((malloc(freeFrame,1)))
static Frame *newFrame(uint16_t num_stacks, uint16_t num_locals)
{
    Frame *ret;

    if ((ret = calloc(1, sizeof(Frame))) == NULL) {
        warn("newFrame: calloc");
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
    }
    return NULL;
}

__attribute__((nonnull))
static exception *readException(uint32_t me, FILE *const f, cp_info **const cp, uint32_t /*cpcnt*/)
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
__attribute__((nonnull))
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

__attribute__((nonnull))
static union verification_type_info *readVerificationTypeInfo(FILE *const f)
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

__attribute__((nonnull))
static attribute_info *readAttribute(Thread *const thread, uint32_t me, FILE *const f, cp_info **const cp, uint32_t cp_cnt)
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

        if (strcmp(cp[idx]->utf8.ascii, pa->name)) continue;
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
                        if (!strcmp(attr->attributes[j]->name->utf8.ascii, "LineNumberTable"))
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
                            attr->op.val.vint = attr->entry->tint.c_int;
                            break;
                        case CONSTANT_Long:
                            attr->op.type = TYPE_LONG;
                            attr->op.val.vlong = attr->entry->tlong.c_long;
                            break;
                        case CONSTANT_Float:
                            attr->op.type = TYPE_FLOAT;
                            attr->op.val.vfloat = attr->entry->tfloat.c_float;
                            break;
                        case CONSTANT_Double:
                            attr->op.type = TYPE_DOUBLE;
                            attr->op.val.vdouble = attr->entry->tdouble.c_double;
                            break;
                        case CONSTANT_String:
                            attr->op.type = TYPE_OREF;
                            if (jvm->VM) {
                                Object *tmp = NULL;
                                if (attr->entry->str.intern == NULL)
                                    linkCpString(thread, (cp_info *)attr->entry);

                                attr->op.val.vref = NULL;

                                if (newStringFromChar(thread, attr->entry->str.intern, &tmp) != NULL) {
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
                {
                    found = true;
                    if ((ret = calloc(1, sizeof(attribute_info))) == NULL) goto fail;
                    ret->attribute_name_index = idx;
                    ret->attribute_length = len;
                    BootstrapMethods_attribute *attr = calloc(1, pa->size);
                    if (attr == NULL) goto fail;
                    if (!read2(f, &attr->num_bootstrap_methods)) goto fail;
                    if ((attr->bootstrap_methods = calloc(attr->num_bootstrap_methods+1U,
                                    sizeof(cp_info *))) == NULL) goto fail;
                    ret->d.bootstrap = attr;

                    for (int j = 0; j < attr->num_bootstrap_methods; j++)
                    {
                        if ((attr->bootstrap_methods[j] = calloc(1, sizeof(bootstrap_method))) == NULL) goto fail;
                        if (!read2(f, &attr->bootstrap_methods[j]->bootstrap_method_ref)) goto fail;
                        if (!read2(f, &attr->bootstrap_methods[j]->num_bootstrap_arguments)) goto fail;
                        if ((attr->bootstrap_methods[j]->bootstrap_arguments =
                                    calloc(attr->bootstrap_methods[j]->num_bootstrap_arguments,
                                        sizeof(bootstrap_argument))) == NULL)
                            goto fail;
                        for (int k = 0; k < attr->bootstrap_methods[j]->num_bootstrap_arguments; k++)
                            if (!read2(f, &attr->bootstrap_methods[j]->bootstrap_arguments[k].arg_idx)) goto fail;
                    }
                }
                break;
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

    }

    ret->name = cp[idx];

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

__attribute__((nonnull))
static const char *parseParamDescriptor(des_Field *const desc, const char *const str)
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
//                desc->className = strndup((char *)ptr, (size_t)(tmp-ptr-1U));
//                desc->orig_className = strdup(desc->className);
//                for(ptr = (uint8_t *)desc->className; *ptr && *ptr != ';'; ptr++)
//                    if(*ptr == '/') *ptr = '.';
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

__attribute__((nonnull))
static field_info *readField(Thread *const thread, FILE *const f, cp_info **const cp, uint32_t cp_cnt, ClassFile *const cls)
{
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

    if (!parseParamDescriptor(&ret->desc, ret->descriptor->utf8.ascii)) {
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
            if (attrs[i] && !strcmp(attrs[i]->name->utf8.ascii, "ConstantValue")) {
                if (ret->static_operand != NULL)
                    warnx("readField: multiple static_operand");
                else
                    ret->static_operand = dupOperand(&attrs[i]->d.constant->op);
            }

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
        freeAndNullOperand(&ret->static_operand);
        free(ret);
        ret = NULL;
    }
    return NULL;
}


__attribute__((nonnull))
static bool parseMethodDescriptor(des_Method *const desc, const char *const str)
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

__attribute__((nonnull))
static method_info *readMethod(Thread *const thread, FILE *const f, cp_info **const cp, uint32_t cp_cnt, ClassFile *const class)
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

    if (!parseMethodDescriptor(&ret->desc, ret->descriptor->utf8.ascii)) {
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
        else if (!strcmp(ret->attributes[i]->name->utf8.ascii, "Exceptions"))
            ret->exceptions = ret->attributes[i]->d.exception;
        else if (!strcmp(ret->attributes[i]->name->utf8.ascii, "Code"))
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

__attribute__((nonnull))
static uint16_t *parseCpUtf8ToUtf16(const uint8_t *src, register uint16_t len, uint16_t *utf16len)
{
    register uint16_t ptr = 0;
    register uint16_t dst = 0;
    uint16_t *ret = NULL;
    uint16_t *tmp = NULL;

    if (len == 0) {
        *utf16len = 0;
        return NULL;
    }

    if ((ret = malloc((int)len * sizeof(uint16_t))) == NULL) {
        warn("parseCpUtf8ToUtf16: malloc");
        return NULL;
    }

    while (ptr < len)
    {
        register uint8_t val = src[ptr++];

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

    if ((tmp = realloc(ret, dst * sizeof(uint16_t))) == NULL) {
        warn("parseCpUtf8ToUtf16: realloc: %d", dst);
        goto fail;
    }

    ret = tmp;
    *utf16len = dst;

    return ret;
fail:
    if (ret) free(ret);
    return NULL;
}

__attribute__((nonnull))
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

__attribute__((nonnull))
static bool fixupCpUtf8(cp_info *ci)
{
    if (ci->utf8.utf16 == NULL)
        ci->utf8.utf16 = parseCpUtf8ToUtf16(ci->utf8.bytes, ci->utf8.length, &ci->utf8.utf16len);
    if (ci->utf8.ascii == NULL)
        ci->utf8.ascii = parseUtf16ToAscii(ci->utf8.utf16, ci->utf8.utf16len);
    return true;
}

/* TODO - do not normalise the name here */
__attribute__((nonnull))
static bool fixUtf8ClassName(cp_info *ci)
{
    if (ci->utf8.ascii) {
        if (strchr(ci->utf8.ascii, '/') == NULL)
            return true;
#ifdef DEBUG
        //printf("fixUtf8ClassName: %s => ", ci->utf8.ascii);
#endif
        free(ci->utf8.ascii);
        ci->utf8.ascii = NULL;
    }

    if (ci->utf8.utf16) {
        free(ci->utf8.utf16);
        ci->utf8.utf16 = NULL;
    }

    for (uint16_t i = 0; i < ci->utf8.length; i++)
    {
        uint8_t val = ci->utf8.bytes[i];

        if ((val & 0xe0) == 0xc0) {
            i++;
        } else if ((val & 0xf0) == 0xe0) {
            i++; i++;
        } else if (val == '/') {
            ci->utf8.bytes[i] = '.';
        }
    }

    if (!fixupCpUtf8(ci))
        return false;
#ifdef DEBUG
    //printf("%s\n", ci->utf8.ascii);
#endif
    return true;
}

__attribute__((nonnull))
static bool fixupCpString(Thread * /* thread */, cp_info *ci, cp_info **pool, uint16_t cp_len)
{
    if (ci->str.string && ci->str.intern)
        return true;

    if (ci->str.string_index >= cp_len)
        return false;
    if (pool[ci->str.string_index]->tag != CONSTANT_Utf8)
        return false;

    ci->str.string = pool[ci->str.string_index];

    return true;
}

__attribute__((nonnull))
static bool linkCpString(Thread *thread, cp_info *ci)
{
    if ((ci->str.intern = newArray(thread, TYPE_CHAR, ci->str.string->utf8.utf16len, NULL)) == NULL)
        return false;

    // lock-- in freeClass
    ((Object *)ci->str.intern)->lock++;

    fixupCpUtf8((cp_info *)ci->str.string);

    assert(ci->str.intern->array.len == ci->str.string->utf8.utf16len);

    for (uint32_t i = 0; i < ci->str.string->utf8.utf16len; i++) {
        freeAndNullOperand(&ci->str.intern->array.data[i]);
        ci->str.intern->array.data[i] = newOperand(TYPE_CHAR, &ci->str.string->utf8.utf16[i]);
    }

    return true;
}

__attribute__((nonnull))
static bool fixupCpClass(cp_info *ci, cp_info **pool, uint16_t cp_len)
{
    if (ci->class.name)
        return true;

    if (ci->class.name_index >= cp_len) return false;
    if (pool[ci->class.name_index]->tag != CONSTANT_Utf8) return false;

    ci->class.name = pool[ci->class.name_index];

    return true;
}

__attribute__((nonnull))
static bool fixupCpFieldref(cp_info *ci, cp_info **pool, uint16_t cp_len)
{
    if (ci->fr.class && ci->fr.nametype)
        return true;

    if (ci->fr.class_index >= cp_len || ci->fr.name_and_type_index >= cp_len)
        return false;

    ci->fr.class = pool[ci->fr.class_index];
    ci->fr.nametype = pool[ci->fr.name_and_type_index];

    if (ci->fr.class->tag != CONSTANT_Class || ci->fr.nametype->tag != CONSTANT_NameAndType)
        return false;

    return true;
}

__attribute__((nonnull))
static bool fixupCpNameAndType(cp_info *ci, cp_info **pool, uint16_t cp_len)
{
    if (ci->nt.name && ci->nt.descriptor) return true;

    if (ci->nt.name_index >= cp_len || ci->nt.descriptor_index >= cp_len)
        return false;

    ci->nt.name = pool[ci->nt.name_index];
    ci->nt.descriptor = pool[ci->nt.descriptor_index];

    if (ci->nt.name->tag != CONSTANT_Utf8 || ci->nt.descriptor->tag != CONSTANT_Utf8)
        return false;

    return true;
}

__attribute__((nonnull))
static bool fixupCpInvokeDynamic(cp_info *ci, cp_info **pool, uint16_t cp_len)
{
    if (ci->id.nametype)
        return true;

    if (ci->id.name_and_type_index >= cp_len)
        return false;

    ci->id.nametype = pool[ci->id.name_and_type_index];
    if (ci->id.nametype->tag != CONSTANT_NameAndType)
        return false;

    return true;
}

__attribute__((nonnull))
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
        case CONSTANT_InvokeDynamic:
            return fixupCpInvokeDynamic(ci, pool, cp_len);
        default:
            break;
    }
    return true;
}

__attribute__((nonnull))
static bool fixupPostConstant(Thread * /*thread*/, cp_info *ci, cp_info ** /*pool*/, uint16_t /*cp_len*/)
{
    bool ret = true;

    switch(ci->tag)
    {
        case CONSTANT_Class:
            return fixUtf8ClassName(ci->class.name);

        case CONSTANT_NameAndType:
            return fixUtf8ClassName(ci->nt.descriptor);

        default:
            break;
    }
    return ret;
}

__attribute__((nonnull(1,2,3)))
static inline Operand *loadInnerClass(Thread *thread, const InnerClasses_attribute *ic,
        const ClassFile *cf, Object *classLoader)
{
    for (int i = 0; i < ic->number_of_classes; i++)
    {
        inner_class_def *const def = ic->classes[i];
        const char *clsname = getClassName(def->inner_ci);

        if(!def || !strcmp(clsname, getClassName(cf->this_class)))
            continue;

        if(isClassLoaded(thread, clsname))
            continue;

        if ((def->inner_class = loadClass(thread, clsname, classLoader)) == NULL) {
            warnx("loadInnerClass: unable to load %s", clsname);
            return throw(thread, "java.lang.ClassNotFoundException", NULL, 0);
        }
        dbg_printf("(loadInnerClass: %s)\n", clsname);
        def->inner_class->isInnerClass = true;

        /* TODO is this right? why link inners but not outers? */
        /*if (thread->jvm->VM)
            linkClass(thread, (ClassFile *)def->inner_class);
        else*/
            def->inner_class->isLinked = false;
    }

    return NULL;
}

__attribute__((nonnull))
static Field *findFieldByName(const Object *const o, const char *const name)
{
    const field_info *fi;

    for (int i = 0; i < o->obj.num_fields; i++)
    {
        if ((fi = (field_info *)o->obj.fields[i].field_info) == NULL)
            continue;

        if (strcmp(fi->name->utf8.ascii, name)) continue;

        return &o->obj.fields[i];
    }

    return NULL;
}

/* not-jvm-safe */
__attribute__((nonnull))
static Operand *newStringFromChar(Thread *thread, const Object *buf,
        Object **result)
{
    assert(thread != NULL);
    assert(buf != NULL);
    assert(result != NULL);
    assert(thread->jvm->VM == true);
    Frame *cur_frame = currentFrame(thread);

    /* validate inputs */
    //if (buf == NULL) {
    //    warnx("newStringFromChar: buf == NULL");
    //    return throw(thread, "java.lang.NullPointerException", ERR_AT, 0);
    //}

    if (buf->type != OBJ_ARRAY || buf->array.type != TYPE_CHAR) {
        warnx("newStringFromChar: buf not [C");
        return throw(thread, "java.lang.IllegalArguementException", ERR_AT, 0);
    }

    /* create the String */
    ClassFile *str_cls = findClass(thread, "java.lang.String", NULL);
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
    method_info *mi = findMethodByClass(thread, str_cls, "<init>", "([C)V", INVOKE_SPECIAL);
    assert(mi != NULL);
    Operand *ex_opr = NULL;
    if ((ex_opr = invokeMethod(thread, mi, true, thread->pc, INVOKE_SPECIAL)) != NULL) {
        warnx("newStringFromChar: <init> ([C)V threw");
        return ex_opr;
    }

    /* store the result if no exception thrown */
    *result = str_obj;
    return NULL;
}

/* not-jvm-safe */
__attribute__((nonnull))
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
        assert(i < array->array.len);
        const uint16_t ch = (uint8_t)text[i];
        freeAndNullOperand(&array->array.data[i]);
        array->array.data[i] = newOperand(TYPE_CHAR, &ch);
        assert(array->array.data[i] != NULL);
    }

    return newStringFromChar(thread, array, result);
}

/* TODO move to JVM? */
static uint32_t num_ary_maps = 0;
static ary_map_t *ary_maps = NULL;
static pthread_rwlock_t clslock;

__attribute__((nonnull(1)))
ClassFile *createClassForArray(Thread *thread, const uint8_t type, const uint8_t dim, const ClassFile *compClass)
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
        strcat(buf, getClassName(compClass->this_class));
        strcat(buf, ";");
    }

    dbg_printf("createClassForArray: %s[%d] of %s\n", printOpType(type), dim, buf);

    if ((rc = pthread_rwlock_tryrdlock(&clslock)) != 0) {
        strerror_r(rc, errbuf, sizeof(errbuf));
        warnx("createClassForArray(%s): pthread_rwlock_rdlock: %s", buf, errbuf);
        return NULL;
    }

    for (ssize_t i = 0; i < num_ary_maps; i++)
        if (ary_maps[i].canonical && !strcmp(buf, ary_maps[i].canonical)) {
            pthread_rwlock_unlock(&clslock);
            dbg_printf("createClassForArray: returning existing Class Object for %s\n", buf);
            return ary_maps[i].class_file;
        }

    /* TODO we need to preserve the lock until we've added it*/
    pthread_rwlock_unlock(&clslock);

    ClassFile *jlClass_cf = findClass2(thread, "java.lang.Class", false, NULL);
    if (!jlClass_cf)
        return NULL;

    ClassFile *jlObject_cf = findClass2(thread, "java.lang.Object", false, NULL);
    if (!jlObject_cf)
        return NULL;

    Frame *cur_frame = currentFrame(thread);

    /* create the ClassFile that this Class object will wrap */
    ClassFile *new_cf = calloc(1, sizeof(ClassFile));
    if ((rc = pthread_rwlock_init(&new_cf->rwlock, NULL))) {
        /* TODO */
        exit(EXIT_FAILURE);
    }

    new_cf->magic = CLASSFILE_MAGIC;
    new_cf->major_version = 55;
    new_cf->minor_version = 0;
    new_cf->access_flags = ACC_PUBLIC|ACC_FINAL|ACC_ABSTRACT;
    new_cf->super_class = jlObject_cf->this_class;
    new_cf->super_name = strdup("java.lang.Object");
    new_cf->this_class = calloc(1, sizeof(cp_info));
    new_cf->this_class->dyn = true;
    new_cf->this_class->class.name = calloc(1, sizeof(cp_info));
    new_cf->this_class->class.name->utf8.ascii = strdup(buf);
    //new_cf->isDynamic = true;

    addClass(thread, new_cf, NULL);
    linkClass(thread, new_cf);

    /* create an instance of the Class object */
    //Object *ret = newObject(thread, jlClass_cf);
    Object *ret = new_cf->class_object = createClass(thread, new_cf, NULL);

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
    //ary_maps[num_ary_maps].object = ret;
    ary_maps[num_ary_maps].class_file = new_cf;
    if ((ary_maps[num_ary_maps].canonical = strdup(buf)) == NULL) {
        warn("createClassForArray: strdup");
        ret->lock--;
        //ary_maps[num_ary_maps].object = NULL;
    }
    dbg_printf("createClassForArray: bumped ary_maps for %s\n", ary_maps[num_ary_maps].canonical);
    num_ary_maps++;

    pthread_rwlock_unlock(&clslock);

    method_info *mi = findMethodByClass(thread, jlClass_cf, "<init>", "()V", INVOKE_SPECIAL);
    if (!push(cur_frame, newOperand(TYPE_OREF, ret))) {
        ret->lock--;
        warnx("createClassForArray: push");
        return NULL;
    }

    /* call the 'new' method on the Class object */
    if (invokeMethod(thread, mi, true, thread->pc, INVOKE_SPECIAL))
        errx(EXIT_FAILURE, "createClassForArray: thrown");

    Field *fld = NULL;
    Object *str = NULL;
    Operand tmpOp;

    if ((fld = findFieldByName(ret, "isArray")) == NULL)
        errx(EXIT_FAILURE, "createClassForArray: name");
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
        snprintf(buf, BUFSIZ, "%s", getClassName(compClass->this_class)); // FIXME
    else
        snprintf(buf, BUFSIZ, "%s", printOpType(type));

    for (ssize_t i = 0; i < dim; i++)
        strcat(buf, "[]");

    if ((fld = findFieldByName(ret, "canonicalName")) == NULL)
        errx(EXIT_FAILURE, "createClassForArray: name");
    freeAndNullOperand(&fld->op);

    if ((newString(thread, buf, &str)) != NULL)
        errx(EXIT_FAILURE, "createClassForArray: newString threw");
    freeAndNullOperand(&fld->op);
    fld->op = newOperand(TYPE_OREF, str);

    /* default is false?
    if ((fld = findFieldByName(ret, "isPrimitive")) == NULL)
        errx(EXIT_FAILURE, "createClassForArray: isPrimitive");
    freeAndNullOperand(&fld->op);
    tmpOp.val.vbool = false;
    fld->op = newOperand(TYPE_BOOL, &tmpOp); */

    if ((fld = findFieldByName(ret, "componentType")) == NULL)
        errx(EXIT_FAILURE, "createClassForArray: componentType");
    freeAndNullOperand(&fld->op);

    if (compClass) {
        if ((fld->op = newOperand(TYPE_OREF, getClassObject(thread, (ClassFile *)compClass, true))) == NULL)
            errx(EXIT_FAILURE, "createClassForArray: unable to getClassObject for compClass");
    } else if (thread->jvm->primUp) {
        if ((fld->op = newOperand(TYPE_OREF, findObjForPrim(type))) == NULL)
            errx(EXIT_FAILURE, "createClassForArray: unable to findObjForPrim for %s", printOpType(type));
    } else {
        fld->op = newOperand(TYPE_NULL, &type);
        if (thread->jvm->primUp)
            warnx("createClassForArray: NULL componentType for %s", printOpType(type));
    }

    // TODO componentType for primitives

    return new_cf;
}

/* not-jvm-safe */
__attribute__((nonnull(1,2)))
static Object *createClass(Thread *thread, ClassFile *cf, Object *classLoader)
{
    assert(thread->jvm->VM == true);

    ClassFile *class_cf = findClass2(thread, "java.lang.Class", false, classLoader);
    if (!class_cf) return NULL;

    Frame *cur_frame = currentFrame(thread);
    Object *ret = newObject(thread, class_cf);
    // lock-- in freeClass
    ret->lock++;
    method_info *mi = findMethodByClass(thread, class_cf, "<init>", "()V", INVOKE_SPECIAL);
    push(cur_frame, newOperand(TYPE_OREF, ret));
    if (invokeMethod(thread, mi, true, thread->pc, INVOKE_SPECIAL))
        errx(EXIT_FAILURE, "createClass: thrown");

    Field *fld = NULL;
    Object *str = NULL;

    if ((fld = findFieldByName(ret, "canonicalName")) == NULL)
        errx(EXIT_FAILURE, "createClass: canonicalName");
    if (newString(thread, getClassName(cf->this_class), &str))
        errx(EXIT_FAILURE, "createClass: canonicalName newString");

    freeAndNullOperand(&fld->op);
    fld->op = newOperand(TYPE_OREF, str);

    const char *class_name;

    class_name = getClassName(cf->this_class);

    const char *simple_name;

    if ((simple_name = strrchr(class_name, '.')) == NULL)
        simple_name = class_name;
    else
        simple_name++;

    const char *package_name;
    const char *tmp;

    if ((tmp = strrchr(class_name, ',')) == NULL)
        package_name = strdup("");
    else {
        package_name = strndup(class_name, tmp - class_name);
    }

    if ((fld = findFieldByName(ret, "packageName")) == NULL)
        errx(EXIT_FAILURE, "createClass: packageName");
    if (newString(thread, package_name, &str))
        errx(EXIT_FAILURE, "createClass: packageName newString");

    freeAndNullOperand(&fld->op);
    fld->op = newOperand(TYPE_OREF, str);

    free((char *)package_name);

    if ((fld = findFieldByName(ret, "simpleName")) == NULL)
        errx(EXIT_FAILURE, "createClass: simpleName");
    if (newString(thread, simple_name, &str))
        errx(EXIT_FAILURE, "createClass: simpleName newString");

    freeAndNullOperand(&fld->op);
    fld->op = newOperand(TYPE_OREF, str);

    if ((fld = findFieldByName(ret, "name")) == NULL)
        errx(EXIT_FAILURE, "createClass: name");
    if (newString(thread, getClassName(cf->this_class), &str))
        errx(EXIT_FAILURE, "createClass: name newString");

    freeAndNullOperand(&fld->op);
    fld->op = newOperand(TYPE_OREF, str);

    return ret;
}

static ClassFile *parseClass(Thread *thread, FILE *fp, const char *file_name, Object *classLoader)
{
    ClassFile *ret = calloc(1, sizeof(ClassFile));
    if (ret == NULL) err(EXIT_FAILURE, "parseClass: calloc");

    char buf[BUFSIZ] = {0};

    int rc;
    if ((rc = pthread_rwlock_init(&ret->rwlock, NULL))) {
        strerror_r(rc, buf, sizeof(buf));
        errx(EXIT_FAILURE, "parseClass: [%s] pthread_rwlock_init: %s", file_name, buf);
    }

    if (!read4(fp, &ret->magic)) errx(EXIT_FAILURE, "parseClass: read magic");

    if (ret->magic != CLASSFILE_MAGIC)
        errx(EXIT_FAILURE, "parseClass: [%s] Bad magic: 0x%0x", file_name, ret->magic);

    if (!read2(fp, &ret->minor_version)) err(EXIT_FAILURE, "parseClass: read minor");
    if (!read2(fp, &ret->major_version)) err(EXIT_FAILURE, "parseClass: read major");

    if (ret->major_version > 55) {
        warnx("parseClass: [%s] Unsupported JRE version: %d.%d",
                file_name, ret->major_version, ret->minor_version);
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

//      ret->methods[i]->original_descriptor = strdup(ret->methods[i]->descriptor->utf8.ascii);
//      ret->methods[i]->original_name       = strdup(ret->methods[i]->name->utf8.ascii);

        /* fix class naming style - TODO preserve?*/
        if (strchr(ret->methods[i]->descriptor->utf8.ascii, '/') != NULL)
            fixUtf8ClassName(ret->methods[i]->descriptor);

        /* link Code segment */
        for(int j = 0; j < ret->methods[i]->attributes_count; j++)
        {
            if (!strcmp(ret->methods[i]->attributes[j]->name->utf8.ascii, "Code"))
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

    for(int i = 1; i < ret->constant_pool_count; i++) {
        if (!fixupPostConstant(thread, (cp_info *)ret->constant_pool[i], ret->constant_pool,
                    ret->constant_pool_count))
            errx(EXIT_FAILURE, "fixupPostConstantPool[%d]", i);

        if (ret->constant_pool[i]->tag == CONSTANT_Long ||
                ret->constant_pool[i]->tag == CONSTANT_Double) i++;
    }

    if (ret->super_class)
        if ((ret->super_name = strdup(getClassName(ret->super_class))) == NULL)
            err(EXIT_FAILURE, "parseClass: strdup(super_class name)");

    /* TODO preserve the on-disk format elsewhere, making this human readable */

    if ((ret->interface_class = calloc(ret->interfaces_count+1U,
                    sizeof(cp_info *))) == NULL)
        errx(EXIT_FAILURE, "parseClass: calloc(%d) interface_class", ret->interfaces_count+1U);

    for(uint32_t i = 0; i < ret->interfaces_count; i++)
        ret->interface_class[i] = ret->constant_pool[ret->interfaces[i]];

    if ((rc = pthread_rwlock_wrlock(&ret->rwlock))) {
        strerror_r(rc, buf, sizeof(buf));
        errx(EXIT_FAILURE, "parseClass: pthread_rwlock_lock: %s", buf);
    }

    if (!addClass(thread, ret, classLoader)) {
        warnx("parseClass: unable to addClass");
        return NULL;
    }

    ret->isLinked = false;

    for(uint32_t i = 0; i < ret->attributes_count; i++)
        if (!strcmp(ret->attributes[i]->name->utf8.ascii, "InnerClasses")) {
            /* FIXME something screwy is happening in loadInnerClass,
             * but by bypassing this we wdon't set def->inner_class->isInnerClass etc.
             * but .. does that matter? */
            //loadInnerClass(thread, ret->attributes[i]->d.inner, ret);
        } else if (!strcmp(ret->attributes[i]->name->utf8.ascii, "SourceFile")) {
            ret->srcfile = ret->attributes[i]->d.srcfile->name;
        }

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

__attribute__((nonnull))
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

__attribute__((nonnull(1)))
static Operand *throw(Thread *thread, const char *name, const char *text,
        int64_t pc)
{
    assert(thread->jvm->VM == true);
#ifdef DEBUG
    Frame *cur = currentFrame(thread);
#endif
    dbg_printf(ANSI_BYEL "(throw %s %s pc:%lu: ",  name, text, pc);
    dbg_printf("mi:%s ", cur->mi->name->utf8.ascii);
    dbg_printf("cls:%s ", getClassName(cur->class->this_class));
    dbg_printf(")\n" ANSI_RESET);

    //abort();

    Frame *cur_frame = currentFrame(thread);
    while(cur_frame->sp)
        freeOperand(pop(cur_frame));

    Operand *ex_opr = NULL;
    ClassFile *thrcls = findClass(thread, name, NULL);

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

    /* INVOKE_??? */
    method_info *mi = findMethodByClass(thread, thrcls,
            "<init>", text ? "(Ljava.lang.String;)V" : "()V", INVOKE_VIRTUAL);
    if ((ex_opr = invokeMethod(thread, mi, true, pc, INVOKE_VIRTUAL)) != NULL)
        return ex_opr;

    if(!push(cur_frame, newOperand(TYPE_OREF, thr)))
        errx(EXIT_FAILURE, "throw: unable to init");

    /* INVOKE_??? */
    mi = findMethodByClass(thread, thrcls,
            "fillInStackTrace", "()Ljava.lang.Throwable;", INVOKE_VIRTUAL);
    if ((ex_opr = invokeMethod(thread, mi, true, pc, INVOKE_VIRTUAL)) != NULL)
        return ex_opr;

    freeOperand(pop(cur_frame));

    return newOperand(TYPE_OREF, thr);
}

__attribute__((nonnull))
static bool isClassLoaded(Thread *thread, const char *clsname)
{
    JVM *jvm = (JVM *)thread->jvm;

    dbg_printf("(isClassLoaded: %s)\n", clsname);

    if (pthread_rwlock_rdlock(&jvm->rwlock))
        errx(EXIT_FAILURE, "isClassLoaded: pthread_rwlock_rdlock");

    for (ssize_t i = 0; i < MAX_METHOD; i++)
    {
        if (!jvm->method_area[i])
            continue;
        if (!strcmp(getClassName(jvm->method_area[i]->this_class), clsname)) {
            pthread_rwlock_unlock(&jvm->rwlock);
            return true;
        }
    }

    pthread_rwlock_unlock(&jvm->rwlock);
    return false;
}

/* setting load to false prevents recursive hell */
static ClassFile *findClass2(Thread *const thread, const char *const clsname, const bool load, Object *classLoader)
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
        if (!strcmp(getClassName(jvm->method_area[i]->this_class),
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
        tmp = loadClass(thread, clsname, classLoader);
        if (!tmp) return NULL;
        if (thread->jvm->VM && !linkClass(thread, tmp)) return NULL;
        ret = tmp;
    }
    if (!ret)
        printf("findClass2: %s\n", clsname);
    return ret;
}

__attribute__((nonnull(1,3)))
static bool canCast(Thread *thread, const char *from, const char *to)
{
    /* null can always be cast to any Object? */
    if (from == NULL)
        return true;

    if (!strcmp(from, to)) return true;
    if (!strcmp("java.lang.Object", to)) return true; /* this is a bit shit FIXME */

    ClassFile *fromCls = findClass(thread, from, NULL);
    ClassFile *toCls = findClass(thread, to, NULL);

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
__attribute__((nonnull(1,2)))
static ClassFile *findClass(Thread *const thread, const char *const clsname, Object *classLoader)
{
    assert(thread->jvm->VM == true);
    return findClass2(thread, clsname, true, classLoader);
}

/*
 * TODO: check the search method aligns to the specification, specifically
 * with interfaces vs parent classes
 */
__attribute__((nonnull(1,3,4)))
static method_info *findMethodByClass(Thread *thread, const ClassFile *cls, const char *mthname, const char *desc, invoke_t invoke)
{
    dbg_printf("check_method: invoke is %s\n", invokeTypes[invoke]);

    if (cls == NULL) {
        warnx("findMethodByClass: cls == NULL looking for method %s and desc %s", mthname, desc);
        abort();
        return NULL;
    }

    /* there is a bug here in that sometimes an abstract method of an interface is returned,
     * instead of the concrete implementation in a super class */

    if (invoke == INVOKE_VIRTUAL || invoke == INVOKE_SPECIAL || invoke == INVOKE_STATIC || invoke == INVOKE_INTERFACE)
        for (ssize_t i = 0; i < cls->methods_count; i++)
        {
            if ( (cls->methods[i]->access_flags & ACC_STATIC) && invoke != INVOKE_STATIC && invoke != INVOKE_SPECIAL)
                continue;

            dbg_printf("check_method: (%s) %s:%s == %s:%s\n", 
                    getClassName(cls->this_class),
                    mthname, desc, 
                    cls->methods[i]->name->utf8.ascii, cls->methods[i]->descriptor->utf8.ascii);
            if (!strcmp(cls->methods[i]->name->utf8.ascii, mthname) &&
                    !strcmp(desc, cls->methods[i]->descriptor->utf8.ascii)) {
                    /* && (permitAbstract || !(cls->methods[i]->access_flags & ACC_ABSTRACT))*/
                dbg_printf("check_method: MATCH\n");
                return cls->methods[i];
            }
        }

    /* should this check if the parent class is an interface? */
    if (cls->super_name) {
        method_info *mi = NULL;
        //dbg_printf("check_method: super: %s\n", cls->super_name);
        if ((mi = findClassMethod(thread, cls->super_name, mthname, desc, invoke)) != NULL)
            return mi;
    }

    if (invoke == INVOKE_INTERFACE || invoke == INVOKE_VIRTUAL)
        if (cls->interfaces_count > 0) {
            method_info *mi = NULL;
            for (ssize_t i = 0; i < cls->interfaces_count; i++) {
                dbg_printf("check_method: interface %s\n", getClassName(cls->interface_class[i]));
                if ((mi = findClassMethod(thread,
                                getClassName(cls->interface_class[i]), mthname,
                                desc, invoke)) != NULL)
                    return mi;
            }
        }

    dbg_printf("check_method: fail: %s %s\n", mthname, desc);
    return NULL;
}

    __attribute__((nonnull))
static method_info *findClassMethod(Thread *thread, const char *clsname, const char *mthname, const char *desc, invoke_t invoke)
{
    assert(thread->jvm->VM == true);
    ClassFile *cls = findClass(thread, clsname, NULL);
    if (cls == NULL) return NULL;

    dbg_printf("findClassMethod: converted %s to %s\n", clsname, getClassName(cls->this_class));

    return findMethodByClass(thread, cls, mthname, desc, invoke);
}

    __attribute__((nonnull))
static field_info *findClassField(Thread *thread, const char *clsname, const char *fldname)
{
    assert(thread->jvm->VM == true);
    ClassFile *cls = findClass(thread, clsname, NULL);
    if (cls == NULL) {
        warnx("findClassField: no such class %s", clsname);
        return NULL;
    }

    return findClassFieldForClass(thread, cls, fldname);
}

    __attribute__((nonnull))
static field_info *findClassFieldForClass(Thread *thread, ClassFile *cls, const char *fldname)
{
    for (ssize_t i = 0; i < cls->fields_count; i++)
        if (!strcmp(cls->fields[i]->name->utf8.ascii, fldname))
            return cls->fields[i];

    if(cls->super_class)
        return findClassField(thread, cls->super_name, fldname);
    return NULL;
}

    __attribute__((nonnull))
static attribute_info *findAttribute(attribute_info **attrs, uint16_t num, const char *name)
{
    for(uint16_t i = 0; i < num; i++)
        if (!strcmp(attrs[i]->name->utf8.ascii, name))
            return (attrs[i]);
    return NULL;
}

    __attribute__((nonnull))
static Operand *classInit(Thread *thread, ClassFile *cls)
{
    assert(thread->jvm->VM == true);

    char buf[BUFSIZ] = {0};
    int rc;
    method_info *mi = NULL;
    Operand *ret = NULL;
    Operand *ex_opr = NULL;


    if (cls->isInit) // FIXME lock the class first
        return NULL;


    if ((rc = pthread_rwlock_wrlock(&cls->rwlock))) {
        strerror_r(rc, buf, sizeof(buf));
        warnx("classInit: pthread_rwlock_wrlock(class): %s", buf);
        return NULL; // FIXME error?
    }

    if (cls->isInit) {
        goto done;
    }

    dbg_printf("(classInit: %s)\n", getClassName(cls->this_class));

    cls->isInit = true;

    /* we can't create String Objects during class load, as things may not be ready */
    for (ssize_t i = 0; i < cls->fields_count; i++) {
        if (!cls->fields[i] || cls->fields[i]->desc.baseType != TYPE_OREF)
            continue;

        for (ssize_t j = 0; j < cls->fields[i]->attributes_count; j++)
            if (cls->fields[i]->attributes[j] && !strcmp("ConstantValue",
                        cls->fields[i]->attributes[j]->name->utf8.ascii))
            {
                attribute_info *ai = cls->fields[i]->attributes[j];

                if (ai->d.constant->op.type == TYPE_OREF) {
                    field_info *fi = cls->fields[i];

                    if (ai->d.constant->op.val.vref == NULL) {
                        Object *tmp = NULL;
                        cp_String_info *si = &ai->d.constant->entry->str;
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

    if ( ((mi = cls->init) != NULL) || ((mi = cls->init = findMethodByClass(thread, cls, "<clinit>", "()V", INVOKE_SPECIAL)) != NULL) )
        ret = invokeMethod(thread, mi, false, thread->pc, INVOKE_SPECIAL);
    else
        ret = NULL;

    pthread_rwlock_unlock(&cls->rwlock);

    return ret;
}

/* FIXME we should return throw() not NULL */
__attribute__((nonnull))
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
            getClassName(cp->fr.class),
            cp->fr.nametype->nt.name->utf8.ascii);

    if (fi && !fi->class->isInit)
        if ((ret = classInit(thread, (ClassFile *)fi->class)) != NULL)
            return ret;

    if (!fi) {
        warnx("getFieldInfo: unable to find field for %s %s",
                getClassName(cp->fr.class),
                cp->fr.nametype->nt.name->utf8.ascii);
    }

    *fld = fi;

    return NULL;
}

/* TODO: resolve the inconsistency in pc+1 vs pc, without breaking all invocations */

__attribute__((nonnull))
static inline uint16_t readShort(uint8_t *code, int64_t *pc)
{
    uint16_t ret = (uint16_t)(256U * code[(*pc)+1] + code[(*pc)+2]);
    *pc += 2;
    return ret;
}

__attribute__((nonnull))
static inline int32_t readInt(const uint8_t *code, int64_t *pc)
{
    int32_t ret = (int32_t)(16777216U * code[(*pc)] + 65536U * code[(*pc)+1] + 256U * code[(*pc)+2] + code[(*pc)+3]);
    *pc += 4;
    return ret;
}

__attribute__((nonnull))
static inline int16_t readBranch(const uint8_t *code, int64_t *pc, const int16_t /*off*/)
{
    int16_t bra = (int16_t)(256U * code[(*pc)+1] + code[(*pc)+2]);
    *pc += 2;
    return (int16_t)(bra - 3);
}

__attribute__((nonnull))
static inline int32_t readBranchWide(const uint8_t *code, int64_t *pc, const int16_t /*off*/)
{
    int32_t bra = (int32_t)(16777216U * code[(*pc)] + 65536U * code[(*pc)+1] + 256U * code[(*pc)+2] + code[(*pc)+3]);
    *pc += 2;
    return (int32_t)(bra - 5);
}

__attribute__((nonnull))
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
                            "java.lang.String", NULL));

                if(!push(cur_frame, newOperand(TYPE_OREF, o)))
                    return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                if(!push(cur_frame, newOperand(TYPE_AREF, cp->str.intern)))
                    return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

                method_info *meth;
                meth = findMethodByClass(thread, strcls, "<init>", "([C)V", INVOKE_SPECIAL);

                if (meth == NULL)
                    return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

                if((ex_opr = invokeMethod(thread, meth, true, pc, INVOKE_SPECIAL)) != NULL)
                    return ex_opr;

                if(!push(cur_frame, newOperand(TYPE_OREF, o)))
                    return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
            }
            break;
        case CONSTANT_Class:
            {
                dbg_printf("CONSTANT: Class %s\n", getClassName(cp));
                ClassFile *cls = findClass(thread, getClassName(cp), NULL);
                if (!cls || !getClassObject(thread, (ClassFile *)cls, false)) {
                    printf("CONSTANT_Class fail\n");
                    return throw(thread, "java.lang.ClassNotFoundException", ERR_AT, pc);
                }

                if (!push(cur_frame, newOperand(TYPE_OREF, getClassObject(thread, (ClassFile *)cls, false))))
                    return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
            }
            break;
        case CONSTANT_Float:
            if(!push(cur_frame, newOperand(TYPE_FLOAT, &cp->tfloat.c_float)))
                return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
            break;
        case CONSTANT_Integer:
            if(!push(cur_frame, newOperand(TYPE_INT, &cp->tint.c_int)))
                return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
            break;
        case CONSTANT_Long:
            if(!push(cur_frame, newOperand(TYPE_LONG, &cp->tlong.c_long)))
                return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
            break;
        default:
            errx(EXIT_FAILURE, "ldc: unknown constant pool tag: %d", cp->tag);
    }
    return NULL;
}

__attribute__((nonnull(1,2),hot))
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

#ifdef DEBUG_CODE
    const Frame *const cur_frame = currentFrame(thread);
    dumpFrame(cur_frame);
#endif

    dbg_cprintf("\nrunCode: (%s:%s)\n",
            getClassName(cur_frame->class->this_class),
            cur_frame->mi->name->utf8.ascii
            );

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


        dbg_cprintf("%03ld: [%02x] ", pc, opc);


        switch(opc)
        {
            case 0x00: /* nop */
                break;
            case 0x01: /* aconst_null */
                {

                    dbg_cprintf(ANSI_INSTR " aconst_null\n" ANSI_RESET);

                    push(cur_frame, newOperand(TYPE_NULL, NULL));
                }
                break;
            case 0x02: /* iconst_m1 */
                {

                    dbg_cprintf(ANSI_INSTR " iconst_m1\n" ANSI_RESET);

                    tmpOp.val.vint = -1;
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                }
                break;
            case 0x03: /* iconst 0 */
                {

                    dbg_cprintf(ANSI_INSTR " iconst_0\n" ANSI_RESET);

                    tmpOp.val.vint = 0;
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                }
                break;
            case 0x04: /* iconst 1 */
                {

                    dbg_cprintf(ANSI_INSTR" iconst_1\n" ANSI_RESET);

                    tmpOp.val.vint = 1;
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                }
                break;
            case 0x05: /* iconst 2 */
                {

                    dbg_cprintf(ANSI_INSTR" iconst_2\n" ANSI_RESET);

                    tmpOp.val.vint = 2;
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                }
                break;
            case 0x06: /* iconst 3 */
                {

                    dbg_cprintf(ANSI_INSTR" iconst_3\n" ANSI_RESET);

                    tmpOp.val.vint = 3;
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                }
                break;
            case 0x07: /* iconst 4 */
                {

                    dbg_cprintf(ANSI_INSTR" iconst_4\n" ANSI_RESET);

                    tmpOp.val.vint = 4;
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                }
                break;
            case 0x08: /* iconst 5 */
                {

                    dbg_cprintf(ANSI_INSTR" iconst_5\n" ANSI_RESET);

                    tmpOp.val.vint = 5;
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                }
                break;
            case 0x09: /* lconst 0 */
                {

                    dbg_cprintf(ANSI_INSTR " lconst_0\n" ANSI_RESET);

                    tmpOp.val.vlong = 0;
                    push(cur_frame, newOperand(TYPE_LONG, &tmpOp.val.vlong));
                }
                break;
            case 0x0a: /* lconst 1 */
                {

                    dbg_cprintf(ANSI_INSTR " lconst_1\n" ANSI_RESET);

                    tmpOp.val.vlong = 1;
                    push(cur_frame, newOperand(TYPE_LONG, &tmpOp.val.vlong));
                }
                break;
            case 0x0b: /* fconst_0 */
                {

                    dbg_cprintf(ANSI_INSTR " fconst_0 " ANSI_RESET "\n");

                    tmpOp.val.vfloat = 0.0f;
                    push(cur_frame, newOperand(TYPE_FLOAT, &tmpOp.val.vfloat));
                }
                break;
            case 0x0c: /* fconst_1 */
                {
                    dbg_cprintf(ANSI_INSTR " fconst_1 " ANSI_RESET "\n");

                    tmpOp.val.vfloat = 1.0f;
                    push(cur_frame, newOperand(TYPE_FLOAT, &tmpOp.val.vfloat));
                }
                break;
            case 0x0d: /* fconst_2 */
                {
                    dbg_cprintf(ANSI_INSTR " fconst_2 " ANSI_RESET "\n");

                    tmpOp.val.vfloat = 2.0f;
                    push(cur_frame, newOperand(TYPE_FLOAT, &tmpOp.val.vfloat));
                }
                break;
            case 0x0e: /* dconst_0 */
                {
                    dbg_cprintf(ANSI_INSTR " dconst_0 " ANSI_RESET "\n");

                    tmpOp.val.vdouble = 0.0f;
                    push(cur_frame, newOperand(TYPE_DOUBLE, &tmpOp.val.vdouble));
                }
                break;
            case 0x0f: /* dconst_1 */
                {
                    dbg_cprintf(ANSI_INSTR " dconst_1 " ANSI_RESET "\n");

                    tmpOp.val.vdouble = 1.0f;
                    push(cur_frame, newOperand(TYPE_DOUBLE, &tmpOp.val.vdouble));
                }
                break;
            case 0x10: /* bipush */
                {
                    tmpOp.val.vint = code[++pc];

                    dbg_cprintf(ANSI_INSTR " bipush " ANSI_RESET "%d\n", tmpOp.val.vint);

                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                }
                break;
            case 0x11: /* sipush */
                {
                    tmpw = readShort(code, &pc);

                    dbg_cprintf(ANSI_INSTR " sipush " ANSI_RESET " %d\n", tmpw);

                    tmpOp.val.vint = tmpw;
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                }
                break;
            case 0x12: /* ldc */
                {
                    tmp = code[++pc];

                    dbg_cprintf(ANSI_INSTR " ldc " ANSI_RESET "#%d ", tmp);
                    dbg_cprintf("\n");

                    Operand *ex_opr = ldc(tmp, cur_frame, cls, thread, pc);
                    if (ex_opr) return ex_opr;


                }
                break;
            case 0x13: /* ldc_w */
                {
                    tmpw = readShort(code, &pc);

                    dbg_cprintf(ANSI_INSTR "ldc_w " ANSI_RESET "%d\n", tmpw);

                    Operand *ex_opr = ldc(tmpw, cur_frame, cls, thread, pc);
                    if (ex_opr) return ex_opr;

                    dbg_cprintf("\n");

                }
                break;
            case 0x14: /* ldc2_w */
                {
                    tmpw = readShort(code, &pc);

                    dbg_cprintf(ANSI_INSTR " ldc2_w " ANSI_RESET "%d\n", tmpw);

                    cp_info *cp = cls->constant_pool[tmpw];

                    switch(cp->tag)
                    {
                        case CONSTANT_Long:
                            if(!push(cur_frame, newOperand(TYPE_LONG, &cp->tlong.c_long)))
                                return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                            break;
                        case CONSTANT_Double:
                            if(!push(cur_frame, newOperand(TYPE_DOUBLE, &cp->tdouble.c_double)))
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

                    dbg_cprintf(ANSI_INSTR " iload " ANSI_RESET "%d [%s %s]\n", 
                            tmp,
                            printOpType(getLocal(cur_frame, tmp)->type),
                            printOpValue(getLocal(cur_frame, tmp))
                            );

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

                    dbg_cprintf(ANSI_INSTR " lload " ANSI_RESET "%d\n", tmp);

                    if(!push(cur_frame, dupOperand(getLocal(cur_frame, tmp))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x17: /* fload */
                {
                    tmp = code[++pc];

                    dbg_cprintf(ANSI_INSTR " fload " ANSI_RESET "%d\n", tmp);

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

                    dbg_cprintf(ANSI_INSTR " dload " ANSI_RESET "%d\n", tmp);

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
                    
                    dbg_cprintf(ANSI_INSTR " aload " ANSI_RESET "%d [%s %s]\n", 
                            tmp,
                            printOpType(getLocal(cur_frame, tmp)->type),
                            printOpValue(getLocal(cur_frame, tmp))
                            );

                    if(!push(cur_frame, dupOperand(getLocal(cur_frame, tmp))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x1a: /* iload_0 */
                {

                    dbg_cprintf(ANSI_INSTR " iload_0\n" ANSI_RESET);

                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,0))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x1b: /* iload_1 */
                {

                    dbg_cprintf(ANSI_INSTR" iload_1\n" ANSI_RESET);

                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,1))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x1c: /* iload_2 */
                {

                    dbg_cprintf(ANSI_INSTR" iload_2\n" ANSI_RESET);

                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,2))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x1d: /* iload_3 */
                {

                    dbg_cprintf(ANSI_INSTR" iload_3 " ANSI_RESET "%s %s\n",
                            printOpType(cur_frame->local[3]->type),
                            printOpValue(cur_frame->local[3]));

                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,3))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x1e: /* lload_0 */
                {

                    dbg_cprintf(ANSI_INSTR " lload_0 " ANSI_RESET "%s %s\n",
                            printOpType(cur_frame->local[0]->type),
                            printOpValue(cur_frame->local[0]));

                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,0))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x1f: /* lload_1 */
                {

                    dbg_cprintf(ANSI_INSTR " lload_1 " ANSI_RESET "%s %s\n",
                            printOpType(cur_frame->local[1]->type),
                            printOpValue(cur_frame->local[1]));

                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,1))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x20: /* lload_2 */
                {

                    dbg_cprintf(ANSI_INSTR " lload_2 " ANSI_RESET "%s %s\n",
                            printOpType(cur_frame->local[2]->type),
                            printOpValue(cur_frame->local[2]));

                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,2))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x21: /* lload_3 */
                {

                    dbg_cprintf(ANSI_INSTR " lload_3 " ANSI_RESET "%s %s\n",
                            printOpType(cur_frame->local[3]->type),
                            printOpValue(cur_frame->local[3]));

                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,3))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x22: /* fload_0 */
                {

                    dbg_cprintf(ANSI_INSTR " fload_0 " ANSI_RESET "%s %s\n",
                            printOpType(cur_frame->local[0]->type),
                            printOpValue(cur_frame->local[0]));

                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,0))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x23: /* fload_1 */
                {

                    dbg_cprintf(ANSI_INSTR " fload_1 " ANSI_RESET "%s %s\n",
                            printOpType(cur_frame->local[1]->type),
                            printOpValue(cur_frame->local[1]));

                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,1))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x24: /* fload_2 */
                {

                    dbg_cprintf(ANSI_INSTR " fload_2 " ANSI_RESET "%s %s\n",
                            printOpType(cur_frame->local[2]->type),
                            printOpValue(cur_frame->local[2]));

                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,2))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x25: /* fload_3 */
                {

                    dbg_cprintf(ANSI_INSTR " fload_3 " ANSI_RESET "%s %s\n",
                            printOpType(cur_frame->local[3]->type),
                            printOpValue(cur_frame->local[3]));

                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,3))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x26: /* dload_0 */
                {

                    dbg_cprintf(ANSI_INSTR " dload_0 " ANSI_RESET "%s %s\n",
                            printOpType(cur_frame->local[0]->type),
                            printOpValue(cur_frame->local[0]));

                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,0))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x27: /* dload_1 */
                {

                    dbg_cprintf(ANSI_INSTR " dload_1 " ANSI_RESET "%s %s\n",
                            printOpType(cur_frame->local[1]->type),
                            printOpValue(cur_frame->local[1]));

                    Operand *o = getLocal(cur_frame,1);
                    if(o == NULL || o->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
                    if(!push(cur_frame, dupOperand(o)))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x28: /* dload_2 */
                {

                    dbg_cprintf(ANSI_INSTR " dload_2 " ANSI_RESET "%s %s\n",
                            printOpType(cur_frame->local[2]->type),
                            printOpValue(cur_frame->local[2]));

                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,2))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x29: /* dload_3 */
                {

                    dbg_cprintf(ANSI_INSTR " dload_3 " ANSI_RESET "%s %s\n",
                            printOpType(cur_frame->local[3]->type),
                            printOpValue(cur_frame->local[3]));

                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,3))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x2a: /* aload_0 */
                {

                    dbg_cprintf(ANSI_INSTR " aload_0 " ANSI_RESET " %s %s\n",
                            printOpType(cur_frame->local[0]->type),
                            printOpValue(cur_frame->local[0]));

                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,0))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x2b: /* aload_1 */
                {

                    dbg_cprintf(ANSI_INSTR " aload_1" ANSI_RESET " %s %s\n",
                            printOpType(cur_frame->local[1]->type),
                            printOpValue(cur_frame->local[1]));

                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,1))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x2c: /* aload_2 */
                {
                    dbg_cprintf(ANSI_INSTR " aload_2" ANSI_RESET " %s %s\n",
                            printOpType(cur_frame->local[2]->type),
                            printOpValue(cur_frame->local[2]));
                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,2))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x2d: /* aload_3 */
                {
                    dbg_cprintf(ANSI_INSTR" aload_3" ANSI_RESET " %s %s\n",
                            printOpType(cur_frame->local[3]->type),
                            printOpValue(cur_frame->local[3]));
                    if(!push(cur_frame, dupOperand(getLocal(cur_frame,3))))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x2e: /* iaload */
                {
                    dbg_cprintf(ANSI_INSTR " iaload\n" ANSI_RESET);
                    Operand *index = pop(cur_frame);
                    Operand *array = pop(cur_frame);

                    if (index->type == TYPE_NULL || array->type == TYPE_NULL) {
                        freeAndNullOperand(&index);
                        freeAndNullOperand(&array);
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
                    }

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->array.len) {
                        freeAndNullOperand(&index);
                        freeAndNullOperand(&array);
                        return throw(thread, "java.lang.IndexOutOfBoundsException", NULL, pc);
                    }

                    if (array->val.vref->array.data[index->val.vint])
                        push(cur_frame, dupOperand(array->val.vref->array.data[index->val.vint]));
                    else {
                        tmpOp.val.vint = 0;
                        push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                    }

                    freeAndNullOperand(&array);
                    freeAndNullOperand(&index);
                }
                break;
            case 0x2f: /* laload */
                {

                    dbg_cprintf(ANSI_INSTR " laload\n" ANSI_RESET);

                    Operand *index = pop(cur_frame);
                    Operand *array = pop(cur_frame);

                    if (index->type == TYPE_NULL || array->type == TYPE_NULL) {
                        freeOperand(array);
                        freeOperand(index);
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
                    }

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->array.len) {
                        freeOperand(array);
                        freeOperand(index);
                        return throw(thread, "java.lang.IndexOutOfBoundsException", NULL, pc);
                    }

                    if (array->val.vref->array.data[index->val.vint])
                        push(cur_frame, dupOperand(array->val.vref->array.data[index->val.vint]));
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

                    dbg_cprintf(ANSI_INSTR " faload\n" ANSI_RESET);

                    Operand *index = pop(cur_frame);
                    Operand *array = pop(cur_frame);

                    if (index->type == TYPE_NULL || array->type == TYPE_NULL) {
                        freeOperand(array);
                        freeOperand(index);
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
                    }

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->array.len) {
                        freeOperand(array);
                        freeOperand(index);
                        return throw(thread, "java.lang.IndexOutOfBoundsException", NULL, pc);
                    }

                    if (array->val.vref->array.data[index->val.vint])
                        push(cur_frame, dupOperand(array->val.vref->array.data[index->val.vint]));
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

                    dbg_cprintf(ANSI_INSTR " daload\n" ANSI_RESET);

                    Operand *index = pop(cur_frame);
                    Operand *array = pop(cur_frame);

                    if (index->type == TYPE_NULL || array->type == TYPE_NULL) {
                        freeOperand(array);
                        freeOperand(index);
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
                    }

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->array.len) {
                        freeOperand(array);
                        freeOperand(index);
                        return throw(thread, "java.lang.IndexOutOfBoundsException", NULL, pc);
                    }

                    if (array->val.vref->array.data[index->val.vint])
                        push(cur_frame, dupOperand(array->val.vref->array.data[index->val.vint]));
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

                    dbg_cprintf(ANSI_INSTR " aaload\n" ANSI_RESET);

                    Operand *index = pop(cur_frame);
                    Operand *array = pop(cur_frame);

                    if (index->type == TYPE_NULL || array->type == TYPE_NULL) {
                        freeOperand(array);
                        freeOperand(index);
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
                    }

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->array.len) {
                        abort();
                        freeOperand(array);
                        freeOperand(index);
                        return throw(thread, "java.lang.IndexOutOfBoundsException", NULL, pc);
                    }

                    if (array->val.vref->array.data[index->val.vint])
                        push(cur_frame, dupOperand(array->val.vref->array.data[index->val.vint]));
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


                    dbg_cprintf(ANSI_INSTR " baload\n" ANSI_RESET);

                    if (index->type == TYPE_NULL || array->type == TYPE_NULL) {
                        freeOperand(array);
                        freeOperand(index);
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
                    }

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->array.len) {
                        freeOperand(array);
                        freeOperand(index);
                        return throw(thread, "java.lang.IndexOutOfBoundsException", NULL, pc);
                    }

                    if (array->val.vref->array.data[index->val.vint]) {
                        if (array->val.vref->array.type == TYPE_BOOL)
                            tmpOp.val.vint = array->val.vref->array.data[index->val.vint]->val.vbool;
                        else
                            tmpOp.val.vint = array->val.vref->array.data[index->val.vint]->val.vbyte;

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

                    dbg_cprintf(ANSI_INSTR " caload" ANSI_RESET " [%d] %s\n", index->val.vint,
                            printOpType(array->val.vref->array.type));

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->array.len) {
                        freeOperand(index);
                        freeOperand(array);
                        return throw(thread, "java.lang.IndexOutOfBoundsException", ERR_AT, pc);
                    }

                    if (array->val.vref->array.data[index->val.vint]) {
                        tmpOp.val.vint = array->val.vref->array.data[index->val.vint]->val.vchar;
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

                    dbg_cprintf(ANSI_INSTR " saload" ANSI_RESET " [%d] %s\n", index->val.vint,
                            printOpType(array->val.vref->array.type));

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->array.len) {
                        freeOperand(index);
                        freeOperand(array);
                        return throw(thread, "java.lang.IndexOutOfBoundsException", ERR_AT, pc);
                    }

                    if (array->val.vref->array.data[index->val.vint]) {
                        tmpOp.val.vint = array->val.vref->array.data[index->val.vint]->val.vshort;
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

                    dbg_cprintf(ANSI_INSTR " istore "ANSI_RESET" [%s %s] => %d\n",
                            printOpType(val->type),
                            printOpValue(val),
                            tmp);

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

                    dbg_cprintf(ANSI_INSTR " lstore "ANSI_RESET" %s.%s => %d\n",
                            printOpType(val->type),
                            printOpValue(val),
                            tmp);

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

                    dbg_cprintf(ANSI_INSTR " fstore "ANSI_RESET"%s.%s => %d\n",
                            printOpType(val->type),
                            printOpValue(val),
                            tmp);

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

                    dbg_cprintf(ANSI_INSTR " dstore "ANSI_RESET"%s.%s => %d\n",
                            printOpType(val->type),
                            printOpValue(val),
                            tmp);

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

                    dbg_cprintf(ANSI_INSTR " astore "ANSI_RESET"%s.%s => %d\n",
                            printOpType(val->type),
                            printOpValue(val),
                            tmp);

                    if(setLocal(cur_frame, tmp, val) == NULL) {
                        freeOperand(val);
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    }
                }
                break;
            case 0x3b: /* istore_0 */
                {
                    Operand *a = pop(cur_frame);

                    dbg_cprintf(ANSI_INSTR" istore_0 "  ANSI_RESET);
                    dbg_cprintf("<= %s %s\n", printOpType(a->type), printOpValue(a));

                    if(setLocal(cur_frame,0,a) == NULL) {
                        freeOperand(a);
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    }
                }
                break;
            case 0x3c: /* istore_1*/
                {
                    Operand *a = pop(cur_frame);

                    dbg_cprintf(ANSI_INSTR " istore_1 "  ANSI_RESET);
                    dbg_cprintf("<= %s %s\n", printOpType(a->type), printOpValue(a));

                    if(setLocal(cur_frame,1,a) == NULL) {
                        freeOperand(a);
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    }
                }
                break;
            case 0x3d: /* istore_2*/
                {
                    Operand *a = pop(cur_frame);

                    dbg_cprintf(ANSI_INSTR" istore_2 " ANSI_RESET);
                    dbg_cprintf("<= %s %s\n", printOpType(a->type), printOpValue(a));

                    if(setLocal(cur_frame,2,a) == NULL) {
                        freeOperand(a);
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    }
                }
                break;
            case 0x3e: /* istore_3*/
                {
                    Operand *a = pop(cur_frame);

                    dbg_cprintf(ANSI_INSTR" istore_3 " ANSI_RESET);
                    dbg_cprintf("<= %s %s\n", printOpType(a->type), printOpValue(a));

                    if(setLocal(cur_frame,3,a) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;

            case 0x3f: /* lstore_0*/
                {
                    Operand *a = pop(cur_frame);

                    dbg_cprintf(ANSI_INSTR" lstore_0 " ANSI_RESET);
                    dbg_cprintf("<= %s %s\n", printOpType(a->type), printOpValue(a));

                    if(setLocal(cur_frame,0,a) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x40: /* lstore_1*/
                {
                    Operand *a = pop(cur_frame);

                    dbg_cprintf(ANSI_INSTR" lstore_1 " ANSI_RESET);
                    dbg_cprintf("<= %s %s\n", printOpType(a->type), printOpValue(a));

                    if(setLocal(cur_frame,1,a) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x41: /* lstore_2*/
                {
                    Operand *a = pop(cur_frame);

                    dbg_cprintf(ANSI_INSTR" lstore_2 " ANSI_RESET);
                    dbg_cprintf("<= %s %s\n", printOpType(a->type), printOpValue(a));

                    if(setLocal(cur_frame,2,a) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x42: /* lstore_3*/
                {
                    Operand *a = pop(cur_frame);

                    dbg_cprintf(ANSI_INSTR" lstore_3 " ANSI_RESET);
                    dbg_cprintf("<= %s %s\n", printOpType(a->type), printOpValue(a));

                    if(setLocal(cur_frame,3,a) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;

            case 0x43: /* fstore_0*/
                {
                    Operand *a = pop(cur_frame);

                    dbg_cprintf(ANSI_INSTR" fstore_0 " ANSI_RESET);
                    dbg_cprintf("<= %s %s\n", printOpType(a->type), printOpValue(a));

                    if(setLocal(cur_frame,0,a) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x44: /* fstore_1*/
                {
                    Operand *a = pop(cur_frame);

                    dbg_cprintf(ANSI_INSTR" fstore_1 " ANSI_RESET);
                    dbg_cprintf("<= %s %s\n", printOpType(a->type), printOpValue(a));

                    if(setLocal(cur_frame,1,a) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x45: /* fstore_2*/
                {
                    Operand *a = pop(cur_frame);

                    dbg_cprintf(ANSI_INSTR" fstore_2 " ANSI_RESET);
                    dbg_cprintf("<= %s %s\n", printOpType(a->type), printOpValue(a));

                    if(setLocal(cur_frame,2,a) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x46: /* fstore_3*/
                {
                    Operand *a = pop(cur_frame);

                    dbg_cprintf(ANSI_INSTR" fstore_3 " ANSI_RESET);
                    dbg_cprintf("<= %s %s\n", printOpType(a->type), printOpValue(a));

                    if(setLocal(cur_frame,3,a) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;

            case 0x47: /* dstore_0 */
                {

                    dbg_cprintf(ANSI_INSTR "dstore_0\n" ANSI_RESET);

                    if(setLocal(cur_frame,0,pop(cur_frame)) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x48: /* dstore_1 */
                {

                    dbg_cprintf(ANSI_INSTR "dstore_1\n" ANSI_RESET);

                    if(setLocal(cur_frame,1,pop(cur_frame)) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;

            case 0x49: /* dstore_2 */
                {

                    dbg_cprintf(ANSI_INSTR "dstore_2\n" ANSI_RESET);

                    if(setLocal(cur_frame,2,pop(cur_frame)) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;

            case 0x4a: /* dstore_3 */
                {

                    dbg_cprintf(ANSI_INSTR "dstore_3\n" ANSI_RESET);

                    if(setLocal(cur_frame,3,pop(cur_frame)) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x4b: /* astore_0 */
                {

                    dbg_cprintf(ANSI_INSTR" astore_0\n" ANSI_RESET);

                    if(setLocal(cur_frame,0,pop(cur_frame)) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x4c: /* astore_1 */
                {

                    dbg_cprintf(ANSI_INSTR" astore_1\n" ANSI_RESET);

                    if(setLocal(cur_frame,1,pop(cur_frame)) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x4d: /* astore_2 */
                {

                    dbg_cprintf(ANSI_INSTR" astore_2\n" ANSI_RESET);

                    if(setLocal(cur_frame,2,pop(cur_frame)) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x4e: /* astore_3 */
                {

                    dbg_cprintf(ANSI_INSTR" astore_3\n" ANSI_RESET);

                    if(setLocal(cur_frame,3,pop(cur_frame)) == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0x4f: /* iastore */
                {

                    dbg_cprintf(ANSI_INSTR" iastore\n" ANSI_RESET);

                    Operand *value = pop(cur_frame);
                    Operand *index = pop(cur_frame);
                    Operand *array = pop(cur_frame);


                    if (index->type == TYPE_NULL || array->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->array.len)
                        return throw(thread, "java.lang.IndexOutOfBoundsException", ERR_AT, pc);

                    Operand *cur = array->val.vref->array.data[index->val.vint];
                    if (cur)
                        freeOperand(cur);

                    array->val.vref->array.data[index->val.vint] = dupOperand(value);

                    freeOperand(value);
                    freeOperand(index);
                    freeOperand(array);
                }
                break;

            case 0x50: /* lastore */
                {

                    dbg_cprintf(ANSI_INSTR" lastore\n" ANSI_RESET);

                    Operand *value = pop(cur_frame);
                    Operand *index = pop(cur_frame);
                    Operand *array = pop(cur_frame);

                    if (index->type == TYPE_NULL || array->type == TYPE_NULL) {
                        freeOperand(index); freeOperand(array);
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
                    }

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->array.len) {
                        freeOperand(index); freeOperand(array);
                        return throw(thread, "java.lang.IndexOutOfBoundsException", ERR_AT, pc);
                    }


                    if (value->type != array->type) {
                        freeOperand(index); freeOperand(array); freeOperand(value);
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    }

                    Operand *cur = array->val.vref->array.data[index->val.vint];
                    if (cur)
                        freeOperand(cur);

                    array->val.vref->array.data[index->val.vint] = dupOperand(value);

                    freeOperand(value);
                    freeOperand(index);
                    freeOperand(array);
                }
                break;

            case 0x51: /* fastore */
                {

                    dbg_cprintf(ANSI_INSTR" fastore\n" ANSI_RESET);

                    Operand *value = pop(cur_frame);
                    Operand *index = pop(cur_frame);
                    Operand *array = pop(cur_frame);

                    if (index->type == TYPE_NULL || array->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->array.len)
                        return throw(thread, "java.lang.IndexOutOfBoundsException", ERR_AT, pc);

                    Operand *cur = array->val.vref->array.data[index->val.vint];
                    if (cur)
                        freeOperand(cur);

                    array->val.vref->array.data[index->val.vint] = dupOperand(value);

                    freeOperand(array);
                    freeOperand(index);
                    freeOperand(value);
                }
                break;

            case 0x52: /* dastore */
                {

                    dbg_cprintf(ANSI_INSTR" dastore\n" ANSI_RESET);

                    Operand *value = pop(cur_frame);
                    Operand *index = pop(cur_frame);
                    Operand *array = pop(cur_frame);

                    if (index->type == TYPE_NULL || array->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->array.len)
                        return throw(thread, "java.lang.IndexOutOfBoundsException", ERR_AT, pc);

                    Operand *cur = array->val.vref->array.data[index->val.vint];
                    if (cur)
                        freeOperand(cur);

                    array->val.vref->array.data[index->val.vint] = dupOperand(value);

                    freeOperand(value);
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

                    dbg_cprintf(ANSI_INSTR " aastore " ANSI_RESET "[%d]\n",
                            index->val.vint);

                    if (value->type != TYPE_NULL && array->val.vref->array.type != value->type) {
                        warnx("aastore: %d != %d", value->type, array->val.vref->array.type);
                        warnx("aastore: invalid type");
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    }

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->array.len)
                        return throw(thread, "java.lang.IndexOutOfBoundsException", ERR_AT, pc);

                    if (array->val.vref->array.data[index->val.vint])
                        freeOperand(array->val.vref->array.data[index->val.vint]);
                    array->val.vref->array.data[index->val.vint] = dupOperand(value);

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

                    dbg_cprintf(ANSI_INSTR " bastore " ANSI_RESET "%s %s => [%d][%s]\n",
                            printOpType(value->type),
                            printOpValue(value),
                            index->val.vint,
                            printOpType(array->val.vref->array.type));

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->array.len)
                        return throw(thread, "java.lang.IndexOutOfBoundsException", ERR_AT, pc);

                    if (value->type != TYPE_INT || (array->val.vref->array.type != TYPE_BYTE
                                && array->val.vref->array.type != TYPE_BOOL) )
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

                    tmpOp.val.vbyte = (int8_t)value->val.vint;

                    if (array->val.vref->array.data[index->val.vint])
                        freeOperand(array->val.vref->array.data[index->val.vint]);
                    array->val.vref->array.data[index->val.vint] =
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

                    dbg_cprintf(ANSI_INSTR " castore " ANSI_RESET "%s %s => [%d][%s]\n",
                            printOpType(value->type),
                            printOpValue(value),
                            index->val.vint,
                            printOpType(array->val.vref->array.type));

                    if (value->type != TYPE_INT || array->val.vref->array.type != TYPE_CHAR)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->array.len)
                        return throw(thread, "java.lang.IndexOutOfBoundsException", ERR_AT, pc);

                    tmpOp.val.vchar = (uint16_t)value->val.vint;

                    if (array->val.vref->array.data[index->val.vint])
                        freeOperand(array->val.vref->array.data[index->val.vint]);
                    array->val.vref->array.data[index->val.vint] = newOperand(TYPE_CHAR, &tmpOp.val.vchar);

                    freeOperand(array);
                    freeOperand(index);
                    freeOperand(value);
                }
                break;
            case 0x56: /* sastore */
                {

                    dbg_cprintf(ANSI_INSTR" sastore\n" ANSI_RESET);

                    Operand *value = pop(cur_frame);
                    Operand *index = pop(cur_frame);
                    Operand *array = pop(cur_frame);

                    if (index->type == TYPE_NULL || array->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);

                    if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->array.len)
                        return throw(thread, "java.lang.IndexOutOfBoundsException", ERR_AT, pc);

                    Operand *cur = array->val.vref->array.data[index->val.vint];
                    if (cur)
                        freeOperand(cur);

                    array->val.vref->array.data[index->val.vint] = dupOperand(value);

                    freeOperand(value);
                    freeOperand(index);
                    freeOperand(array);
                }
                break;

            case 0x57: /* pop */
                {

                    dbg_cprintf(ANSI_INSTR " pop\n" ANSI_RESET);

                    Operand *op = pop(cur_frame);
                    freeOperand(op);
                }
                break;

            case 0x58: /* pop2 */
                {

                    dbg_cprintf(ANSI_INSTR " pop\n" ANSI_RESET);

                    Operand *op;

                    for(int i = 0; i < 2; i++) {
                        op = pop(cur_frame);
                        freeOperand(op);
                    }
                }
                break;

            case 0x59: /* dup */
                {

                    dbg_cprintf(ANSI_INSTR" dup\n" ANSI_RESET);

                    Operand *cur = cur_frame->stack[cur_frame->sp-1];
                    Operand *new = dupOperand(cur);
                    push(cur_frame, new);
                }
                break;

            case 0x5a: /* dup_x1 */
                {

                    dbg_cprintf(ANSI_INSTR " dup_x1\n" ANSI_RESET);

                    Operand *val1 = pop(cur_frame);
                    Operand *val2 = pop(cur_frame);
                    push(cur_frame, dupOperand(val1));
                    push(cur_frame, val2);
                    push(cur_frame, val1);
                }
                break;

            case 0x5c: /* dup2 */
                {

                    dbg_cprintf(ANSI_INSTR " dup2\n" ANSI_RESET);

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

                    dbg_cprintf(ANSI_INSTR " swap\n" ANSI_RESET);


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

                    dbg_cprintf(ANSI_INSTR " iadd " ANSI_RESET "%d+%d=%d\n",
                            one->val.vint,
                            two->val.vint,
                            res->val.vint);

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

                    dbg_cprintf(ANSI_INSTR " ladd " ANSI_RESET "%ld+%ld=%ld\n",
                            one->val.vlong,
                            two->val.vlong,
                            res->val.vlong);

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

                    dbg_cprintf(ANSI_INSTR " fadd " ANSI_RESET "%f+%f=%f\n",
                            one->val.vfloat,
                            two->val.vfloat,
                            res->val.vfloat);

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

                    dbg_cprintf(ANSI_INSTR " dadd " ANSI_RESET "%f+%f=%f\n",
                            one->val.vdouble,
                            two->val.vdouble,
                            res->val.vdouble);

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

                    dbg_cprintf(ANSI_INSTR " isub " ANSI_RESET "%d-%d=%d\n",
                            two->val.vint,
                            one->val.vint,
                            res->val.vint);

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

                    dbg_cprintf(ANSI_INSTR " lsub " ANSI_RESET "%ld-%ld=%ld\n",
                            two->val.vlong,
                            one->val.vlong,
                            res->val.vlong);

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

                    dbg_cprintf(ANSI_INSTR " fsub " ANSI_RESET "%f-%f=%f\n",
                            two->val.vfloat,
                            one->val.vfloat,
                            res->val.vfloat);

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

                    dbg_cprintf(ANSI_INSTR " dsub " ANSI_RESET "%f-%f=%f\n",
                            two->val.vdouble,
                            one->val.vdouble,
                            res->val.vdouble);

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

                    dbg_cprintf(ANSI_INSTR " imul " ANSI_RESET "%d*%d=%d\n",
                            one->val.vint,
                            two->val.vint,
                            res->val.vint);

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

                    dbg_cprintf(ANSI_INSTR " lmul " ANSI_RESET "%ld*%ld=%ld\n",
                            one->val.vlong,
                            two->val.vlong,
                            res->val.vlong);

                    push(cur_frame, res);
                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0x6a: /* fmul */
                {
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);

                    dbg_cprintf(ANSI_INSTR " fmul" ANSI_RESET "\n");

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

                    dbg_cprintf(ANSI_INSTR " dmul " ANSI_RESET "%f*%f=%f\n",
                            one->val.vdouble,
                            two->val.vdouble,
                            res->val.vdouble);

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

                    dbg_cprintf(ANSI_INSTR " idiv" ANSI_RESET " %d/%d=%d\n",
                            one->val.vint, two->val.vint,
                            tmpOp.val.vint);

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

                    dbg_cprintf(ANSI_INSTR " ldiv" ANSI_RESET " %ld/%ld=%ld\n",
                            one->val.vlong, two->val.vlong,
                            tmpOp.val.vlong);

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

                    dbg_cprintf(ANSI_INSTR " fdiv" ANSI_RESET " %f/%f=%f\n",
                            one->val.vfloat, two->val.vfloat,
                            tmpOp.val.vfloat);

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

                    dbg_cprintf(ANSI_INSTR " ddiv" ANSI_RESET " %f/%f=%f\n",
                            one->val.vdouble, two->val.vdouble,
                            tmpOp.val.vdouble);

                    push(cur_frame, newOperand(TYPE_LONG, &tmpOp.val.vdouble));
                    freeOperand(one);
                    freeOperand(two);
                }
                break;
            case 0x70: /* irem */
                {

                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);

                    int32_t value1 = one->val.vint;
                    int32_t value2 = two->val.vint;

                    dbg_cprintf(ANSI_INSTR " irem " ANSI_RESET " %d, %d\n", value1, value2);

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

                    dbg_cprintf(ANSI_INSTR " lrem\n" ANSI_RESET);

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

                    dbg_cprintf(ANSI_INSTR " frem\n" ANSI_RESET);

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

                    dbg_cprintf(ANSI_INSTR " drem\n" ANSI_RESET);

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

                    dbg_cprintf(ANSI_INSTR "ineg\n" ANSI_RESET);

                    Operand *one = pop(cur_frame);
                    tmpOp.val.vint = -one->val.vint;
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                    freeOperand(one);
                }
                break;
            case 0x75: /* lneg */
                {

                    dbg_cprintf(ANSI_INSTR "lneg\n" ANSI_RESET);

                    Operand *one = pop(cur_frame);
                    tmpOp.val.vlong = -one->val.vlong;
                    push(cur_frame, newOperand(TYPE_LONG, &tmpOp.val.vlong));
                    freeOperand(one);
                }
                break;
            case 0x76: /* fneg */
                {

                    dbg_cprintf(ANSI_INSTR "fneg\n" ANSI_RESET);

                    Operand *one = pop(cur_frame);
                    tmpOp.val.vfloat = -one->val.vfloat;
                    push(cur_frame, newOperand(TYPE_FLOAT, &tmpOp.val.vfloat));
                    freeOperand(one);
                }
                break;
            case 0x77: /* dneg */
                {

                    dbg_cprintf(ANSI_INSTR "dneg\n" ANSI_RESET);

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

                    dbg_cprintf(ANSI_INSTR " ishl\n" ANSI_RESET);

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

                    dbg_cprintf(ANSI_INSTR " lshl\n" ANSI_RESET);

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

                    dbg_cprintf(ANSI_INSTR " ishr\n" ANSI_RESET);

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

                    dbg_cprintf(ANSI_INSTR " lshr\n" ANSI_RESET);

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

                    dbg_cprintf(ANSI_INSTR " iushr\n" ANSI_RESET);

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

                    dbg_cprintf(ANSI_INSTR " lushr\n" ANSI_RESET);

                    tmpOp.val.vlong = ((uint64_t)one->val.vlong) >> (two->val.vint & 0x3F);
                    freeOperand(one);
                    freeOperand(two);

                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));

                }
                break;


            case 0x7e: /* iand */
                {

                    dbg_cprintf(ANSI_INSTR " iand\n" ANSI_RESET);

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

                    dbg_cprintf(ANSI_INSTR " land\n" ANSI_RESET);

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

                    dbg_cprintf(ANSI_INSTR " ior\n" ANSI_RESET);

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

                    dbg_cprintf(ANSI_INSTR " lor\n" ANSI_RESET);

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

                    dbg_cprintf(ANSI_INSTR " ixor\n" ANSI_RESET);

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

                    dbg_cprintf(ANSI_INSTR " lxor\n" ANSI_RESET);

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

                    dbg_cprintf(ANSI_INSTR " iinc " ANSI_RESET "#%d,%d\n", tmp, tmp2);

                    cur_frame->local[tmp]->val.vint += tmp2;
                }
                break;
            case 0x85: /* i2l */
                {

                    dbg_cprintf(ANSI_INSTR " i2l\n" ANSI_RESET);

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

                    dbg_cprintf(ANSI_INSTR " l2i " ANSI_RESET "\n");

                    Operand *value = pop(cur_frame);
                    tmpOp.val.vint = (int32_t)value->val.vlong;
                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                    freeOperand(value);
                }
                break;
            case 0x89: /* l2f */
                {

                    dbg_cprintf(ANSI_INSTR " l2f " ANSI_RESET "\n");

                    Operand *value = pop(cur_frame);
                    tmpOp.val.vfloat = (float)value->val.vlong;
                    push(cur_frame, newOperand(TYPE_FLOAT, &tmpOp.val.vdouble));
                    freeOperand(value);
                }
                break;
            case 0x8a: /* l2d */
                {

                    dbg_cprintf(ANSI_INSTR " l2d " ANSI_RESET "\n");

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

                    dbg_cprintf(ANSI_INSTR " i2b " ANSI_RESET "%d -> %d\n", val->val.vint,
                            tmpOp.val.vint);

                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                    freeOperand(val);
                }
                break;
            case 0x92: /* i2c */
                {
                    Operand *val = pop(cur_frame);
                    tmpOp.val.vint = (uint16_t)(val->val.vint);

                    dbg_cprintf(ANSI_INSTR " i2c " ANSI_RESET "%d -> %u\n", val->val.vint,
                            tmpOp.val.vint);

                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                    freeOperand(val);
                }
                break;
            case 0x93: /* i2s */
                {
                    Operand *val = pop(cur_frame);
                    tmpOp.val.vint = (int16_t)(val->val.vint);

                    dbg_cprintf(ANSI_INSTR " i2s " ANSI_RESET "%d -> %d\n", val->val.vint,
                            tmpOp.val.vint);

                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
                    freeOperand(val);
                }
                break;
            case 0x94: /* lcmp */
                {
                    Operand *two = pop(cur_frame);
                    Operand *one = pop(cur_frame);

                    dbg_cprintf(ANSI_INSTR " lcmp " ANSI_RESET "%ld ? %ld\n",
                            one->val.vlong, two->val.vlong);

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

                    dbg_cprintf(ANSI_INSTR " fcmp%c " ANSI_RESET "%lf ? %lf\n",
                            opc == 0x95 ? 'l' : 'g',
                            one->val.vfloat, two->val.vfloat);

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

                    dbg_cprintf(ANSI_INSTR " dcmp%c " ANSI_RESET "%lg ? %lg\n",
                            opc == 0x97 ? 'l' : 'g',
                            one->val.vdouble, two->val.vdouble);

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



                    dbg_cprintf(ANSI_INSTR " ifeq" ANSI_RESET " %s == 0 ? goto %lu\n",
                            printOpValue(o),
                            pc + bra);

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


                    dbg_cprintf(ANSI_INSTR " ifne" ANSI_RESET " %s != 0 ? goto %lu\n",
                            printOpValue(o),
                            pc + bra);

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

                    dbg_cprintf(ANSI_INSTR " iflt" ANSI_RESET " %s < 0 ? goto %lu\n",
                            printOpValue(o),
                            pc + bra);


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

                    dbg_cprintf(ANSI_INSTR " ifge" ANSI_RESET " %s >= 0 ? goto %lu\n",
                            printOpValue(o),
                            pc + bra);


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

                    dbg_cprintf(ANSI_INSTR " ifgt" ANSI_RESET " %s > 0 ? goto %lu\n",
                            printOpValue(o),
                            pc + bra);


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

                    dbg_cprintf(ANSI_INSTR " ifle" ANSI_RESET " %s <= 0 ? goto %lu\n",
                            printOpValue(o),
                            pc + bra);


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

                    dbg_cprintf(ANSI_INSTR " if_icmpne" ANSI_RESET " %d != %d ? goto %ld\n",
                            one->val.vint, two->val.vint,
                            pc + bra);

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

                    dbg_cprintf(ANSI_INSTR " if_icmpne" ANSI_RESET " %d != %d ? goto %ld\n",
                            one->val.vint, two->val.vint,
                            pc + bra);

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

                    dbg_cprintf(ANSI_INSTR " if_icmplt" ANSI_RESET " %d < %d ? goto %ld\n",
                            one->val.vint, two->val.vint,
                            pc + bra);

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

                    dbg_cprintf(ANSI_INSTR " if_icmpge" ANSI_RESET " %d >= %d ? goto %ld\n",
                            one->val.vint, two->val.vint,
                            pc + bra);

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

                    dbg_cprintf(ANSI_INSTR " if_icmpgt" ANSI_RESET " %d > %d ? goto %ld\n",
                            one->val.vint, two->val.vint,
                            pc + bra);

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

                    dbg_cprintf(ANSI_INSTR " if_icmple" ANSI_RESET " %d <= %d ? goto %lu\n",
                            one->val.vint, two->val.vint,
                            pc + bra);

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

                    dbg_cprintf(ANSI_INSTR " if_acmpeq" ANSI_RESET " ref == ref ? goto %lu\n",
                            pc + bra);

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

                    dbg_cprintf(ANSI_INSTR " if_acmpne" ANSI_RESET " ref != ref ? goto %lu\n",
                            pc + bra);

                    if (one->type == two->type && one->val.vref != two->val.vref)
                        pc += bra;

                    freeOperand(one);
                    freeOperand(two);
                }
                break;

            case 0xa7: /* goto */
                {
                    int16_t bra = readBranch(code, &pc, 3);

                    dbg_cprintf(ANSI_INSTR " goto " ANSI_RESET " %d:%lu\n", bra, pc + bra);

                    pc += bra;
                }
                break;

            case 0xa9: /* ret */
                {
                    tmp = code[++pc];
                    Operand *dest = getLocal(cur_frame, tmp);
                    if(dest->type != TYPE_RETADDR)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

                    dbg_cprintf(ANSI_INSTR " ret " ANSI_RESET "%d\n", dest->val.vint);

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

                    dbg_cprintf(ANSI_INSTR " tableswitch " ANSI_RESET "%d #%d->#%d on %d\n", def, low, high, v->val.vint);

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

                    dbg_cprintf(ANSI_INSTR " lookupswitch " ANSI_RESET "%d #%d == %d\n", def, npairs, v->val.vint);

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

                    dbg_cprintf(ANSI_INSTR " ireturn\n" ANSI_RESET);

                    Operand *o = pop(cur_frame);
                    if (o == NULL) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    cur_frame->ret = o;
                    running = false;
                }
                break;
            case 0xad: /* lreturn */
                {

                    dbg_cprintf(ANSI_INSTR " lreturn\n" ANSI_RESET);

                    Operand *o = pop(cur_frame);
                    if (o == NULL) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    cur_frame->ret = o;
                    running = false;
                }
                break;
            case 0xae: /* freturn */
                {

                    dbg_cprintf(ANSI_INSTR "freturn\n" ANSI_RESET);

                    Operand *o = pop(cur_frame);
                    if (o == NULL) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    cur_frame->ret = o;
                    running = false;
                }
                break;
            case 0xaf: /* dreturn */
                {

                    dbg_cprintf(ANSI_INSTR "dreturn\n" ANSI_RESET);

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

                    dbg_cprintf(ANSI_INSTR " areturn " ANSI_RESET);
                    dbg_cprintf("%s %s\n", printOpType(o->type), printOpValue(o));

                    cur_frame->ret = o;
                    running = false;
                }
                break;
            case 0xb1: /* return */
                {

                    dbg_cprintf(ANSI_INSTR " return\n" ANSI_RESET);

                    running = false;
                }
                break;
            case 0xb2: /* getstatic */
                {
                    tmpw = readShort(code, &pc);

                    dbg_cprintf(ANSI_INSTR " getstatic" ANSI_RESET " #%d ", tmpw);

                    field_info *f = NULL;
                    if ((ex_opr = getFieldInfo(thread, cls, tmpw, &f)) != NULL)
                        return ex_opr;
                    if (f == NULL)
                        errx(EXIT_FAILURE, "getstatic: can't find field");

                    dbg_cprintf("got field_info '%s' '%s' ",
                            f->name->utf8.ascii,
                            f->descriptor->utf8.ascii);

                    if (f->static_operand) {

                        dbg_cprintf("contents %s %s\n",
                                printOpType(f->static_operand->type),
                                printOpValue(f->static_operand));

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

                    dbg_cprintf(ANSI_INSTR " putstatic" ANSI_RESET " #%d ", tmpw);

                    field_info *f = NULL;

                    if ((ex_opr = getFieldInfo(thread, cls, tmpw, &f)) != NULL)
                        return ex_opr;

                    if (f == NULL)
                        errx(EXIT_FAILURE, "putstatic: no field");

                    dbg_cprintf("got field_info '%s' '%s'\n",
                            f->name->utf8.ascii,
                            f->descriptor->utf8.ascii);

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

                    dbg_cprintf(ANSI_INSTR " getfield" ANSI_RESET " #%d ", tmpw);

                    if (fi == NULL) {
                        errx(EXIT_FAILURE, "getfield: no field\n");
                        break;
                    }

                    Operand *o = pop(cur_frame);
                    if (o->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);


                    dbg_cprintf("to: field_info '%s' '%s'\nto: %s %s\n",
                            fi->name->utf8.ascii,
                            fi->descriptor->utf8.ascii,
                            printOpType(o->type),
                            printOpValue(o)
                          );


                    Object *obj = o->val.vref;

                    Field *fld = NULL;
                    for (int i = 0; i < obj->obj.num_fields; i++)
                        if(fi == obj->obj.fields[i].field_info)
                            fld = &obj->obj.fields[i];

                    if (fld == NULL) {
                        warnx("getfield: Can't find field %s %s in %s",
                                fi->name->utf8.ascii,
                                fi->descriptor->utf8.ascii,
                                getClassName(obj->class->this_class));
                        //abort();
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    }

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

                    dbg_cprintf(ANSI_INSTR " putfield " ANSI_RESET " #%d ", tmpw);

                    if (fi == NULL) {
                        errx(EXIT_FAILURE, "putfield: no field");
                        break;
                    }
                    Operand *value = pop(cur_frame);
                    if (value == NULL) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

                    dbg_cprintf(" from: %s %s", printOpType(value->type), printOpValue(value));
                    dbg_cprintf(" to  : %s %s\n",
                            fi->name->utf8.ascii,
                            fi->descriptor->utf8.ascii);

                    Operand *o = pop(cur_frame);
                    if (o->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
                    Object *obj = o->val.vref;

                    Field *fld = NULL;

                    if (o->type != TYPE_OREF) {
                        warnx("putfield: not OREF");
                        __asm__("int $3");
                    }

                    for (int i = 0; i < obj->obj.num_fields; i++) {
                        if(fi == obj->obj.fields[i].field_info)
                            fld = &obj->obj.fields[i];
                    }

                    if (fld == NULL) {
                        warnx("putfield: unknown field: %s %s",
                                fi->name->utf8.ascii, fi->descriptor->utf8.ascii);
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
                    cp_Fieldref_info *mi = &cls->constant_pool[tmpw]->fr;

                    dbg_printf(ANSI_INSTR " invokevirtual" ANSI_RESET " #%d ", tmpw);
                    dbg_printf("method_info %s %s %s\n",
                            getClassName(mi->class),
                            mi->nametype->nt.name->utf8.ascii,
                            mi->nametype->nt.descriptor->utf8.ascii);

                    /* used for num meths only */
                    method_info *meth;

                    meth = findClassMethod(thread,
                            getClassName(mi->class),
                            mi->nametype->nt.name->utf8.ascii,
                            mi->nametype->nt.descriptor->utf8.ascii,
                            INVOKE_VIRTUAL);

                    if (meth == NULL) {
                        printf("invokevirtual: #%d: NULL method: %s.%s on %s\n",
                                tmpw,
                                mi->nametype->nt.name->utf8.ascii,
                                mi->nametype->nt.descriptor->utf8.ascii,
                                getClassName(mi->class));
#ifdef DEBUG
                        dumpFrame(cur_frame);
#endif
                        abort();
                        return throw(thread, "java.lang.NoSuchMethodException",
                                getClassName(mi->class), pc);
                    }

                    mi->meth_cache = meth;

                    if (cur_frame->sp <= meth->desc.num_params)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

                    Operand *op_obj = cur_frame->stack[cur_frame->sp - meth->desc.num_params - 1];

                    if (op_obj->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException",
                                "invokevirtual: object is null: " ERR_AT, pc);

                    Object *this = op_obj->val.vref;
                    if (!this) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

                    /*if (mi->meth_cache2)
                        meth = mi->meth_cache2;
                    else*/
                        meth = findMethodByClass(thread,
                            this->class,
                            mi->nametype->nt.name->utf8.ascii,
                            mi->nametype->nt.descriptor->utf8.ascii,
                            INVOKE_VIRTUAL);

                    if (meth == NULL) {
                        warnx("findMethodByClass failed: %s %s %s",
                                this->class ? getClassName(this->class->this_class) :
                                "(NULL this->class)",
                                mi->nametype->nt.name->utf8.ascii,
                                mi->nametype->nt.descriptor->utf8.ascii);

                        return throw(thread, "java.lang.AbstractMethodError",
                                NULL, pc);
                    }

                    mi->meth_cache2 = meth;

                    if (!this->class) {
                        Operand *ret = classInit(thread, (ClassFile *)this->class);
                        if (ret != NULL) return ret;
                    }


                    dbg_cprintf("           [%s.%s%s] => %s.%s%s on %s\n",
                            getClassName(mi->class),
                            mi->nametype->nt.name->utf8.ascii,
                            mi->nametype->nt.descriptor->utf8.ascii,
                            getClassName(meth->class->this_class),
                            meth->name->utf8.ascii,
                            meth->descriptor->utf8.ascii,
                            this->class ?
                            getClassName(this->class->this_class) :
                            "(array)");


                    if ((ex_opr = invokeMethod(thread, meth, true, pc, INVOKE_VIRTUAL)) != NULL)
                        return ex_opr;

                }
                break;
            case 0xb7: /* invokespecial */
                {
                    tmpw = readShort(code, &pc);
                    cp_Fieldref_info *mi = &cls->constant_pool[tmpw]->fr;

                    dbg_cprintf(ANSI_INSTR " invokespecial" ANSI_RESET " #%d ", tmpw);
                    dbg_cprintf("method_info special: %s:%s %s\n",
                            getClassName(mi->class),
                            mi->nametype->nt.name->utf8.ascii,
                            mi->nametype->nt.descriptor->utf8.ascii);

                    method_info *meth;

                    /*if (mi->meth_cache_special)
                        meth = mi->meth_cache_special;
                    else*/
                        meth = findClassMethod(thread,
                            getClassName(mi->class),
                            mi->nametype->nt.name->utf8.ascii,
                            mi->nametype->nt.descriptor->utf8.ascii,
                            INVOKE_SPECIAL);

                    if (meth == NULL) {
                        /* TODO add warn as per above */
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    }

                    mi->meth_cache_special = meth;

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

                    if (meth == NULL) {
                        warnx("findMethodByClass failed: %s %s %s",
                                this->class ? getClassName(this->class->this_class) :
                                "(NULL this->class)",
                                mi->nametype->nt.name->utf8.ascii,
                                mi->nametype->nt.descriptor->utf8.ascii);
                        return throw(thread, "java.lang.AbstractMethodError", NULL, pc);
                    }

                    dbg_cprintf("           [%s.%s%s] => %s.%s%s on %s\n",
                            getClassName(mi->class),
                            mi->nametype->nt.name->utf8.ascii,
                            mi->nametype->nt.descriptor->utf8.ascii,
                            getClassName(meth->class->this_class),
                            meth->name->utf8.ascii,
                            meth->descriptor->utf8.ascii,
                            getClassName(this->class->this_class));


                    if ((ex_opr = invokeMethod(thread, meth, true, pc, INVOKE_SPECIAL)) != NULL)
                        return ex_opr;
                }
                break;
            case 0xb8: /* invokestatic */
                {
                    tmpw = readShort(code, &pc);
                    cp_Fieldref_info *mi = &cls->constant_pool[tmpw]->fr;

                    dbg_cprintf(ANSI_INSTR " invokestatic" ANSI_RESET " #%d ", tmpw);
                    dbg_cprintf("method_info static %s:%s %s\n",
                            getClassName(mi->class),
                            mi->nametype->nt.name->utf8.ascii,
                            mi->nametype->nt.descriptor->utf8.ascii);

                    /* class containing static mehtod */
                    ClassFile *class = (ClassFile *)findClass(thread,
                            getClassName(mi->class), NULL);

                    method_info *meth;

                    /* static method */
                    if (mi->meth_cache_static)
                        meth = mi->meth_cache_static;
                    else
                        meth = findClassMethod(thread,
                            getClassName(mi->class),
                            mi->nametype->nt.name->utf8.ascii,
                            mi->nametype->nt.descriptor->utf8.ascii,
                            INVOKE_STATIC);

                    if (meth == NULL) {
                        warnx("findClassMethod failed: %s %s %s",
                                getClassName(mi->class),
                                mi->nametype->nt.name->utf8.ascii,
                                mi->nametype->nt.descriptor->utf8.ascii);
                        return throw(thread, "java.lang.AbstractMethodError", ERR_AT, pc);
                    }

                    mi->meth_cache_static = meth;

                    if(!class->isInit) {
                        Operand *ret = classInit(thread, (ClassFile *)class);
                        if (ret != NULL) return ret;
                    }

                    if ((ex_opr = invokeMethod(thread, meth, false, pc, INVOKE_STATIC)) != NULL)
                        return ex_opr;
                }
                break;
            case 0xb9: /* invokeinterface */
                {
                    tmpw = readShort(code, &pc);
                    cp_Fieldref_info *mi = &cls->constant_pool[tmpw]->fr;
                    tmp = code[++pc]; // count
                    pc++; // skip blank field

                    dbg_cprintf(ANSI_INSTR " invokeinterface " ANSI_RESET " #%d,%d\n", tmpw, tmp);
                    dbg_cprintf("(method_info interface %s:%s %s)\n",
                            getClassName(mi->class),
                            mi->nametype->nt.name->utf8.ascii,
                            mi->nametype->nt.descriptor->utf8.ascii);

#ifdef DEBUG
                    dumpFrame(cur_frame);
#endif
                    Operand *op_obj = peeknth(cur_frame, tmp - 1);
                    if (op_obj->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);

                    Object *this = op_obj->val.vref;
                    if (!this) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

                    if (!this->class) {
                        Operand *ret = classInit(thread, (ClassFile *)this->class);
                        if (ret != NULL) return ret;
                    }

                    method_info *meth = findMethodByClass(thread,
                            this->class,
                            mi->nametype->nt.name->utf8.ascii,
                            mi->nametype->nt.descriptor->utf8.ascii,
                            INVOKE_INTERFACE);

                    if (meth == NULL) {
                        warnx("invokeinterface: %s [%s %s] missing",
                                getClassName(this->class->this_class),
                                mi->nametype->nt.name->utf8.ascii,
                                mi->nametype->nt.descriptor->utf8.ascii);
#ifdef DEBUG
                        dumpFrame(cur_frame);
#endif
                        abort();
                        return throw(thread, "java.lang.AbstractMethodError", ERR_AT, pc);
                    } 

                    if (cur_frame->sp <= meth->desc.num_params)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    
                    if ((ex_opr = invokeMethod(thread, meth, true, pc, INVOKE_INTERFACE)) != NULL)
                        return ex_opr;
                }
                break;
            case 0xba: /* invokedynamic PARTIAL */
                {
                    tmpw = readShort(code, &pc);
                    pc++; /* skip 0 */
                    pc++; /* skip 0 */

                    cp_InvokeDynamic_info *idi = &cls->constant_pool[tmpw]->id;

                    printf(ANSI_INSTR " invokedynamic " ANSI_RESET " #%d,%d\n", tmpw, tmp);
                    printf(" %s%s\n",
                            idi->nametype->nt.name->utf8.ascii,
                            idi->nametype->nt.descriptor->utf8.ascii
                            );

                    errx(EXIT_FAILURE, "invokedynamic is not fully supported\n");
                }
                break;
            case 0xbb: /* new */
                {
                    tmpw = readShort(code, &pc);
                    cp_Class_info *ci = &cls->constant_pool[tmpw]->class;

                    dbg_cprintf(ANSI_INSTR " new " ANSI_RESET "#%d ", tmpw);
                    dbg_cprintf("class %s\n", ci->name->utf8.ascii);

                    ClassFile *tmpcls = findClass(thread, ci->name->utf8.ascii, NULL);
                    if (tmpcls == NULL) {
                        printf("new failed\n");
                        return throw(thread, "java.lang.ClassNotFoundException", ERR_AT, pc);
                    }
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

                    dbg_cprintf(ANSI_INSTR " newarray " ANSI_RESET "#%d \n", tmp);

                    Operand *cnt = pop(cur_frame);
                    if (cnt == NULL) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    if (cnt->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);

                    if (cnt->val.vint < 0) {
                        freeOperand(cnt);
                        return throw(thread, "java.lang.NegativeArraySizeException", ERR_AT, pc);
                    }

                    Object *arr = newArray(thread, atypemap[tmp], cnt->val.vint, NULL);
                    freeOperand(cnt);

                    if (arr == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    if (!push(cur_frame, newOperand(TYPE_AREF, arr)))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0xbd: /* anewarray */
                {
                    tmpw = readShort(code, &pc);

                    dbg_cprintf(ANSI_INSTR " anewarray " ANSI_RESET "#%d \n", tmpw);

                    Operand *cnt = pop(cur_frame);
                    if (cnt == NULL) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    if (cnt->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);

                    if (cnt->val.vint < 0) {
                        freeOperand(cnt);
                        return throw(thread, "java.lang.NegativeArraySizeException", ERR_AT, pc);
                    }

                    // FIXME store class from cp[tmpw]
                    ClassFile *class = findClass(thread, getClassName(cls->constant_pool[tmpw]), NULL);
                    if (class == NULL) {
                        printf("anewarray failed\n");
                        return throw(thread, "java.lang.ClassNotFoundException", ERR_AT, pc);
                    }

                    Object *arr = newArray(thread, TYPE_OREF, cnt->val.vint, class);
                    freeOperand(cnt);
                    if (arr == NULL)
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

                    if (!push(cur_frame, newOperand(TYPE_AREF, arr)))
                        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                }
                break;
            case 0xbe: /* arraylength */
                {

                    dbg_cprintf(ANSI_INSTR " arraylength " ANSI_RESET);

                    Operand *op_array = pop(cur_frame);
                    if (op_array == NULL) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    if (op_array->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);

                    Object *array = op_array->val.vref;

                    dbg_cprintf("= %d\n", array->array.len);

                    if (!push(cur_frame, newOperand(TYPE_INT, &array->array.len)))
                        errx(EXIT_FAILURE, "arraylength: stack full");
                    freeOperand(op_array);
                }
                break;
            case 0xbf: /* athrow */
                {

                    dbg_cprintf(ANSI_INSTR " athrow " ANSI_RESET);

                    Operand *ex = pop(cur_frame);
                    Operand *ex_opr = NULL;
                    Operand *tmpop = NULL;

                    if (ex == NULL) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    if (ex->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);

                    while(cur_frame->sp > 0 && (tmpop = pop(cur_frame)) != NULL)
                        freeOperand(tmpop);

                    method_info *mi = findMethodByClass(thread, ex->val.vref->class,
                            "fillInStackTrace", "()Ljava.lang.Throwable;", INVOKE_VIRTUAL);

                    push(cur_frame, dupOperand(ex));

                    if ((ex_opr = invokeMethod(thread, mi, true, pc, INVOKE_VIRTUAL)) != NULL)
                        return ex_opr;
                    else {
                        return ex;
                    }
                }
                break;
            case 0xc0: /* checkcast */
                {
                    tmpw = readShort(code, &pc);
                    cp_Class_info *ci = &cls->constant_pool[tmpw]->class;
                    Operand *ref = pop(cur_frame);
                    if (ref == NULL) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);

                    /* val.vref check for casting null to something */

                    dbg_cprintf(ANSI_INSTR " checkcast " ANSI_RESET "%s >= %s\n",
                            ref->val.vref ? getClassName(ref->val.vref->class->this_class) : "null",
                            ci->name->utf8.ascii);

                    if (!canCast(thread,
                                ref->val.vref ? getClassName(ref->val.vref->class->this_class) : NULL,
                                ci->name->utf8.ascii))
                        return throw(thread, "java.lang.ClassCastException", NULL, pc);
                    push(cur_frame, ref);
                }
                break;
            case 0xc1: /* instanceof */
                {
                    tmpw = readShort(code, &pc);

                    dbg_cprintf(ANSI_INSTR " instanceof\n" ANSI_RESET);

                    Operand *ref = pop(cur_frame);
                    if (ref == NULL) return throw(thread, "java.lang.VirtualMachineError", ERR_AT, pc);
                    if (ref->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);
                    cp_Class_info *ci = &cls->constant_pool[tmpw]->class;
                    tmpOp.val.vint = ref->val.vref->class == findClass(thread, ci->name->utf8.ascii, NULL);

                    push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));

                    freeOperand(ref);
                }
                break;
            case 0xc2: /* monitorenter */
                {
                    Operand *lock = pop(cur_frame);
                    if (lock->type == TYPE_NULL)
                        return throw(thread, "java.lang.NullPointerException", ERR_AT, pc);

                    dbg_cprintf(ANSI_INSTR " monitorenter " ANSI_RESET " %s %s\n",
                            printOpType(lock->type),
                            printOpValue(lock));

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

                    dbg_cprintf(ANSI_INSTR " monitorexit " ANSI_RESET " %s %s\n",
                            printOpType(lock->type),
                            printOpValue(lock));

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

                    dbg_cprintf(ANSI_INSTR " multianewarray " ANSI_RESET "#%d[%d] \n", tmpw, dim);

                    // FIXME store class from cp[tmpw]
                    ClassFile *class = findClass(thread, getClassName(cls->constant_pool[tmpw]), NULL);
                    if (class == NULL) {
                        printf("multianewarray failed\n");
                        return throw(thread, "java.lang.ClassNotFoundException", ERR_AT, pc);
                    }

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
                        sizes[off] = cnt->val.vint;
                        freeOperand(cnt);
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
                    dbg_cprintf(ANSI_INSTR " ifnull\n" ANSI_RESET);
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
                    dbg_cprintf(ANSI_INSTR " ifnonnull\n" ANSI_RESET);
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
                    dbg_cprintf(ANSI_INSTR " goto " ANSI_RESET " %d:%lu\n", bra, pc + bra);
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

#ifdef DEBUG
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
                if (f->local[i]->type == TYPE_OREF && !strcmp("java.lang.String",
                            f->local[i]->val.vref->class->this_class->class.name->utf8.ascii)) {
                    printf("   [String] = ");
                    Object *strObj = f->local[i]->val.vref;
                    const Field *fld = findFieldByName(strObj, "string");
                    if (fld) {
                        const Object *charObj = fld->op->val.vref;
                        if (charObj) {
                            for (ssize_t i = 0; i < charObj->array.len; i++)
                            {
                                if (charObj->array.data[i])
                                    printf("%c", charObj->array.data[i]->val.vchar);
                                else
                                    printf("<NULL>");
                            }
                        }
                    }
                    printf("\n");
                }
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

__attribute__((nonnull))
static Operand *invokeMethod(Thread *thread, method_info *meth, bool withObject, int64_t old_pc, invoke_t /*invoke*/)
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

    if (withObject) {
        throwOp = cur_frame->stack[cur_frame->sp - meth->desc.num_params - 1];
        if (throwOp->type != TYPE_OREF && throwOp->type != TYPE_AREF) {
            printf("(invokeMethod [%s] %s.%s%s ",
                    withObject ? "true": "false",
                    getClassName(meth->class->this_class),
                    meth->name->utf8.ascii,
                    meth->descriptor->utf8.ascii
                  );
            printf("%s %s ", printOpType(throwOp->type), printOpValue(throwOp));
            printf("%s:%u ", cur_frame->class->srcfile->utf8.ascii, getLine(cur_frame));
            printf("\n");
            abort();
            return throw(thread, "java.lang.VirtualMachineError", NULL, thread->pc);
        }
        throwOp = NULL;
    }


    dbg_printf("(invokeMethod [%s] %s.%s%s %s:%u)\n",
            withObject ? "true": "false",
            getClassName(meth->class->this_class),
            meth->name->utf8.ascii,
            meth->descriptor->utf8.ascii,
            cur_frame->class ? cur_frame->class->srcfile->utf8.ascii : "<>",
            getLine(cur_frame)
          );

    /* extract required Frame parameters for native/non-native methods */
    if ((meth->access_flags & (ACC_NATIVE|ACC_STATIC)) == (ACC_NATIVE|ACC_STATIC)) {
        if (!meth->nativeMethod.mstatic)
        {
            errx(EXIT_FAILURE, "Missing static native method in %s %s %s",
                    getClassName(meth->class->this_class),
                    meth->name->utf8.ascii,
                    meth->descriptor->utf8.ascii);
        }
        num_operands = (uint16_t)(meth->desc.num_params + 13U); // FIXME this should be a max(return#, params#)
        num_locals = (uint16_t)(meth->desc.num_params * 2U);
    } else if (meth->access_flags & ACC_NATIVE) {
        if (!meth->nativeMethod.mvirtual)
            errx(EXIT_FAILURE, "Missing virtual native method in %s %s %s",
                    getClassName(meth->class->this_class),
                    meth->name->utf8.ascii,
                    meth->descriptor->utf8.ascii);
        num_operands = (uint16_t)(meth->desc.num_params + 13U); // FIXME this should be a max(return#, params#)
        num_locals = (uint16_t)(meth->desc.num_params * 2U);
    } else {
        if (meth->code_attr == NULL) {
            meth->code_attr = findAttribute(meth->attributes, meth->attributes_count, "Code");
        }
        attribute_info *ai = meth->code_attr;//findAttribute(meth->attributes, meth->attributes_count, "Code");
        if (!ai) {
            char buf[BUFSIZ];
            printAccessFlags(meth->access_flags, buf, sizeof(buf));
            warnx("Missing code attribute for method (%s %s %s [%s])",
                getClassName(meth->class->this_class),
                meth->name->utf8.ascii,
                meth->descriptor->utf8.ascii,
                buf);
#ifdef DEBUG
#endif
            abort();
            return throw(thread, "java.lang.VirtualMachineError", ERR_AT, old_pc);
        }

        if ((code = ai->d.code) == NULL)
            errx(EXIT_FAILURE, "Missing code for method (%s %s %s)",
                    getClassName(meth->class->this_class),
                    meth->name->utf8.ascii,
                    meth->descriptor->utf8.ascii);

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
    method_info *old_meth = thread->cur_method;
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
    dbg_printf("(invoking method %s %s)\n", meth->name->utf8.ascii,
            meth->descriptor->utf8.ascii);
    if ((meth->access_flags & (ACC_NATIVE|ACC_STATIC)) == (ACC_NATIVE|ACC_STATIC)) {
        dbg_printf(" (native+static %s)\n", meth->name->utf8.ascii);
        throwOp = meth->nativeMethod.mstatic(thread, meth->class);
    } else if (meth->access_flags & ACC_NATIVE) {
        dbg_printf(" (native %s)\n", meth->name->utf8.ascii);
        if (!withObject)
            errx(EXIT_FAILURE, "invokeMethod: mvirtual but no withObject");
        if (!newframe->local || !newframe->local[0])
            errx(EXIT_FAILURE, "invokeMethod: missing local[0]");
        if (newframe->local[0]->type != TYPE_OREF && newframe->local[0]->type != TYPE_AREF)
            errx(EXIT_FAILURE, "invokeMethod: not an Object" ERR_AT);
        throwOp = meth->nativeMethod.mvirtual(thread, meth->class,
                newframe->local[0]->val.vref);
    } else {
        dbg_printf(" (normal %s:%u) ",
                newframe->class ? newframe->class->srcfile->utf8.ascii : "<>",
                getLine(newframe));
        if (withObject) {
            dbg_printf("(withObject %s)", printObject(newframe->local[0]->val.vref));
        }
        dbg_printf("\n");
        throwOp = runCode(thread, code, 0, &pc);
    }

redo:
    dbg_printf(ANSI_BRED "(done method %s %s%s)"ANSI_RESET"\n",
            meth->name->utf8.ascii,
            meth->descriptor->utf8.ascii,
            throwOp ? " [throw Object]" : "");

    if (throwOp) {
        th_obj = throwOp->val.vref;
        dbg_printf("(method threw - %s %s)\n", printObject(th_obj), getClassName(th_obj->class->this_class));

        if (meth->code && meth->code->exception_table_length) {
            for (int i = 0; i < meth->code->exception_table_length; i++)
            {
                exception *ex = meth->code->exception_table[i];

                if(pc < ex->start_pc ||
                        pc > ex->end_pc ||
                        (ex->catch_type != NULL &&
                         strcmp(getClassName(th_obj->class->this_class), getClassName(ex->catch_type))))
                    continue;

                dbg_printf("(handler match)\n");
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
                    dbg_printf("(pushing return %s %s)\n",
                            printOpType(newframe->ret->type),
                            printOpValue(newframe->ret)
                          );
                    push(oldframe, dupOperand(newframe->ret));
                    freeAndNullOperand(&newframe->ret);
                } else push(oldframe, newOperand(TYPE_NULL, NULL));
            }
            break;
    } else if (newframe->ret) {
        warnx("newframe: ret is set?");
    }

    /* pop off and free the new frame */
    thread->frame_free--;
    thread->stack[thread->frame_free] = NULL;
    thread->cur_method = old_meth;

    freeFrame(newframe);

    if (old_meth) {
        dbg_printf(ANSI_BMAG "(back to %s.%s %s)\n" ANSI_RESET,
                getClassName(old_meth->class->this_class),
                old_meth->name->utf8.ascii,
                old_meth->descriptor->utf8.ascii);
    } else {
        dbg_printf(ANSI_BYEL "(no parent method)\n" ANSI_RESET);
    }

    if (th_obj) {
        dbg_printf("(returning with thrown object %s)\n", printObject(th_obj));
        return newOperand(TYPE_OREF, th_obj);
    } else
        return NULL;
}

static bool addClass(Thread *thread, ClassFile *cf, Object * /*classLoader*/)
{
    dbg_printf("(addClass: %s)\n", getClassName(cf->this_class));

    assert(cf != NULL);

    uint16_t ptr = 0;

    if (isClassLoaded(thread, getClassName(cf->this_class))) {
        warnx("addClass: %s [exists]", getClassName(cf->this_class));
        abort();
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

    //printf(ANSI_BYEL "addClass: %s\n" ANSI_RESET, getClassName(cf->this_class));

    return true;
}

static long totalOperands = 0;

__attribute__((malloc(freeOperand,1),nonnull))
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
    //dbg_printf("dupOperand: @%p -> @%p %s %s\n", (void *)old, (void *)new, printOpType(new->type), printOpValue(new));
    return new;
}


__attribute__((malloc(freeOperand,1)))
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

            case TYPE_NULL:     ret->val.vint = *(int32_t *)value;      break; /* KLUDGE */

            case TYPE_OREF:
            case TYPE_AREF:
                                {
                                    switch(((Object *)value)->type) {
                                        case OBJ_CLASS:
                                        case OBJ_ARRAY:
                                            break;
                                        default:
                                            warnx("newOperand: attempt to create OREF or AREF to non Object!");
                                            abort();
                                            break;
                                    }
                                    ret->val.vref = (Object *)value;
                                    ret->val.vref->lock++; // lock-- in freeOperand
                                }
                                break;
            default:
                                warnx("newOperand: no idea how to create type: %x with a value", type);
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

    //dbg_printf("newOperand: @%p %s %s\n", (void *)ret, printOpType(ret->type), printOpValue(ret));

    return ret;
}

__attribute__((nonnull))
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

__attribute__((nonnull))
static uint32_t freeOperand(Operand *o)
{
    uint32_t ret = 99999;

    assert(o != NULL);
    assert(o->freed == 0);

    switch(o->type)
    {
        case TYPE_AREF:
        case TYPE_OREF:
            if (o->val.vref) {
                if (o->val.vref->lock == 0) {
                    warnx("freeOperand: attempting to free an unlocked Object. Operand @ %p", (void *)o);
                    //dbg_printf("freeOperand: %s:%s\n", printOpType(o->type), printOpValue(o));
                } else
                    ret = --o->val.vref->lock;
                o->val.vref = NULL;

            } else
                ret = 999999;
            /* FALLTHRU */
        default:
            o->type = TYPE_NULL;
            memset(o, 0, sizeof(Operand));
            o->freed = 1;
            free(o);
            totalOperands--;
            break;
    }

    return ret;
}

__attribute__((nonnull))
static Operand *filedescriptor_sysClose(Thread *thread, ClassFile * /*cls*/,
        Object * /*this*/)
{
    Frame *cur_frame = currentFrame(thread);
    int ret,fd;

    if ( (ret = close(fd = cur_frame->local[1]->val.vint)) == -1 )
        warn("filedescriptor_sysClose: close: %d", fd);

    return NULL;
}

__attribute__((nonnull))
static Operand *file_access(Thread *thread, ClassFile * /*cls*/, Object *this)
{
    const Field *fld = NULL;
    const Object *pathname_str;
    const Object *pathname_ch;

    if (!getObjectFieldValue(this, "pathname", &pathname_str) || pathname_str == NULL)
        return throw(thread, "java.lang.VirtualMachineError", NULL, thread->pc);

    if (!getObjectFieldValue(pathname_str, "string", &pathname_ch) || pathname_ch == NULL)
        return throw(thread, "java.lang.RuntimeException", NULL, thread->pc);

    uint16_t *chars = NULL;
    uint32_t len = pathname_ch->array.len;

    if ((chars = calloc(len, sizeof(uint16_t))) == NULL)
        return throw(thread, "java.lang.RuntimeException", NULL, thread->pc);

    for (ssize_t i = 0; i < pathname_ch->array.len; i++)
        chars[i] = pathname_ch->array.data[i]->val.vchar;

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

__attribute__((nonnull))
static Operand *file_stat(Thread *thread, ClassFile * /*cls*/, Object *this)
{
    const Field *fld = NULL;
    if ((fld = findFieldByName(this, "pathname")) == NULL)
        return throw(thread, "java.lang.VirtualMachineError", NULL, thread->pc);

    const Object *pathname_str = fld->op->val.vref;
    if ((fld = findFieldByName(pathname_str, "string")) == NULL)
        return throw(thread, "java.lang.RuntimeException", NULL, thread->pc);

    const Object *pathname_ch = fld->op->val.vref;
    uint16_t *chars = NULL;
    uint32_t len = pathname_ch->array.len;

    if ((chars = calloc(len, sizeof(uint16_t))) == NULL)
        return throw(thread, "java.lang.RuntimeException", NULL, thread->pc);

    for (ssize_t i = 0; i < pathname_ch->array.len; i++)
        chars[i] = pathname_ch->array.data[i]->val.vchar;

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

__attribute__((nonnull))
static Operand *filedescriptor_sysOpen(Thread *thread, ClassFile * /*cls*/,
        Object * /*this*/)
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
    uint32_t len = name->array.len;

    if ((chars = calloc(len, sizeof(uint16_t))) == NULL)
        return throw(thread, "java.lang.RuntimeException", NULL, thread->pc);

    for (ssize_t i = 0; i < name->array.len; i++)
        chars[i] = name->array.data[i]->val.vchar;

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

__attribute__((nonnull))
static Operand *fileinputstream_readint(Thread *thread, ClassFile * /*cls*/,
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

__attribute__((nonnull))
static Operand *fileoutputstream_writeb(Thread *thread, ClassFile * /*cls*/,
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

__attribute__((nonnull))
static Operand *fileoutputstream_writebarray(Thread *thread, ClassFile * /*cls*/,
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
    if (array == NULL || array->array.len == 0) {
        return NULL;
    }

    if ((buf = malloc(array->array.len)) == NULL) {
        warn("malloc");
        goto fail;
    }

    for (uint32_t i = 0; i < array->array.len; i++)
    {
        Operand *op = array->array.data[i];
        if (!op)
            buf[i] = 0;
        else {
            if (op->type != TYPE_BYTE) {
                warnx("fileoutputstream_writebarray: not TYPE_BYTE or NULL: %d [%d/%d]",
                        op ? op->type : -1,
                        i,
                        array->array.len);
                goto fail;
            }
            buf[i] = op->val.vbyte;
        }
    }
    if (write(fd, buf, array->array.len) == -1) {
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

__attribute__((nonnull))
static Operand *nio_filechannel_init(Thread *thread, ClassFile * /*cls*/, Object *this)
{
    Frame *cur_frame;
    struct nio_filechannel_native *native;
    const Field *fld;
    Object *fdObj = NULL;
    int fd = -1;
    Operand *ret = NULL;

    if ((cur_frame = currentFrame(thread)) == NULL) {
        warnx("nio_filechannel_init: NULL currentFrame!");
        goto fail;
    }

    warnx("nio_filechannel_init: %s", getClassName(this->class->this_class));

    fld = findFieldByName(this, "fdObj");
    if(fld == NULL || fld->op == NULL) {
        warnx("nio_filechannel_init: fdObj not present");
        goto fail;
    }
    if (fld->op->type == TYPE_NULL)
        goto fail;

    fdObj = fld->op->val.vref;
    if (fdObj == NULL) {
        warnx("nio_filechannel_init: fdObj is NULL");
        goto fail;
    }

    fld = findFieldByName(fdObj, "fd");
    if(fld == NULL || fld->op == NULL) {
        warnx("nio_filechannel_init: fd is NULL");
        goto fail;
    }

    fd = fld->op->val.vint;
    if (fd == -1)
        goto fail;

    if ((native = this->obj.native_data = calloc(1,
                    sizeof(struct nio_filechannel_native))) == NULL) {
        warn("nio_filechannel_init: native_data: calloc");
        goto fail;
    }

    native->fd = fd;
    this->obj.native_free = free;

    /* FIXME not sure if this is needed or not ?
    const method_info *mi_put;
    mi_put = findMethodByClass(thread, this->class, "put", "([B)Ljava.nio.ByteBuffer;");
    assert(mi_put != NULL);

    method_info *mi_remaining;
    mi_remaining = findMethodByClass(thread, this->class, "remaining", "(V)I");
    assert(mi_remaining != NULL);

    push(cur_frame, newOperand(TYPE_OREF, this));
    ret = invokeMethod(thread, mi_remaining, true, thread->pc);

    if (ret->type != TYPE_INT)
        goto fail;
    */

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

__attribute__((nonnull, unused))
static Operand *nio_implfilechannel_read(Thread *thread, ClassFile *cls,
        Object *this)
{
    Operand *ret = NULL;

    if (!this->obj.native_data)
        if ((ret = nio_filechannel_init(thread, cls, this)) != NULL)
            return ret;

    return ret;
}

__attribute__((nonnull))
static Operand *niochannels_implfilechannel_nativewrite(Thread *thread, ClassFile *cls,
        Object *this)
{
    Frame *cur_frame = currentFrame(thread);
    Operand *ret = NULL;
    //int len = 0;

#ifdef DEBUG_CODE
    dumpFrame(cur_frame);
#endif

    //warnx("niochannels_implfilechannel_nativewrite");

    if (!this->obj.native_data)
        if ((ret = nio_filechannel_init(thread, cls, this)) != NULL)
            return ret;


    const Field *fld = findFieldByName(this, "fdObj");
    if (fld == NULL) {
        warnx("niochannels_implfilechannel_nativewrite: can't find fdObj");
        return throw(thread, "java.lang.VirtualMachineError", ERR_AT, thread->pc);
    }

    const Object *fdObj = fld->op->val.vref;
    if (fdObj == NULL) {
        warnx("niochannels_implfilechannel_nativewrite: fdObj is NULL");
        return throw(thread, "java.lang.IOException", ERR_AT, thread->pc);
    }

    fld = findFieldByName(fdObj, "fd");
    const int fd = fld->op->val.vint;

    uint8_t a;
    int ret_val;

    a = getLocal(cur_frame, 1)->val.vbyte;
    ret_val = write(fd, &a, 1);

    cur_frame->ret = newOperand(TYPE_INT, &ret_val);

    return NULL;
}

__attribute__((nonnull))
static Object *getClassObject(Thread *thread, ClassFile *cls, bool lock)
{
    int rc;
    char buf[BUFSIZ];
    Object *ret = NULL;

    if (cls->class_object) return cls->class_object;

    if (lock)
        if ((rc = pthread_rwlock_wrlock(&cls->rwlock)) != 0) {
            strerror_r(rc, buf, sizeof(buf));
            warnx("getClassObject: %s", buf);
            return NULL;
        }

    if ((ret = cls->class_object)) goto done;

    ret = cls->class_object = createClass(thread, cls, NULL);

done:
    if (lock)
        pthread_rwlock_unlock(&cls->rwlock);
    return ret;
}

__attribute__((nonnull))
static Operand *javalangmath_sin(Thread *thread, ClassFile * /*cls*/)
{
    Frame *cur_frame = currentFrame(thread);
    const Operand *a = cur_frame->local[0];

    double r = sin(a->val.vdouble);

    cur_frame->ret = newOperand(TYPE_DOUBLE, &r);

    return NULL;
}

__attribute__((nonnull))
static Operand *javalangmath_sinh(Thread *thread, ClassFile * /*cls*/)
{
    Frame *cur_frame = currentFrame(thread);
    const Operand *a = cur_frame->local[0];

    double r = sinh(a->val.vdouble);

    cur_frame->ret = newOperand(TYPE_DOUBLE, &r);

    return NULL;
}

__attribute__((nonnull))
static Operand *javalangmath_cos(Thread *thread, ClassFile * /*cls*/)
{
    Frame *cur_frame = currentFrame(thread);
    const Operand *a = cur_frame->local[0];

    double r = cos(a->val.vdouble);

    cur_frame->ret = newOperand(TYPE_DOUBLE, &r);

    return NULL;
}

__attribute__((nonnull))
static Operand *javalangmath_cosh(Thread *thread, ClassFile * /*cls*/)
{
    Frame *cur_frame = currentFrame(thread);
    const Operand *a = cur_frame->local[0];

    double r = cosh(a->val.vdouble);

    cur_frame->ret = newOperand(TYPE_DOUBLE, &r);

    return NULL;
}

__attribute__((nonnull))
static Operand *javalangmath_sqrt(Thread *thread, ClassFile * /*cls*/)
{
    Frame *cur_frame = currentFrame(thread);
    const Operand *a = cur_frame->local[0];

    double r = sqrt(a->val.vdouble);

    cur_frame->ret = newOperand(TYPE_DOUBLE, &r);

    return NULL;
}

__attribute__((nonnull))
static Operand *javalangmath_atan2(Thread *thread, ClassFile * /*cls*/)
{
    Frame *cur_frame = currentFrame(thread);
    const Operand *b = cur_frame->local[1];
    const Operand *a = cur_frame->local[0];

    double r = atan2(a->val.vdouble, b->val.vdouble);

    cur_frame->ret = newOperand(TYPE_DOUBLE, &r);

    return NULL;
}

__attribute__((nonnull))
static Operand *javalangmath_atan(Thread *thread, ClassFile * /*cls*/)
{
    Frame *cur_frame = currentFrame(thread);
    const Operand *a = cur_frame->local[0];

    double r = atan(a->val.vdouble);

    cur_frame->ret = newOperand(TYPE_DOUBLE, &r);

    return NULL;
}

__attribute__((nonnull))
static Operand *javalangmath_acos(Thread *thread, ClassFile * /*cls*/)
{
    Frame *cur_frame = currentFrame(thread);
    const Operand *a = cur_frame->local[0];

    double r = acos(a->val.vdouble);

    cur_frame->ret = newOperand(TYPE_DOUBLE, &r);

    return NULL;
}

__attribute__((nonnull))
static Operand *javalangmath_asin(Thread *thread, ClassFile * /*cls*/)
{
    Frame *cur_frame = currentFrame(thread);
    const Operand *a = cur_frame->local[0];

    double r = asin(a->val.vdouble);

    cur_frame->ret = newOperand(TYPE_DOUBLE, &r);

    return NULL;
}

__attribute__((nonnull))
static Operand *javalangmath_log(Thread *thread, ClassFile * /*cls*/)
{
    Frame *cur_frame = currentFrame(thread);
    const Operand *a = cur_frame->local[0];

    double r = log(a->val.vdouble);

    cur_frame->ret = newOperand(TYPE_DOUBLE, &r);

    return NULL;
}

__attribute__((nonnull))
static Operand *javalangmath_log10(Thread *thread, ClassFile * /*cls*/)
{
    Frame *cur_frame = currentFrame(thread);
    const Operand *a = cur_frame->local[0];

    double r = log10(a->val.vdouble);

    cur_frame->ret = newOperand(TYPE_DOUBLE, &r);

    return NULL;
}

__attribute__((nonnull))
static Operand *javalangmath_log1p(Thread *thread, ClassFile * /*cls*/)
{
    Frame *cur_frame = currentFrame(thread);
    const Operand *a = cur_frame->local[0];

    double r = log1p(a->val.vdouble);

    cur_frame->ret = newOperand(TYPE_DOUBLE, &r);

    return NULL;
}

__attribute__((nonnull))
static Operand *javalangmath_exp(Thread *thread, ClassFile * /*cls*/)
{
    Frame *cur_frame = currentFrame(thread);
    Operand *a = cur_frame->local[0];

    double r = exp(a->val.vdouble);

    cur_frame->ret = newOperand(TYPE_DOUBLE, &r);

    return NULL;
}

__attribute__((nonnull))
static Operand *javalangmath_hypot(Thread *thread, ClassFile *cls)
{
    Frame *cur_frame = currentFrame(thread);
    Operand *b = cur_frame->local[1];
    Operand *a = cur_frame->local[0];

    double r = hypot(a->val.vdouble, b->val.vdouble);

    cur_frame->ret = newOperand(TYPE_DOUBLE, &r);

    return NULL;
}

__attribute__((nonnull))
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

__attribute__((nonnull))
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

__attribute__((nonnull))
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

__attribute__((nonnull))
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


__attribute__((nonnull))
static Operand *javalangobject_getClass(Thread *thread, ClassFile *cls,
        Object *this)
{
    Frame *cur_frame = currentFrame(thread);
    dbg_printf("getClass: %s @ %p\n", getClassName(this->class->this_class), this->class);
    cur_frame->ret = newOperand(TYPE_OREF, getClassObject(thread,
                (ClassFile *)this->class, true));
    dbg_printf("getClass: ret: %p\n", cur_frame->ret->val.vref);

    return NULL;
}

__attribute__((nonnull))
static Operand *javalangobject_hashCode(Thread *thread, ClassFile *cls,
        Object *this)
{
    Frame *cur_frame = currentFrame(thread);
    int32_t hash = (int32_t)((int64_t)this);
    cur_frame->ret = newOperand(TYPE_INT, &hash);
    return NULL;
}

__attribute__((nonnull))
static Operand *javalangthread_nativeStart(Thread *thread, ClassFile *cls,
        Object *this)
{
    //Frame *cur_frame = currentFrame(thread);
    //Operand *op_th = cur_frame->local[0];
    Thread *newth = newThread((JVM *)thread->jvm, this);
    startThread(newth);
    return NULL;
}

__attribute__((nonnull))
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

__attribute__((nonnull))
static Operand *javalangthread_currentThread(Thread *thread, ClassFile *cls)
{
    Frame *cur_frame = currentFrame(thread);

    if (pthread_rwlock_rdlock(&thread->rwlock))
        err(EXIT_FAILURE, "pthread_rwlock_rdlock");

    push(cur_frame, newOperand(TYPE_OREF, thread->thread));

    pthread_rwlock_unlock(&thread->rwlock);
    return NULL;
}

__attribute__((nonnull))
static Operand *javalangreflectarray_newInstance(Thread *thread, ClassFile *cls)
{
    return NULL;
}

__attribute__((nonnull))
static Operand *javalangsystem_exit(Thread *thread, ClassFile *cls)
{
    Frame *cur_frame = currentFrame(thread);
    Operand *rc_op = getLocal(cur_frame,0); //pop(cur_frame);
    if (rc_op) {
        const int rc = rc_op->val.vint;
        freeAndNullOperand(&rc_op);
        exit(rc); // FIXME exit other threads?
    } else
        exit(EXIT_SUCCESS);
}

__attribute__((nonnull))
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

__attribute__((nonnull))
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

    if ((chars = calloc(name->array.len, sizeof(uint16_t))) == NULL)
        return NULL;

    for (ssize_t i = 0; i < name->array.len; i++)
        chars[i] = name->array.data[i]->val.vchar;

    char *ascii = parseUtf16ToAscii(chars, name->array.len);
    free(chars);
    chars = NULL;

    const size_t len = strlen(ascii);

    for(size_t i = 0; i < len; i++)
        if(ascii[i] == '.') ascii[i]='/';

    const ClassFile *cf = findClass(thread, ascii, this);
    free(ascii);
    ascii = NULL;

    if (cf) {
        cur_frame->ret = newOperand(TYPE_OREF, getClassObject(thread,
                    (ClassFile *)cf, true));
    } else {
        cur_frame->ret = newOperand(TYPE_NULL, NULL);
    }

    return NULL;
}

__attribute__((nonnull))
static bool getObjectFieldValue(const Object *obj, const char *name, void *out)
{
    const Field *fld;

    if ((fld = findFieldByName(obj, name)) == NULL)
        return false;

    if (fld->op == NULL) {
        /* TODO */
        return false;
    }

    switch (fld->op->type)
    {
        case TYPE_BYTE:    *(int8_t  *)out = fld->op->val.vbyte;    break;
        case TYPE_BOOL:    *(bool    *)out = fld->op->val.vbool;    break;
        case TYPE_CHAR:    *(uint16_t*)out = fld->op->val.vchar;    break;
        case TYPE_SHORT:   *(int16_t *)out = fld->op->val.vshort;   break;
        case TYPE_RETADDR: *(uint32_t*)out = fld->op->val.vretaddr; break;
        case TYPE_INT:     *(int32_t *)out = fld->op->val.vint;     break;
        case TYPE_FLOAT:   *(float   *)out = fld->op->val.vfloat;   break;
        case TYPE_DOUBLE:  *(double  *)out = fld->op->val.vdouble;  break;
        case TYPE_LONG:    *(long    *)out = fld->op->val.vlong;    break;
        case TYPE_OREF:
        case TYPE_AREF:
                           *(Object **)out = fld->op->val.vref;     break;
        default:
                           warnx("getObjectFieldValue: unsupported op->type");
                           return false;
                           break;
    }
    return true;
}

__attribute__((nonnull))
static Operand *javalangclassloader_findLoadedClass(Thread *thread, ClassFile * /*cls*/, Object *this)
{
    const Field *fld = NULL;
    uint16_t *chars = NULL;
    Frame *cur_frame = currentFrame(thread);
    Object *str = getLocal(cur_frame, 1)->val.vref;

    if ((fld = findFieldByName(str, "string")) == NULL) {
        warnx("Cannot find string");
        return NULL;
    }

    const Object *name = fld->op->val.vref;

    if ((chars = calloc(name->array.len, sizeof(uint16_t))) == NULL) {
        /* TODO throw memory error ? */
        return NULL;
    }

    for (ssize_t i = 0; i < name->array.len; i++)
        chars[i] = name->array.data[i]->val.vchar;

    char *ascii = parseUtf16ToAscii(chars, name->array.len);
    free(chars);
    chars = NULL;

    if (ascii == NULL) {
        /* TODO throw memory error ? */
        return NULL;
    }

    const ClassFile *cf = findClass2(thread, ascii, false, this); /* should this be NULL? */
    free(ascii);
    ascii = NULL;

    if (cf) {
        cur_frame->ret = newOperand(TYPE_OREF, getClassObject(thread,
                    (ClassFile *)cf, true));
    } else {
        cur_frame->ret = newOperand(TYPE_NULL, NULL);
    }

    return NULL;
}

__attribute__((nonnull))
static Operand *fail_debug_print(Thread *thread, ClassFile *cls)
{
    Frame *cur_frame = currentFrame(thread);
    Operand *argOp = getLocal(cur_frame, 0);
    assert(argOp != NULL);
    Object *argObj = argOp->val.vref;
    assert(argObj != NULL);

    printf("fail_debug_print: [%d]: |<", argObj->array.len);

    bool lastOk = false;

    for (uint32_t i = 0; i < argObj->array.len; i++)
        if (!argObj->array.data[i])
            break;
        else if (isprint(argObj->array.data[i]->val.vbyte)) {
            printf("%c", argObj->array.data[i]->val.vbyte);
            lastOk = true;
        } else {
            if (lastOk)
                fputc(' ', stdout);
            printf("%#x ", argObj->array.data[i]->val.vbyte);
            lastOk = false;
        }

    printf(">|\n");

    return NULL;
}

static int32_t getLine(const Frame *fr)
{
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
    return line;
}


    __attribute__((nonnull))
static Operand *javalangthrowable_fillInStackTrace(Thread *thread,
        ClassFile * /*cls*/, Object *this)
{
    Frame *cur_frame = currentFrame(thread);

    ClassFile *ste_c = findClass(thread, "java.lang.StackTraceElement", NULL);
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
            "(Ljava.lang.String;Ljava.lang.String;Ljava.lang.String;I)V", INVOKE_VIRTUAL);
    Operand *ex_opr;
    Object *str = NULL;

    for (ssize_t sp = thread->frame_free - 2; sp >= 2; sp--, arrpos++)
    {
        assert(sp < trace->array.len);

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
                        getClassName(fr->class->this_class) : "unknown",
                        &str)) != NULL)
            return ex_opr;
        if (!push(cur_frame, newOperand(TYPE_OREF, str))) {
            warnx("unable to push: " ERR_AT);
            return NULL;
        }

        /* methodName */
        if ((ex_opr = newString(thread,
                        fr->mi ? fr->mi->name->utf8.ascii : "unknown",
                        &str)) != NULL)
            return ex_opr;
        if (!push(cur_frame, newOperand(TYPE_OREF, str))) {
            warnx("unable to push: " ERR_AT);
            return NULL;
        }

        /* fileName */
        const char *fileName = (fr->class && fr->class->srcfile) ?
            fr->class->srcfile->utf8.ascii : "unknown";

        if ((ex_opr = newString(thread, fileName, &str)) != NULL)
            return ex_opr;
        if (!push(cur_frame, newOperand(TYPE_OREF, str))) {
            warnx("unable to push: " ERR_AT);
            return NULL;
        }

        int32_t line = -1;

        line = getLine(fr);

        if (line == -1) {
            warnx("No linenumber for %d: mi:%s code:%s lineno:%s", fr->pc,
                    fr->mi ? "true" : "false",
                    (fr->mi && fr->mi->code) ? "true" : "false",
                    (fr->mi && fr->mi->code && fr->mi->code->lineno) ? "true" : "false");
        }
        else {
            dbg_printf(ANSI_BYEL "(fillInStackTrace: %s:%u)\n" ANSI_RESET, fileName, line);
        }

        if (!push(cur_frame, newOperand(TYPE_INT, &line))) {
            warnx("unable to push: " ERR_AT);
            return NULL;
        }

        if ((ex_opr = invokeMethod(thread, mi, true, 0, INVOKE_SPECIAL)) != NULL)
            return ex_opr;

        trace->array.data[arrpos] = newOperand(TYPE_OREF, ste_o);

    }

    cur_frame->ret = newOperand(TYPE_OREF, this);
    return NULL;
}

static const internalClass java_lang_Throwable = {
    .name = "java.lang.Throwable",
    .methods = {
        { .name = "fillInStackTrace", .desc = "()Ljava.lang.Throwable;", .mvirtual = javalangthrowable_fillInStackTrace },
        {.name = NULL}
    }
};

static const internalClass java_lang_Object = {
    .name = "java.lang.Object",
    .methods = {
        { .name = "hashCode", .desc = "()I",                .mvirtual = javalangobject_hashCode },
        { .name = "getClass", .desc = "()Ljava.lang.Class;",.mvirtual = javalangobject_getClass },
        {.name = NULL}
    }
};

static const internalClass java_io_FileOutputStream = {
    .name = "java.io.FileOutputStream",
    .methods = {
        { .name = "write", .desc = "(I)V",  .mvirtual = fileoutputstream_writeb },
        { .name = "write", .desc = "([B)V", .mvirtual = fileoutputstream_writebarray },
        {.name = NULL}
    }
};

static const internalClass java_io_FileInputStream = {
    .name = "java.io.FileInputStream",
    .methods = {
        { .name = "read", .desc = "()I",    .mvirtual = fileinputstream_readint },
        {.name = NULL}
    }
};

static const internalClass java_io_File = {
    .name = "java.io.File",
    .methods = {
        { .name = "stat", .desc = "()V", .mvirtual = file_stat },
        { .name = "access", .desc = "()V", .mvirtual = file_access },
        {.name = NULL}
    }
};

static const internalClass java_io_FileDescriptor = {
    .name = "java.io.FileDescriptor",
    .methods = {
        { .name = "sysClose", .desc = "(I)V",                   .mvirtual = filedescriptor_sysClose },
        { .name = "sysOpen",  .desc = "(Ljava.lang.String;Z)I", .mvirtual = filedescriptor_sysOpen },
        {.name = NULL}
    }
};

static const internalClass java_lang_Thread = {
    .name = "java.lang.Thread",
    .methods = {
        { .name = "nativeStart",    .desc = "()V",                  .mvirtual = javalangthread_nativeStart },
        { .name = "sleep",          .desc = "(JI)V",                .mstatic = javalangthread_sleep },
        { .name = "currentThread",  .desc = "()Ljava.lang.Thread;", .mstatic = javalangthread_currentThread },
        {.name = NULL}
    }
};

static const internalClass java_lang_reflect_Array = {
    .name = "java.lang.reflect/Array",
    .methods = {
        { .name = "newInstance", .desc = "(Ljava.lang.Class;[I)Ljava.lang.Object;", .mstatic = javalangreflectarray_newInstance },
        {.name = NULL}
    }
};

static const internalClass java_lang_System = {
    .name = "java.lang.System",
    .methods = {
        { .name = "exit",               .desc = "(I)V", .mstatic = javalangsystem_exit },
        { .name = "currentTimeMillis",  .desc = "()J",  .mstatic = javalangsystem_currentTimeMillis },
        {.name = NULL}
    }
};

static const internalClass java_lang_ClassLoader = {
    .name = "java.lang.ClassLoader",
    .methods = {
        { .name = "findLoadedClass",    .desc = "(Ljava.lang.String;)Ljava.lang.Class;", .mvirtual = javalangclassloader_findLoadedClass },
        { .name = "findClass",          .desc = "(Ljava.lang.String;)Ljava.lang.Class;", .mvirtual = javalangclassloader_findClass },
        {.name = NULL}
    }
};

static const internalClass java_lang_Double = {
    .name = "java.lang.Double",
    .methods = {
        { .name = "longBitsToDouble",   .desc = "(J)D", .mstatic = javalangdouble_longBitsToDouble },
        { .name = "doubleToRawLongBits",.desc = "(D)J", .mstatic = javalangdouble_doubleToRawLongBits },
        {.name=NULL}
    }
};

static const internalClass java_lang_Float = {
    .name = "java.lang.Float",
    .methods = {
        { .name = "intBitsToFloat",    .desc = "(I)F", .mstatic = javalangfloat_intBitsToFloat    },
        { .name = "floatToRawIntBits", .desc = "(F)I", .mstatic = javalangfloat_floatToRawIntBits },
        {.name=NULL}
    }
};

static const internalClass java_lang_Math = {
    .name = "java.lang.Math",
    .methods = {
        { .name = "atan2",  .desc = "(DD)D",.mstatic = javalangmath_atan2 },
        { .name = "hypot",  .desc = "(DD)D",.mstatic = javalangmath_hypot },
        { .name = "acos",   .desc = "(D)D", .mstatic = javalangmath_acos },
        { .name = "asin",   .desc = "(D)D", .mstatic = javalangmath_asin },
        { .name = "atan",   .desc = "(D)D", .mstatic = javalangmath_atan },
        { .name = "atan2",  .desc = "(D)D", .mstatic = javalangmath_atan2 },
        { .name = "cos",    .desc = "(D)D", .mstatic = javalangmath_cos },
        { .name = "cosh",   .desc = "(D)D", .mstatic = javalangmath_cosh },
        { .name = "sin",    .desc = "(D)D", .mstatic = javalangmath_sin },
        { .name = "sinh",   .desc = "(D)D", .mstatic = javalangmath_sinh },
        { .name = "exp",    .desc = "(D)D", .mstatic = javalangmath_exp },
        { .name = "log",    .desc = "(D)D", .mstatic = javalangmath_log },
        { .name = "log10",  .desc = "(D)D", .mstatic = javalangmath_log10 },
        { .name = "log1p",  .desc = "(D)D", .mstatic = javalangmath_log1p },
        { .name = "sqrt",   .desc = "(D)D", .mstatic = javalangmath_sqrt },
        {.name=NULL}
    }
};

static const internalClass java_nio_channels_ImplFileChannel = {
    .name = "java.nio.channels.ImplFileChannel",
    .methods = {
        //{ .name = "read", .desc = "(Ljava.nio.ByteBuffer;J)J", .meth.mvirtual = nio_implfilechannel_read     },
        //{ .name = "read", .desc = "(Ljava.nio.ByteBuffer;)I",  .meth.mvirtual = nio_implfilechannel_read_pos },
        { .name = "nativeWrite", .desc = "(BJ)I", .mvirtual = niochannels_implfilechannel_nativewrite           },
        {.name=NULL}
    }
};

static const internalClass fail_Debug = {
    .name = "fail.Debug",
    .methods = {
        { .name = "print", .desc = "([B)V", .mstatic = fail_debug_print },
        {.name=NULL}
    }
};

#ifdef DEBUG
__attribute__((nonnull))
static void dumpConstantPool(const int me, const cp_info *cp)
{
    printf(" [%2d]: ", me);

    switch(cp->tag)
    {
        case CONSTANT_Class:
            {
                printf("class:  name[%d]="ANSI_BRED"%s"ANSI_RESET,
                        cp->class.name_index,
                        getClassName(cp)
                        //cp->class.original_name
                        );
            }
            break;
        case CONSTANT_Fieldref:
            {
                const cp_Fieldref_info *ent = &cp->fr;
                printf("fieldr: class[%d]=%s nametype[%d]=%s %s",
                        ent->class_index,
                        getClassName(ent->class),
                        ent->name_and_type_index,
                        ent->nametype->nt.name->utf8.ascii,
                        ent->nametype->nt.descriptor->utf8.ascii
                      );
            }
            break;
        case CONSTANT_Methodref:
            {
                const cp_Fieldref_info *ent = &cp->fr;
                printf("methdr: class[%d]=%s nametype[%d]='%s %s'",
                        ent->class_index,
                        getClassName(ent->class),
                        ent->name_and_type_index,
                        ent->nametype->nt.name->utf8.ascii,
                        ent->nametype->nt.descriptor->utf8.ascii
                      );
            }
            break;
        case CONSTANT_NameAndType:
            {
                const cp_NameAndType_info *ent = &cp->nt;
                printf("nm&typ: name[%d]='%s' descriptor[%d]='%s'",
                        ent->name_index,
                        ent->name->utf8.ascii,
                        //ent->original_name,
                        ent->descriptor_index,
                        ent->descriptor->utf8.ascii
                        //,ent->original_descriptor
                      );
            }
            break;
        case CONSTANT_String:
            {
                const cp_String_info *ent = &cp->str;
                printf("string: utf8[%d]='%s'", ent->string_index,
                        ent->string->utf8.ascii);
            }
            break;
        case CONSTANT_Utf8:
            {
                const cp_Utf8_info *ent = &cp->utf8;
                printf("utf8:   bytes='%s'", ent->ascii);
            }
            break;
        case CONSTANT_Integer:
            {
                const cp_Integer_info *ent = &cp->tint;
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

__attribute__((nonnull))
static void dumpAttribute(const attribute_info *ai)
{
    int val;
    printf("attr=%s ", ai->name ? ai->name->utf8.ascii : "<<missing name>>");
    switch((val = lookupPredefinedAttribute(ai->name->utf8.ascii)))
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
                            ex->catch_type ? getClassName(ex->catch_type) : "<none");
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
        case ATTR_BOOTSTRAPMETHODS:
            printf("<<ATTR_BOOTSTRAPMETHODS[%d] not supported>>", val);
            break;
        case ATTR_INVALID:
            printf("<<ATTR_INVALID[%d] not supported>>", val);
            break;
        case ATTR_EXCEPTIONS:
            printf("number=%d\n", ai->d.exception->number_of_exceptions);
            for (int i = 0; i < ai->d.exception->number_of_exceptions; i++)
            {
                printf("   [%d]: exception=%s\n", i,
                        getClassName(ai->d.exception->exception_index_table[i])
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
        case ATTR_SOURCEFILE:
        case ATTR_LINENUMBERTABLE:
        default:
            printf("<<ATTR [%s] not implemented [%d]>>", ai->name->utf8.ascii, val);
            break;
    }
    printf("\n");
}

__attribute__((nonnull))
static void dumpField(const int me, const field_info *fi)
{
    char buf[BUFSIZ];

    printf(" [%d]: ", me);
    printAccessFlags(fi->access_flags, buf, BUFSIZ);
    printf("access=%s name=%s descriptor=%s\n",
            buf,
            fi->name->utf8.ascii,
            fi->descriptor->utf8.ascii
          );

    for (int i = 0; i < fi->attributes_count; i++)
    {
        printf("  [%d]: ", i);
        dumpAttribute(fi->attributes[i]);
    }
}

__attribute__((nonnull))
static void dumpMethod(const int me, method_info *mi)
{
    printf(" [%2d]: ", me);
    char buf[BUFSIZ];
    printAccessFlags(mi->access_flags, buf, BUFSIZ);
    printf("access=%s name="ANSI_BRED"%s"ANSI_RESET" descriptor="ANSI_BMAG"%s"ANSI_RESET"\n",
            buf,
            mi->name->utf8.ascii,
            //mi->original_name,
            mi->descriptor->utf8.ascii
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
__attribute__((nonnull))
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
            if (obj->heaploc == -1)
                warnx("findHeapSlot: jvm->maxHeap is -1");
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

__attribute__((malloc(freeObject,2),nonnull(1,3)))
static Object *newMultiArray(Thread *thread, uint8_t type, int32_t *sizes,
        uint8_t dim, const ClassFile *clsType)
{
    Object *new_array_obj = NULL;
    int32_t size = 0;
    char buf[BUFSIZ] = {0};
    int rc;
    bool pti = false;
#ifdef DEBUG
    Frame *cur_frame = currentFrame(thread);
#endif

    dbg_printf("newMultiArray [%s, %d[%d]]: called from %s:%u\n",
            printOpType(type), dim, sizes[0],
            cur_frame->class ? cur_frame->class->srcfile->utf8.ascii : "<>",
            getLine(cur_frame));

    if (type == TYPE_NULL) {
        warnx("newMultiArray: type is NULL");
        goto fail;
    }

    if (type == TYPE_OREF && clsType == NULL)
        abort();

    /* danger, we are duplicating the logic of newObject here but with a (potentially
     * bespoke Class */
    if ((new_array_obj = calloc(1, sizeof(Object)))== NULL) {
        warn("newMultiArray: calloc");
        goto fail;
    }

    if ((rc = pthread_mutex_init(&new_array_obj->monitor, NULL)) != 0) {
        strerror_r(rc, buf, sizeof(buf));
        warnx("newMultiArray: pthread_mutex_init: %s", buf);
        goto fail;
    }

    pti = true;

    if (!findHeapSlot(thread, new_array_obj)) {
        warnx("newMultiArray: findHeapSlot");
        goto fail;
    }

    new_array_obj->type = OBJ_ARRAY;
    new_array_obj->array.type = type;

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
        strcat(buf, getClassName(clsType->this_class));
        strcat(buf, ";");
    } else if(type == TYPE_AREF)
        abort();

    if (isClassLoaded(thread, buf)) {
        new_array_obj->class = findClass(thread, buf, NULL);
        //new_array_obj->class_obj = new_array_obj->class->class_object;
        if (!new_array_obj->class)
            warnx("newMultiArray: no class");
    } else {
        dbg_printf("newMultiArray: creating a new class for %s\n", buf);
        if ((new_array_obj->class = createClassForArray(thread, type, dim, clsType)) == NULL)
            abort();
        //new_array_obj->class_obj = new_array_obj->class->class_object;
    }

    dbg_printf("newMultiArray: class is now %s, class_obj is now %s\n",
            getClassName(new_array_obj->class->this_class),
            new_array_obj->class->class_object
            ? getClassName(new_array_obj->class->class_object->class->this_class)
            : "<>");

    new_array_obj->array.len = size;
    new_array_obj->array.dimensions = dim;
    new_array_obj->array.sizes = sizes;
    new_array_obj->array.className = strdup(buf);

    if (size > 0) {
        if ((new_array_obj->array.data = calloc(size, sizeof(Operand *))) == NULL) {
            warn("newMultiArray: calloc");
            goto fail;
        }
    }

#ifdef DEBUG
    dbg_printf("newMultiArray: %s %s [",
            printOpType(type),
            clsType ? getClassName(clsType->this_class) : "<n/a>");
    for (ssize_t i = 0; i < dim ; i++)
        dbg_printf("%d%s",sizes[i], i < dim-1 ? "," : "" );
    dbg_printf("] @ %p\n", (void *)new_array_obj);
#endif
    return new_array_obj;

fail:
    free(sizes);
    sizes = NULL;
    if (new_array_obj) {
        if (pti)
            pthread_mutex_destroy(&new_array_obj->monitor);

        if (/*thread &&*/ new_array_obj->heaploc != - 1) {
            if (pthread_rwlock_wrlock(&((JVM *)thread->jvm)->rwlock))
                err(EXIT_FAILURE, "newMultiArray: wrlock");
            ((JVM *)thread->jvm)->heap[new_array_obj->heaploc] = NULL;
            if (pthread_rwlock_unlock(&((JVM *)thread->jvm)->rwlock))
                err(EXIT_FAILURE, "newMultiArray: unlock");
        }
        free(new_array_obj);
        new_array_obj = NULL;
    }
    return NULL;

}

__attribute__((nonnull))
static Object *newArray(Thread *thread, uint8_t type, int32_t size, const ClassFile *clsType)
{
    int32_t *const sizes = calloc(2, sizeof(uint32_t));

    if (sizes == NULL) {
        warn("newArray: calloc");
        return NULL;
    }

    sizes[0] = size;
    sizes[1] = 0;

    return newMultiArray(thread, type, sizes, 1U, clsType);
}

__attribute__((nonnull))
static uint16_t getSuperFieldCount(const Thread *thread, const ClassFile *const cls)
{
    assert(thread->jvm->VM == true);
    assert(cls != NULL);

    if(cls->super_class)
        return (uint16_t)(cls->fields_count + getSuperFieldCount(thread,
                    findClass((Thread *)thread, cls->super_name, NULL)));
    else
        return cls->fields_count;
    return 0;
}

__attribute__((malloc(freeObject,2)))
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
#ifdef DEBUG
    printf("newObject: %s\n", getClassName(cls->this_class));
#endif

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

    if (!thread) {
        warnx("newObject: allocated off heap");
        ret->heaploc = -1;
    } else if (!findHeapSlot(thread, ret)) {
        warnx("newObject: no heapslot");
        goto fail;
    }

    ret->type = OBJ_CLASS;
    ret->class = cls;

    field_count = getSuperFieldCount(thread, cls);

    if ((fields = ret->obj.fields = calloc(field_count + 1U,
                    sizeof(Field))) == NULL) {
        warn("newObject: fields calloc");
        goto fail;
    }

    ret->obj.num_fields = field_count;

    while(super)
    {
        for (int i = 0; i < super->fields_count; fldptr++, i++)
        {
            uint64_t nowt = 0;

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
            super = findClass(thread, super->super_name, NULL);
        } else
            super = NULL;
    }

    dbg_printf("newObject: OBJ_CLASS heap_id=%d class=%s @ %p\n",
            ret->heaploc,
            getClassName(cls->this_class),
            (void *)ret);
    return ret;

fail:
    warnx("Unable to create Object from %s",
            getClassName(cls->this_class));
    if (ret) {
        if (pti) {
            pthread_mutex_destroy(&ret->monitor);
        }
        if (ret->type == OBJ_CLASS) {
            if (ret->obj.fields) {
                free(ret->obj.fields);
                ret->obj.fields = NULL;
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
__attribute__((nonnull))
void dumpClass(const ClassFile *cf)
{
    char buf[BUFSIZ];

    printf( "Class: %s\n",
            getClassName(cf->this_class)
            //cf->canonical_name,
            //cf->original_name
            );
    printAccessFlags(cf->access_flags, buf, BUFSIZ);
    printf( "       version:%d.%d access_flags:%s super:%s this_class_idx:%d\n",
            cf->major_version,
            cf->minor_version,
            buf,
            cf->super_class ? getClassName(cf->super_class) : "(none)",
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

__attribute__((nonnull(1,2)))
static ClassFile *loadClass(Thread *const thread, const char *restrict sfile, Object *classLoader)
{
    ClassFile *ret;
    FILE *fp;
    char buf[BUFSIZ];
    char file[BUFSIZ];

    const char *start;

    assert(sfile != NULL);
    assert(*sfile != 0);

    dbg_printf("(loadClass: %s)\n", sfile);

    if ((fp = fopen(sfile, "r")) != NULL) {
        dbg_printf("(loadClass trying.0: %s direct found)\n", sfile);
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

    dbg_printf("(loadClass normalised to: %s)\n", file);

    if (strchr(file, '/')) {
        if(snprintf(buf, sizeof(buf), "%s/%s%s", opt_classpath, file,
                    strstr(file, ".class") != NULL ? "" : ".class") > (int)sizeof(buf)) {
            STRING_TOO_LONG();
        }
        dbg_printf(" (loadClass trying.1: %s)\n", buf);
        fp = fopen(buf, "r");

    } else {

        /* this is a bit of a mess as .2 .3 .4 are (rarely? never?) used and
         * should be instead made to use the classpath to search */
        snprintf(buf, sizeof(buf), "%s", file);

        dbg_printf(" (loadClass trying.2: %s)\n", buf);
        if ((fp = fopen(buf, "r")) == NULL) {
            for (int i = 0; search[i]; i++)
            {
                if(snprintf(buf, sizeof(buf), "%s/%s",
                            search[i], file) > (int)sizeof(buf)) {
                    STRING_TOO_LONG();
                }
                dbg_printf(" (loadClass trying.3: %s)\n", buf);
                if ((fp = fopen(buf, "r")) != NULL) break;

                if(snprintf(buf, sizeof(buf), "%s/%s.class", search[i],
                            file) > (int)sizeof(buf)) {
                    STRING_TOO_LONG();
                }
                dbg_printf(" (loadClass trying.4: %s)\n", buf);
                if ((fp = fopen(buf, "r")) != NULL) break;

            }
        }
    }

    if (fp == NULL) {
        warnx("loadClass: no such class: %s", buf);
        return NULL;
    }

found:
    ret = parseClass(thread, fp, sfile, classLoader);
    if (!ret)
        warnx("loadClass: parseClass failed for %s", buf);
    fclose(fp);

    return ret;
}

__attribute__((nonnull))
static void freeAttribute(attribute_info *const ai)
{
    const struct predefinedAttribute *pa;
    bool found = false;

    for (int i = 0; ((pa = &predefinedAttrs[i])->name) != NULL; i++)
    {
        if (!ai->name || !ai->name->utf8.ascii ||
                strcmp(ai->name->utf8.ascii, pa->name)) continue;

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
            case ATTR_BOOTSTRAPMETHODS:
                found = true;
                for (int j = 0; j < ai->d.bootstrap->num_bootstrap_methods; j++) {
                    if (ai->d.bootstrap->bootstrap_methods[j]) {
                        if (ai->d.bootstrap->bootstrap_methods[j]->bootstrap_arguments) {
                            free(ai->d.bootstrap->bootstrap_methods[j]->bootstrap_arguments);
                            ai->d.bootstrap->bootstrap_methods[j]->bootstrap_arguments = NULL;
                        }
                    }
                    free(ai->d.bootstrap->bootstrap_methods[j]);
                    ai->d.bootstrap->bootstrap_methods[j] = NULL;
                }
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

#ifdef DEBUG
__attribute__((nonnull))
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
                    printOpType(o->array.type),
                    o->array.len);
            if (o->array.type == TYPE_OREF)
                fprintf(bfp, " %s",
                    o->array.className);
            break;

        case OBJ_CLASS:
            if (o->class && o->class->this_class)
                fprintf(bfp, "%s", getClassName(o->class->this_class));
            else
                fprintf(bfp, "<null class name>");
            break;
        default:
            errx(EXIT_FAILURE, "printObject: unknown type: %d", o->type);
    }

    fclose(bfp);
    return buf;
}
#endif

/* caller must have a wrlock on jvm */
__attribute__((nonnull))
static bool freeObject(JVM *const jvm, Object *restrict const o)
{
    assert(o != NULL);
    assert(jvm != NULL);

    if (o->type == OBJ_CLASS) {
        assert(o->class != NULL);
    }

    if (o->type == OBJ_CLASS && o->class->isInnerClass) {
        if (o->lock > 1) /* TODO where is this extra lock set? */
            return false;
    } else if (o->lock) {
        return false;
    }

    if ((pthread_mutex_trylock(&o->monitor) != 0)) {
        return false;
    }

    assert(o->freed == 0);

    //dbg_printf("freeObject: %p lock:%d %s\n", (void *)o, o->lock, printObject(o));

    switch (o->type)
    {
        case OBJ_CLASS:
            {
                if (o->obj.fields) {
                    for (int i = 0; i < o->obj.num_fields; i++)
                    {
                        Operand *tmpO;
                        if ( (tmpO = o->obj.fields[i].op) != NULL ) {
                            freeAndNullOperand(&tmpO);
                        }
                        o->obj.fields[i].op = NULL;
                    }
                    free(o->obj.fields);
                    o->obj.fields = NULL;
                }

                if (o->obj.native_data) {
                    assert(o->obj.native_free != NULL);
                    o->obj.native_free(o->obj.native_data);
                    o->obj.native_data = NULL;
                }
            }
            break;

        case OBJ_ARRAY:
            {
                int count = 0;
                for(uint32_t i = 0; i < o->array.len; i++) {
                    if(!o->array.data[i])
                        continue;

                    count++;
                    freeAndNullOperand(&o->array.data[i]);
                }
                //printf("Freeing Array [%d]\n", count);
                free(o->array.data);
                o->array.data = NULL;
                free(o->array.sizes);
                o->array.sizes = NULL;
                free((void *)o->array.className);
                o->array.className = NULL;
            }
            break;

        default:
            warnx("freeObject: unknown type %d", o->type);
    }

    if (o->heaploc != -1) {
        jvm->heap[o->heaploc] = NULL;
    } else {
        warnx("freeObject: heaploc is -1");
        abort();
    }

    pthread_mutex_unlock(&o->monitor);
    pthread_mutex_destroy(&o->monitor);

    // use-after-free
    //if (o->class && o->class->isDynamic)
    //    freeClass(jvm, (ClassFile *)o->class);

    memset(o, 0, sizeof(Object));
    o->freed = 1;
    free(o);
    return true;
}

/* caller must have wrlock on jvm */
__attribute__((nonnull))
static void freeClass(JVM *const jvm, ClassFile *restrict const cf)
{
    char buf[BUFSIZ] = {0};
    int rc;

    if ((rc = pthread_rwlock_trywrlock(&cf->rwlock))) {
        strerror_r(rc, buf, sizeof(buf));
        warnx("freeClass: pthread_rwlock_wrlock: %s", buf);
        return;
    }

    if (cf->class_object) {
        // lock++ in createClass
        cf->class_object->lock--;
        freeObject(jvm, cf->class_object);
        //cf->class_object->class = NULL;
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
                        free((void *)mi->desc.params[j]);
                    }
                }
                free(mi->desc.params);
                mi->desc.params = NULL;
            }

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

    /* handle dynamically created ClassFile's in newMultiArray */
    if (cf->this_class && !cf->this_class->dyn)
        cf->this_class = NULL;

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
                    }
                    break;
                case CONSTANT_Class:
                    {
                    }
                    break;
                case CONSTANT_String:
                    {
                        cp_String_info *str = &cp->str;
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
                        cp_Utf8_info *utf8 = &cp->utf8;
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

    if (cf->interfaces) {
        free(cf->interfaces);
        cf->interfaces = NULL;
    }

    if (cf->interface_class) {
        free(cf->interface_class);
        cf->interface_class = NULL;
    }

    /* handle dynamically created ClassFile's in newMultiArray */
    if (cf->this_class && cf->this_class->dyn) {
        cp_info *cp;
        if ((cp = cf->this_class->class.name)) {
            if (cp->utf8.ascii)
                free (cp->utf8.ascii);
            if (cp->utf8.utf16)
                free (cp->utf8.utf16);
            if (cp->utf8.bytes)
                free (cp->utf8.bytes);
            free(cp);
        }
        free(cf->this_class);
        cf->this_class = NULL;
    }

    if (cf->heaploc != -1)
        jvm->method_area[cf->heaploc] = NULL;

    pthread_rwlock_unlock(&cf->rwlock);
    pthread_rwlock_destroy(&cf->rwlock);

    free(cf);
}

__attribute__((nonnull))
static bool freeThread(Thread *restrict const t)
{
    if (t->thread) {
        t->thread->lock--; // lock++ in newThread
        if(t->thread->lock != 0)
            return false;
        // warnx("thread still locked");
        // assert(t->thread->lock == 0); /* inner classes explode?? */
    }

    /* checking for missing frames (probably a bug) or the initial frame (not a bug) */
    for (uint16_t i = 0; i < t->frame_free; i++)
        if (t->stack[i]) {
            freeFrame(t->stack[i]);
            t->stack[i] = NULL;
        }

    free(t);
    return true;
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

__attribute__((nonnull))
static void freeJVM(JVM *restrict const j)
{
    int rc;
    char buf[BUFSIZ] = {0};

    int count;

    const struct timespec rqtp = {
        .tv_sec = 0,
        .tv_nsec = 500000L,
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
                    if (ex_op) {
                        const Object *ex_obj = ex_op->val.vref;
                        if (ex_op->type == TYPE_OREF && ex_obj && ex_obj->class) {
                            warnx("Uncaught exception %s",
                                    getClassName(ex_obj->class->this_class));
                        } else {
                            warnx("Uncaught exception (null)");
                        }
                        freeAndNullOperand(&ex_op);
                    }
                    /* might be better to freeThread() here */
                    freeThread(j->threads[id]);
                    j->threads[id] = NULL;
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
        if (ary_maps[i].class_file->class_object->lock > 0)
            ((Object *)ary_maps[i].class_file->class_object)->lock--; // lock++ createClassForArray
        free((char *)ary_maps[i].canonical);
        freeClass(j, ary_maps[i].class_file);
        ary_maps[i].canonical = NULL;
        ary_maps[i].class_file = NULL;
    }

    free(ary_maps);
    ary_maps = NULL;

    for (uint16_t i = 0; i < MAX_METHOD; i++)
    {
        if (j->method_area[i]) {
            ClassFile *cf = j->method_area[i];
            for (int k = 0; k < cf->fields_count; k++)
                if ( (cf->fields[k]->access_flags & ACC_STATIC) == ACC_STATIC )
                    if (cf->fields[k]->static_operand)
                        freeAndNullOperand(&cf->fields[k]->static_operand);
            if (cf->class_object) {
                cf->class_object->lock--;
                freeObject(j, cf->class_object);
                cf->class_object = NULL;
            }
        }
    }

    /* not sure if this is the best place for this or not really */
    for (uint16_t id = 0 ; id < MAX_THREADS; id++) {
        if (j->threads[id] && j->threads[id]->state == TH_DEAD) {
            freeThread(j->threads[id]);
            j->threads[id] = NULL;
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
        if (found == 0)
            break;
    }

    for (uint16_t i = 0; i < MAX_METHOD; i++)
    {
        if (j->method_area[i]) {
            freeClass(j, (ClassFile *)j->method_area[i]);
            j->method_area[i] = NULL;
        }
    }

    for (int k = 0; k < 10; k++)
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
        if (found == 0)
            break;
    }

    if (found) {
#ifdef DEBUG
        warnx("%d objects still remain: totalOperands=%ld", found, totalOperands);
#endif
        for (uint16_t i = 0; i < j->maxHeap; i++)
        {
            if (!j->heap[i]) continue;
#ifdef DEBUG
            Object *o = j->heap[i];
            dbg_printf("Object: %s\n", printObject(o));
            warnx("object: heap[%d=%d] OBJ_%s lock=%d class=%s",
                    o->heaploc,
                    i,
                    o->type == OBJ_CLASS ? "CLASS" : "ARRAY",
                    o->lock,
                    (o->type == OBJ_CLASS) ?
                        (o->class && o->class->this_class) ?
                            getClassName(o->class->this_class) :
                            "<null class>" :
                        printOpType(o->array.type));
#endif
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

__attribute__((nonnull))
static ClassFile *processNatives(Thread *const thread, const internalClass *const ic)
{
    ClassFile *cf = NULL;

    if (!ic->name) {
        warnx("processNatives: missing name");
        goto fail;
    }

    if ((cf = (ClassFile *)findClass(thread, ic->name, NULL)) == NULL) {
        warnx("processNatives: unable to load class: %s", ic->name);
        goto fail;
    }
    
    const method_info *mi;

    for (uint16_t i = 0; i < MAX_METHODS; i++)
    {
        if (!ic->methods[i].name) continue;

        if (ic->methods[i].name == NULL || ic->methods[i].desc == NULL) {
            warnx("processNatives: missing name/desc in %s[%d]", ic->name, i);
            continue;
        }

        if (ic->methods[i].mstatic)
            mi = findMethodByClass(thread, cf, ic->methods[i].name, ic->methods[i].desc, INVOKE_STATIC);
        else
            mi = findMethodByClass(thread, cf, ic->methods[i].name, ic->methods[i].desc, INVOKE_VIRTUAL);

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
            ((method_info *)mi)->nativeMethod.mstatic = ic->methods[i].mstatic;
        else
            ((method_info *)mi)->nativeMethod.mvirtual = ic->methods[i].mvirtual;

        dbg_printf("processNatives: patched in %s:%s\n", ic->methods[i].name,
                ic->methods[i].desc);
    }

    return cf;
fail:
    warnx("processNatives: unable to process: %s", ic->name);
    return NULL;
}

__attribute__((nonnull(1)))
static Thread *newThread(JVM *const jvm, Object *const thread)
{
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
                            "java.lang.Thread", NULL))) == NULL) goto fail;
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
        warnx("newThread: no free threads");
        goto fail;
    }

    jvm->threads[th_id] = ret;
    ret->state = TH_NEW;
    ret->thread_id = th_id;

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
__attribute__((nonnull))
static void *threadinit(void *arg)
{
    int rc;
    Thread *const self = (Thread *)arg;
    char buf[BUFSIZ] = {0};

    assert(self != NULL);

    if ((rc = pthread_rwlock_wrlock(&self->rwlock))) {
        strerror_r(rc, buf, sizeof(buf));
        warnx("threadinit: pthread_rwlock_wrlock: %s", buf);
        return NULL;
    }

    if (self->state != TH_NEW) {
        pthread_rwlock_unlock(&self->rwlock);
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

    /* INVOKE_??? might be wrong */
    Operand *ret = NULL;
    if (isStatic) {
        ret = invokeMethod(self, self->cur_method, false, 0, INVOKE_STATIC); /* STATIC? */
    } else {
        ret = invokeMethod(self, self->cur_method, true, 0, INVOKE_VIRTUAL);
    }

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
                    "printStackTrace", "()V", INVOKE_VIRTUAL);
            freeAndNullOperand(&ret);
            if (mi)
                ret = invokeMethod(self, mi, true, 0, INVOKE_VIRTUAL);
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

__attribute__((nonnull))
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
                th_obj->class, "run", "()V", INVOKE_INTERFACE); /* public interface Runnable */
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
        .tv_nsec = 1000000L,
    };

    nanosleep(&rqtp, NULL);
    return NULL;
}

__attribute__((noreturn,nonnull))
static void show_usage(const char *const name, int rc)
{
    fprintf(stderr, "Usage: %s [-p CLASSPATH] CLASSFILE CLASSNAME\n", name);
    exit(rc);
}

int main(int argc, char *argv[])
{
    {
        int opt;
        while ((opt = getopt(argc, argv, "hp:")) != -1)
        {
            switch (opt)
            {
                case 'h':
                    show_usage(argv[0], EXIT_SUCCESS);
                case 'p':
                    opt_classpath = optarg;
                    break;
                default:
                    show_usage(argv[0], EXIT_FAILURE);
            }
        }
    }


    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    if (argc - optind != 2)
        show_usage(argv[0], EXIT_FAILURE);

    if ((jvm = calloc(1, sizeof(JVM))) == NULL)
        err(EXIT_FAILURE, "boot: calloc(jvm)");

    if ((jvm->heap = calloc(DEF_HEAP, sizeof(Object *))) == NULL)
        err(EXIT_FAILURE, "boot: calloc(jvm->heap[%u])", DEF_HEAP);

    jvm->maxHeap = DEF_HEAP;
    jvm->VM = false;

    int rc;

    if ((rc = pthread_rwlock_init(&jvm->rwlock, NULL)))
        errx(EXIT_FAILURE, "boot: JVM.rwlock: %s", strerror(rc));

    if ((rc = pthread_rwlock_init(&clslock, NULL)))
        errx(EXIT_FAILURE, "boot: clslock: %s", strerror(rc));

    atexit(cleanup);

    Thread *parent = NULL;

    if ((parent = newThread(jvm, NULL)) == NULL)
        errx(EXIT_FAILURE, "boot: newThread");

    if (parent->thread)
        errx(EXIT_FAILURE, "boot: thread not null");

    Frame *cur_frame = parent->stack[parent->frame_free++] = newFrame(3, 2);

    ClassFile *cls_String, *cls_Class, *cls_Object, *cls_ClassLoader, *cls_Thread;

    /* load classes with circular references */
    if ((cls_Object = loadClass(parent, "java.lang.Object", NULL)) == NULL)
        errx(1, "unable to load java.lang.Object");
    if ((cls_String = loadClass(parent, "java.lang.String", NULL)) == NULL)
        errx(1, "unable to load java.lang.String");
    if ((cls_Class = loadClass(parent, "java.lang.Class", NULL)) == NULL)
        errx(1, "unable to load java.lang.Class");
    if ((cls_Thread = loadClass(parent, "java.lang.Thread", NULL)) == NULL)
        errx(1, "unable to load java.lang.Thread");
    if ((cls_ClassLoader = loadClass(parent, "java.lang.ClassLoader", NULL)) == NULL)
        errx(1, "unable to load java.lang.ClassLoader");

    jvm->VM = true;

    dbg_printf("boot: VM now enabled\n");

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

    dbg_printf("boot: linking early classes\n");

    /* ensure getClass() works on them */
    for (uint16_t i = 0; i < MAX_METHOD; i++)
    {
        ClassFile *const ret = (ClassFile *)jvm->method_area[i];
        if (!ret)
            continue;
        if (!ret->isLinked)
            linkClass(parent, ret);
    }

    dbg_printf("boot: patching natives\n");

    /* patch natives */
    
    ClassFile *cf = NULL;
    cf = processNatives(parent, &java_lang_Object);
    if (cf == NULL) errx(EXIT_FAILURE, "boot: unable to process natives for java.lang.Object");

    cf = processNatives(parent, &java_io_FileOutputStream);
    if (cf == NULL) warnx("boot: no java.io.FileOutputStream");

    cf = processNatives(parent, &java_io_FileInputStream);
    if (cf == NULL) warnx("boot: no java.io.FileInputStream");

    cf = processNatives(parent, &java_io_File);
    if (cf == NULL) warnx("boot: no java.io.File");

    cf = processNatives(parent, &java_lang_Thread);
    if (cf == NULL) errx(EXIT_FAILURE, "boot: unable to process natives for java.lang.Thread");

    cf = processNatives(parent, &java_lang_Throwable);
    if (cf == NULL) errx(EXIT_FAILURE, "boot: unable to process natives for java.lang.Throwable");

    cf = processNatives(parent, &java_lang_System);
    if (cf == NULL) warnx("boot: no java.lang.System");

    cf = processNatives(parent, &java_lang_reflect_Array);
    if (cf == NULL) warnx("boot: no java.lang.reflect.Array");

    cf = processNatives(parent, &java_lang_ClassLoader);
    if (cf == NULL) errx(EXIT_FAILURE, "boot: unable to process natives for java.lang.ClassLoader");

    cf = processNatives(parent, &java_io_FileDescriptor);
    if (cf == NULL) warnx("boot: no java.io.FileDescriptor");

    /* load primitive classes */
    for (ssize_t i = 0; primitiveClassMap[i].c_name; i++)
    {
        ClassFile *cls = findClass(parent, primitiveClassMap[i].c_name, NULL);
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

    /* fix all the references from createClassForArray that were unable to primUp */
    const ClassFile *jlClass = findClass2(parent, "java.lang.Class", false, NULL);
    for (ssize_t i = 0; i < num_ary_maps; i++) {
        if (ary_maps[i].class_file) {
            if (ary_maps[i].class_file->super_class != jlClass->this_class)
                continue;

            dbg_printf("boot: checking %s\n", getClassName(ary_maps[i].class_file->this_class));

            Object *tmpObj = ary_maps[i].class_file->class_object;
            assert(tmpObj != NULL);

            Field *fld = findFieldByName(tmpObj, "componentType");
            assert(fld != NULL);

            if (fld->op->type == TYPE_NULL) {
                const int type = fld->op->val.vint; /* kludge */
                dbg_printf("boot: fixing %s\n", getClassName(tmpObj->class->this_class));
                freeAndNullOperand(&fld->op);
                fld->op = newOperand(TYPE_OREF, findObjForPrim(type));
                dbg_printf("boot: fixed to %s %s\n", printOpType(fld->op->type), printOpValue(fld->op));
#ifdef DEBUG
                dumpClass(ary_maps[i].class_file);
#endif
            }
        }
    }

    if ((parent->thread = newObject(parent, findClass2(parent, "java.lang.Thread", false, NULL))) == NULL) {
        errx(EXIT_FAILURE, "boot: unable to create Thread Object");
    }

    parent->thread->lock++;

    cf = processNatives(parent, &java_lang_Math);
    if (cf == NULL) warnx("boot: no java.lang.Math");

    cf = processNatives(parent, &java_lang_Double);
    if (cf == NULL) warnx("boot: no java.lang.Double");

    cf = processNatives(parent, &java_lang_Float);
    if (cf == NULL) warnx("boot: no java.lang.Float");

    cf = processNatives(parent, &java_nio_channels_ImplFileChannel);
    if (cf == NULL) warnx("boot: no java.nio.channels.ImplFileChannel");

    cf = processNatives(parent, &fail_Debug);
    if (cf == NULL) warnx("boot: no fail.Debug");

    cf = findClass(parent, "java.lang.System", NULL);
    if (cf == NULL) exit(1);
    if (!cf->isInit)
        if (classInit(parent, cf))
            errx(EXIT_FAILURE, "boot: error thrown in java.lang.System:<clsinit>");

    const char *const classfile = argv[optind++];
    const char *const classname = argv[optind++];

    cf = loadClass(parent, classfile, NULL);

    printf("boot: VM loaded\n");

    if (!cf)
        errx(EXIT_FAILURE, "boot: no main class");
    linkClass(parent, cf);

    method_info *const m_Main = findClassMethod(parent, classname, "main", "([Ljava.lang.String;)V", INVOKE_STATIC);
    if (m_Main == NULL)
        errx(EXIT_FAILURE, "boot: Cannot find method main in class %s", classname);

    parent->cur_method = m_Main;

    push(cur_frame, newOperand(TYPE_AREF,
                newArray(parent, TYPE_OREF, 0,
                    findClass(parent, "java.lang.String", NULL))));

    startThread(parent);

    const struct timespec rqtp = {
        .tv_sec = 0,
        .tv_nsec = 2000000L,
    };

    nanosleep(&rqtp, NULL);
}
