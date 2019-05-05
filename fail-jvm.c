#define _XOPEN_SOURCE 700
#undef NDEBUG

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define ERR_AT	__FILE__ ":" TOSTRING(__LINE__)

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

/* macros etc */

#define ANSI_INSTR	"\x1b[1;32m"
#define ANSI_BRED	"\x1b[1;31m"
#define ANSI_BYEL	"\x1b[1;33m"
#define ANSI_BMAG	"\x1b[1;35m"
#define ANSI_RESET	"\x1b[0m"

/* access flags */

#define	ACC_PUBLIC		0x0001
#define ACC_PRIVATE		0x0002
#define ACC_PROTECTED	0x0004
#define ACC_STATIC		0x0008
#define ACC_FINAL		0x0010
#define ACC_SUPER		0x0020
#define ACC_SYNCH		0x0020
#define ACC_VOLATILE	0x0040
#define ACC_BRIDGE		0x0040
#define ACC_TRANSIENT	0x0080
#define ACC_NATIVE		0x0100
#define ACC_INTERFACE	0x0200
#define	ACC_ABSTRACT	0x0400
#define ACC_STRICT		0x0800
#define ACC_SYNTHETIC	0x1000
#define ACC_ANNOTATION	0x2000
#define ACC_ENUM		0x4000

#define TYPE_BYTE		'B'
#define TYPE_CHAR		'C'
#define TYPE_DOUBLE		'D'
#define TYPE_FLOAT		'F'
#define TYPE_INT		'I'
#define TYPE_LONG		'J'
#define TYPE_OREF		'L'
#define TYPE_AREF		'['
#define	TYPE_SHORT		'S'
#define	TYPE_BOOL		'Z'
#define TYPE_VOID		'V'
#define TYPE_NULL		'N'
#define TYPE_RETADDR	'\r'	

#define T_BOOLEAN	4
#define T_CHAR		5
#define T_FLOAT		6
#define	T_DOUBLE	7
#define T_BYTE		8
#define	T_SHORT		9
#define	T_INT		10
#define	T_LONG		11
#define T_MAX		12

const uint8_t atypemap[T_MAX] = {
/*	0, 0, 0, 0,
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


#define CONSTANT_Class				7
#define CONSTANT_Double				6
#define CONSTANT_Fieldref			9
#define CONSTANT_Float				4
#define CONSTANT_Integer			3
#define CONSTANT_InterfaceMethodref	11
#define CONSTANT_InvokeDynamic		18
#define CONSTANT_Long				5
#define CONSTANT_MethodHandle		15
#define CONSTANT_MethodType			16
#define CONSTANT_Methodref			10
#define CONSTANT_NameAndType		12
#define CONSTANT_String				8
#define CONSTANT_Utf8				1

struct _cp_info;

typedef struct {
	uint16_t	 length;
	uint16_t	 utf16len;
	uint16_t	*utf16;
	char		*ascii;
	uint8_t		*bytes;
} cp_Utf8_info;

typedef struct {
	uint16_t	descriptor_index;
} cp_MethodType_info;

typedef struct {
	uint8_t		reference_kind;
	uint16_t	reference_index;
} cp_MethodHandle_info;

typedef struct {
	uint16_t	name_index;
	uint16_t	descriptor_index;

	const struct _cp_info	*name;
	const struct _cp_info	*descriptor;
} cp_NameAndType_info;

typedef struct {
	uint16_t	bootstrap_method_attr_index;
	uint16_t	name_and_type_index;
} cp_InvokeDynamic_info;

typedef struct {
	int64_t	c_long;
} cp_Long_info;

typedef struct {
	int32_t	c_int;
} cp_Integer_info;

typedef struct {
	double		c_double;
} cp_Double_info;

typedef struct {
	float		c_float;
} cp_Float_info;

typedef struct {
	uint16_t	string_index;

	/* char[] */
	const struct _Object *intern;
	const struct _cp_info		 *string;
} cp_String_info;

typedef struct {
	uint16_t	name_index;

	const struct _cp_info *name;
} cp_Class_info;


/* Actually covers both Fieldref_info and Methodref_info */
typedef struct {
	uint16_t	class_index;
	uint16_t	name_and_type_index;

	const struct _cp_info	*class;
	const struct _cp_info	*nametype;
} cp_Fieldref_info;

typedef struct _cp_info {
	uint8_t	tag;

	union {
		cp_Utf8_info			utf8;
		cp_MethodHandle_info	mh;
		cp_MethodType_info		mt;
		cp_NameAndType_info		nt;
		cp_InvokeDynamic_info	id;
		cp_Long_info			tlong;
		cp_Integer_info			tint;
		cp_Float_info			tfloat;
		cp_Double_info			tdouble;
		cp_String_info			str;
		cp_Class_info			class;
		cp_Fieldref_info		fr;
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
	const enum std_attr_e	 attr;
	const char				*name;
	const size_t			 size;
};

struct _ClassFile;

typedef struct {
	uint16_t	start_pc;
	uint16_t	end_pc;
	uint16_t	line_number;
} line_number_t;

typedef struct {
	uint16_t		  line_number_table_length;
	line_number_t	 *line_number_table;
} LineNumberTable_attribute;

typedef struct {
	uint16_t	inner_class_info_index;
	uint16_t	outer_class_info_index;
	uint16_t	inner_name_index;
	uint16_t	inner_class_access_flags;

	const cp_info			*inner_ci;
	const cp_info			*outer_ci;
	const struct _ClassFile	*inner_class;
	const cp_info			*inner_name;
} inner_class_def;

typedef struct {
	uint16_t		  number_of_classes;
	inner_class_def	**classes;
} InnerClasses_attribute;

typedef struct {
	uint16_t		 sourcefile_index;
	const cp_info	*name;
} SourceFile_attribute;

typedef struct {
	uint16_t		  number_of_exceptions;
	const cp_info	**exception_index_table;
} Exceptions_attribute;

typedef union _OperandValue {
	struct _Object	*vref;
	int64_t			 vlong;
	int32_t			 vint;
	int8_t			 vbyte;
	double			 vdouble;
	float			 vfloat;
	int16_t			 vshort;
	bool			 vbool;
	uint16_t		 vchar;
	uint32_t		 vretaddr;
} OperandValue;

typedef struct _Operand {
	uint8_t			type;
	OperandValue	val;
} Operand;

typedef struct {
	uint16_t		 constantvalue_index;
	const cp_info	*entry;
	Operand			 op;
} ConstantValue_attribute;

typedef struct {
	uint16_t		 bootstrap_method_ref;
	uint16_t		 num_bootstrap_arguments;
	const uint16_t	*bootstrap_arguments;
} bootstrap_method;

typedef struct {
	uint16_t				  num_bootstrap_methods;
	const bootstrap_method	**bootstrap_methods;
} BootstrapMethods_attribute;

typedef struct {
	uint16_t start_pc;
	uint16_t end_pc;
	uint16_t handler_pc;
	uint16_t catch_type_idx;

	const cp_info *catch_type;
} exception;

struct _attribute_info;

typedef struct {
	uint16_t		  max_stack;
	uint16_t		  max_locals;
	uint32_t		  code_length;
	const uint8_t	 *code;
	uint16_t		  exception_table_length;
	const exception	**exception_table;
	uint16_t		  attributes_count;

	const struct _method_info		 *method;
	const struct _attribute_info	**attributes;
	const LineNumberTable_attribute	 *lineno;
} Code_attribute;

typedef struct {
	uint8_t		tag;
	uint16_t	cpool_index;
} Object_variable_info;

typedef struct {
	uint8_t		tag;
} Basic_variable_info;

union verification_type_info {
	Object_variable_info	ovi;
	Basic_variable_info		bvi;
};

typedef struct {
	uint16_t offset_delta;
} chop_frame;

typedef struct {
	uint16_t	offset_delta;
	uint8_t		local_num;
	
	union verification_type_info **locals;
} append_frame;

typedef struct {
	uint16_t	offset_delta;
	uint16_t	number_of_locals;
	uint16_t	number_of_stack_items;

	union verification_type_info **locals;
	union verification_type_info **stack;
} full_frame;

typedef struct {
	uint16_t	offset_delta;
} same_frame_extended;

typedef struct {
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

typedef struct {
	uint8_t tag;
	enum	frametype_en type;

	union {
		chop_frame			cf;		// FT_CHOP
		append_frame		af;		// FT_APPEND
		full_frame			ff;		// FT_FULL_FRAME
		samelocalsi_frame	slsi;	// FT_SAME_LOCALS_1_STACK_ITEM
		same_frame_extended	sfe;	// FT_SAME_FRAME_EXTENDED
	} i;
} stack_map_frame;

#define ITEM_Top				0
#define	ITEM_Integer			1
#define ITEM_Float				2
#define ITEM_Long				4
#define	ITEM_Null				5
#define ITEM_UninitialisedThis	6
#define ITEM_Object				7

typedef struct {
	uint16_t		  number_of_entries;

	stack_map_frame	**entries;
} StackMapTable_attribute;

typedef struct _attribute_info {
	uint16_t		 attribute_name_index;
	uint32_t		 attribute_length;

	union {
		const Code_attribute			*code;
		const Exceptions_attribute		*exception;
		const StackMapTable_attribute	*stackmap;
		const ConstantValue_attribute	*constant;
		const InnerClasses_attribute	*inner;
		const SourceFile_attribute		*srcfile;
		const LineNumberTable_attribute	*lineno;
		const void						*data;
	} d;

	const cp_info	*name;

	/* must be last for unknown attributes */
	const uint8_t	 info[];
} attribute_info;

/* descriptors */

typedef struct {
	uint8_t	 baseType;
	uint8_t	 array;

	const char	*className;
} des_Field;

typedef struct {
	des_Field	  ret;
	uint16_t	  num_params;

	const des_Field	**params;
} des_Method;

/* field_info */

struct _Operand;

typedef struct {
	uint16_t		access_flags;
	uint16_t		name_index;
	uint16_t		descriptor_index;
	uint16_t		attributes_count;

	const attribute_info	**attributes;

	des_Field		 desc;

	const cp_info	*name;
	const cp_info	*descriptor;
	struct _Operand *static_operand;
	const struct _ClassFile		*class;
} field_info;

/* method_info */

struct _Thread;
struct _Object;

typedef struct _Operand *(*intStaticMethod)(struct _Thread *, 
		const struct _ClassFile *);
typedef struct _Operand *(*intMethod)(struct _Thread *, 
		const struct _ClassFile *, 
		struct _Object *o);

typedef struct _method_info {
	uint16_t		access_flags;
	uint16_t		name_index;
	uint16_t		descriptor_index;
	uint16_t		attributes_count;

	des_Method		 desc;

	const struct _ClassFile	 *class;
	const attribute_info	**attributes;
	const cp_info			 *name;
	const cp_info			 *descriptor;
	union {
		intStaticMethod	mstatic;
		intMethod		mvirtual;
	} nativeMethod;
	const Exceptions_attribute	*exceptions;
	const Code_attribute		*code;
} method_info;

/* ClassFile */

struct _Object;

typedef struct _ClassFile {
	uint32_t		      magic;
	uint16_t		      minor_version;
	uint16_t	          major_version;
	uint16_t			  constant_pool_count;
	const cp_info		**constant_pool;
	uint16_t			  access_flags;
	uint16_t			  this_class_idx;
	uint16_t			  super_class_idx;
	uint16_t			  interfaces_count;
	uint16_t			 *interfaces;
	const cp_info		**interface_class;
	uint16_t			  fields_count;
	const field_info	**fields;
	uint16_t			  methods_count;
	const method_info	**methods;
	uint16_t			  attributes_count;
	const attribute_info **attributes;

	const cp_info		 *this_class;
	const cp_info		 *super_class;
	const cp_info		 *srcfile;
	const char			 *super_name;
	bool				  isInit;
	int32_t				  heaploc;
	struct _Object		 *class_object;
	bool				  isLinked;

	pthread_rwlock_t	  rwlock;
} ClassFile;

/* Object */

typedef struct {
	Operand				*op;
	const field_info	*field_info;
} Field;

#define OBJ_CLASS	1
#define OBJ_ARRAY	2

typedef struct _Object {
	uint8_t		type;
	uint32_t	lock;
	int32_t		heaploc;

	pthread_mutex_t			 monitor;
	const ClassFile			*class;
	const struct _Object	*class_obj;

	/* place other common to both array & object things above here */

	/* things not common in this union */
	union {
		struct {
			uint16_t		  num_fields;
			Field			 *fields;
		} obj;
		struct {
			uint8_t			  type;
			uint8_t			  dimensions;
			uint32_t		  len;
			uint32_t		 *sizes;
			struct _Operand **data;
		} array;
	} data;
} Object;

#define	MAX_STACK	500

uint32_t framecounter = 0;

typedef struct {
	uint32_t	  frameNum;
	uint32_t	  sp;
	uint32_t	  num_local;
	uint32_t	  num_stack;
	uint32_t	  pc;
	Operand		 *ret;
	Operand		**local;

	const ClassFile		*class;
	const method_info	*mi;
	const Operand		**stack;
} Frame;

enum th_state_en {
	TH_NEW,
	TH_RUNNING,
	TH_FINISHED,
	TH_DEAD
};

typedef struct _Thread {
	uint32_t	 pc;
	uint32_t	 frame_free;
	Frame		*stack[MAX_STACK];
	pthread_t	 pthread;
	uint16_t	 thread_id;
	Object		*thread;

	pthread_rwlock_t	rwlock;
	const struct _JVM	*jvm;
	const method_info	*cur_method;
	enum th_state_en	 state;
} Thread;

#define MAX_THREADS	10

typedef struct _JVM {
	pthread_rwlock_t	rwlock;
	bool			 VM;
	bool			 ClassInit;
	Thread			*cur_thread;
	Object			*heap[MAX_STACK];
	const ClassFile	*method_area[MAX_STACK];
	Thread			*threads[MAX_THREADS];
} JVM;

/* hard coded classes */

#define MAX_FIELDS		20
#define MAX_METHODS		20
#define MAX_INTERFACES	5

typedef struct {
	const char *name;
	const char *desc;
	const uint16_t access;
} internalField;

typedef struct {
	const char		*name;
	const char		*desc;
	const uint16_t	 access;
	union {
		intStaticMethod	mstatic;
		intMethod		mvirtual;
	} meth;
} internalMethod;

typedef struct {
	const char *super;
	const char *name;
	const bool override;
	const uint16_t access;
	internalField fields[MAX_FIELDS];
	internalMethod methods[MAX_METHODS];
	const char *interfaces[MAX_INTERFACES];
} internalClass;

typedef struct {
	const char	 type;
	const char	*c_name;
	Object		*class;
} prim_class_map_t;

/* 
 * Constants 
 */

static struct predefinedAttribute predefinedAttrs[] = {
{ATTR_CONSTANT_VALUE,	"ConstantValue",	sizeof(ConstantValue_attribute)},
{ATTR_CODE,				"Code",				sizeof(Code_attribute)},
{ATTR_STACKMAPTABLE,	"StackMapTable",	sizeof(StackMapTable_attribute)},
{ATTR_EXCEPTIONS,		"Exceptions",		sizeof(Exceptions_attribute)},
{ATTR_BOOTSTRAPMETHODS, "BootstrapMethods",	sizeof(BootstrapMethods_attribute)},
{ATTR_INNERCLASSES,		"InnerClasses",		sizeof(InnerClasses_attribute)},
{ATTR_SOURCEFILE,		"SourceFile",		sizeof(SourceFile_attribute)},
{ATTR_LINENUMBERTABLE,	"LineNumberTable",	sizeof(LineNumberTable_attribute)},
{0, NULL, 0},
};

static prim_class_map_t primitiveClassMap[] = {
	{TYPE_INT,		"java/lang/Integer",	NULL},
	{TYPE_CHAR,		"java/lang/Character",	NULL},
	{TYPE_BOOL,		"java/lang/Boolean",	NULL},
	{TYPE_LONG,		"java/lang/Long",		NULL},
	{TYPE_FLOAT,	"java/lang/Float",		NULL},
	{TYPE_DOUBLE,	"java/lang/Double",		NULL},
	{TYPE_SHORT,	"java/lang/Short",		NULL},
	{TYPE_BYTE,		"java/lang/Byte",		NULL},
	{TYPE_VOID,		"java/lang/Void",		NULL},

	{0, NULL, NULL}
};

/*
 * Forward function declarations
 */

static Object *getClassObject(Thread *, ClassFile *);
static bool linkCpString(Thread *, cp_info *);
static bool fixupCpString(Thread *, cp_info *, const cp_info **, const uint16_t);
static bool linkClass(Thread *, ClassFile *);
static Operand *newStringFromChar(Thread *, const Object *, Object **);
static const field_info *findClassFieldForClass(Thread *, const ClassFile *, const char *);
static bool isClassLoaded(const Thread *, const char *);
static void freeClass(JVM *jvm, ClassFile *cf);
static void freeObject(JVM *jvm, Object *o);
static Operand *newString(Thread *, const char *, Object **);
static bool fixupCpUtf8(cp_info *);
static const ClassFile *findClass2(Thread *, const char *, const bool);
static Operand *throw(Thread *, const char *, const char *, int64_t pc);
static Operand *startThread(Thread *);
static Thread *newThread(JVM *, Object *);
static ClassFile *loadClass(Thread *, const char *restrict);
static Object *newArray(Thread *, const uint8_t, const uint32_t, const ClassFile *);
static Object *newObject(Thread *, const ClassFile *);
static Operand *dupOperand(const Operand *);
static Operand *newOperand(const uint8_t, const void *);
static bool addClass(Thread *, const ClassFile *);
static const ClassFile *findClass(Thread *, const char *);
static const method_info *findClassMethod(Thread *, const char *, const char *, const char *);
static const method_info *findMethodByClass(Thread *, const ClassFile *, const char *, const char *);
static Operand *classInit(Thread *, ClassFile *cls);
static void freeOperand(Operand *);
static Operand *invokeMethod(Thread *, const method_info *, bool, int64_t);
static Object *createClass(Thread *, ClassFile *);

#ifdef DEBUG
const char *printObject(const Object *o);
void dumpClass(const ClassFile *restrict);
void dumpFrame(const Frame *f);
void dumpStack(const Frame *frame, const char *op);
void disasm(const uint8_t *code, const uint32_t len);
#endif
ClassFile *buildInternalClass(const internalClass *ic);

/*
 * Inline function defintions
 */

/*
static inline int max(const int a, const int b)
{
	return a < b ? b : a;
}
*/

static inline unsigned int umax(const unsigned int a, const unsigned int b)
{
	return a < b ? b : a;
}

static inline Frame *currentFrame(Thread *thread)
{
	if (thread->frame_free == 0) return NULL;
	return thread->stack[thread->frame_free-1];
}

/*
 * Function defintions
 */

static const char *printOpType(const uint8_t type)
{
	static char buf [BUFSIZ];
	buf[0] = '\0';

	switch(type)
	{
		case TYPE_BYTE: 	return "byte";			break;
		case TYPE_CHAR:		return "char";			break;
		case TYPE_DOUBLE:	return "double";		break;
		case TYPE_FLOAT:	return "float";			break;
		case TYPE_OREF:		return "reference (L)"; break;
		case TYPE_AREF:		return "reference ([)"; break;
		case TYPE_SHORT:	return "short";			break;
		case TYPE_BOOL:		return "bool";			break;
		case TYPE_VOID:		return "void";			break;
		case TYPE_NULL:		return "null";			break;
		case TYPE_RETADDR:	return "returnAddress"; break;
		case TYPE_INT:		return "int";			break;
		case TYPE_LONG:		return "long";			break;
		default:			
							snprintf(buf, sizeof(buf), "<<unknown 0x%02x>>", type);
							return buf;
							break;
	}
}

#ifdef DEBUG
static const char *printOpValue(const Operand *op);
static const char *printArray(const Object *o)
{
	static char buf[BUFSIZ];
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
		case TYPE_BYTE:		snprintf(buf, sizeof(buf), "%d", op->val.vbyte); break;
		case TYPE_CHAR:		snprintf(buf, sizeof(buf), "%c", op->val.vchar); break;
		case TYPE_DOUBLE:	snprintf(buf, sizeof(buf), "%f", op->val.vdouble); break;
		case TYPE_FLOAT:	snprintf(buf, sizeof(buf), "%f", (double)op->val.vfloat); break;
		case TYPE_OREF:		snprintf(buf, sizeof(buf), "%s", 
									op->val.vref->class->this_class->d.class.name->d.utf8.ascii); break;
		case TYPE_AREF:		snprintf(buf, sizeof(buf), "%s:%s", 
									printOpType(op->val.vref->data.array.type),
									printArray(op->val.vref)); break;
		case TYPE_SHORT:	snprintf(buf, sizeof(buf), "%d", op->val.vshort); break;
		case TYPE_BOOL:		snprintf(buf, sizeof(buf), "%d", op->val.vbool); break;
		case TYPE_VOID:		snprintf(buf, sizeof(buf), "<void>"); break;
		case TYPE_NULL:		snprintf(buf, sizeof(buf), "<null>"); break;
		case TYPE_RETADDR:	snprintf(buf, sizeof(buf), "%d", op->val.vretaddr); break;
		case TYPE_INT:		snprintf(buf, sizeof(buf), "%d", op->val.vint); break;
		case TYPE_LONG:		snprintf(buf, sizeof(buf), "%ld", op->val.vlong); break;
		default:			snprintf(buf, sizeof(buf), "<<unknown type>>"); break;
	}

	return buf;
}

void dumpStack(const Frame *frame, const char *op)
{
	if (!frame->sp) {
		printf(" %4s\t<now empty>\n", op);
		return;
	}

	for(uint32_t i = frame->sp; i > 0; i--)
	{
		printf(" %4s\t[%02d] ", 
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

static bool push(Frame *frame, const Operand *op)
{
	if (op == NULL) {
		warnx("push: null Operand");
		return false;
	} else if (frame->sp >= frame->num_stack) {
		printf(" push\t<<FULL>>\n");
		//__asm__("int $3");
		return false;
	} else {
		if (op->type == 0) {
			warnx("push: bad type");
			__asm__("int $3");
		}
		frame->stack[frame->sp++] = op;
		return true;
	}
}

static Operand *pop(Frame *frame)
{
	if (frame->sp == 0) {
		printf(" pop\t<<EMPTY>>\n");
		//__asm__("int $3");
		return NULL;
	} else {
		Operand *ret = (Operand *)frame->stack[--frame->sp];
		frame->stack[frame->sp] = NULL;
		return ret;
	}
}

static Operand *getLocal(Frame *frame, uint16_t slot)
{
	if (slot >= frame->num_local) return NULL;
	return frame->local[slot];
}

static void clearAndFreeLocal(Frame *frame, uint16_t slot)
{
	const Operand *value = getLocal(frame, slot);
	if (value == NULL) return;

	const bool islong = (value->type == TYPE_LONG || value->type == TYPE_DOUBLE);

	if (islong) {
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
	if (value == NULL || slot > frame->num_local-1) return NULL;

	const bool islong = (value->type == TYPE_LONG || value->type == TYPE_DOUBLE);

	clearAndFreeLocal(frame, slot);
	if (islong) clearAndFreeLocal(frame, (uint16_t)(slot+1U));

	frame->local[slot] = value;
	if (islong) frame->local[slot+1U] = value;

	return value;
}

#ifdef DEBUG
static void printAccessFlags(const uint16_t flag, char *buf)
{
	//static char buf[BUFSIZ];
	buf[0] = '\0';

	if (flag & ACC_PUBLIC) strcat(buf, "public ");
	if (flag & ACC_PRIVATE) strcat(buf, "private ");
	if (flag & ACC_PROTECTED) strcat(buf, "protected ");
	if (flag & ACC_STATIC) strcat(buf, "static ");
	if (flag & ACC_FINAL) strcat(buf,"final ");
	if (flag & ACC_SUPER) strcat(buf,"super ");
	if (flag & ACC_VOLATILE) strcat(buf,"volatile ");
	if (flag & ACC_NATIVE) strcat(buf,"native ");
	if (flag & ACC_INTERFACE) strcat(buf,"interface ");
	if (flag & ACC_ABSTRACT) strcat(buf,"abstract ");
	if (flag & ACC_SYNTHETIC) strcat(buf,"synth ");
	if (flag & ACC_ANNOTATION) strcat(buf,"annotation ");
	if (flag & ACC_ENUM) strcat(buf,"enum ");

	//return buf;
}
#endif

inline static bool read4(FILE *f, uint32_t *dst)
{
	if ( (fread(dst, 1, sizeof(uint32_t), f)) != sizeof(uint32_t)) return false;
	*dst = ntohl(*dst);
	return true;
}

inline static bool read2(FILE *f, uint16_t *dst)
{
	if ( (fread(dst, 1, sizeof(uint16_t), f)) != sizeof(uint16_t)) return false;
	*dst = ntohs(*dst);
	return true;
}

inline static bool read1(FILE *f, uint8_t *dst)
{
	if ( (fread(dst, 1, sizeof(uint8_t), f)) != sizeof(uint8_t)) return false;
	return true;
}

static Object *findObjForPrim(uint8_t type)
{
	for (ssize_t i = 0; primitiveClassMap[i].c_name; i++)
		if (primitiveClassMap[i].type == type)
			return primitiveClassMap[i].class;
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

				if (dbl == 0x7ff0000000000000L)
					val = -INFINITY;
				else if (dbl == 0xfff0000000000000L)
					val = INFINITY;
				else if ((dbl >= 0x7ff0000000000001L && dbl <= 0x7ff0000000000001L) ||
						(dbl >= 0xfff0000000000001L && dbl <= 0xffffffffffffffffL))
					val = NAN;
				else {
					int s = ((dbl >> 63) == 0) ? 1 : -1;
					int e = (int)((dbl >> 52) & 0x7FFL);
					long m = (e == 0) ? 
						(long)((dbl & 0xfffffffffffffL) << 1) :
						(long)((dbl & 0xfffffffffffffL) | 0x10000000000000L);
					val = (double)(s * m) * pow(2, e - 1075);
				}
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
				else if (	(tmp > 0x7f800000 && tmp < 0x80000000) || 
						(tmp > 0xf8000000 && tmp <= 0xffffffff)) 
					tmpf = NAN;
				else {
					int s = ((tmp >> 31) == 0) ? 1 : -1;
					int e = ((tmp >> 23) & 0xFF);
					int m = (e == 0) ?
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
			//	ntohl(((cp_Integer_info *)ret)->c_int);
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

static Frame *newFrame(const uint32_t num_stacks, const uint32_t num_locals)
{
	Frame *ret = calloc(1, sizeof(Frame));

	if (ret == NULL) {
		warn("newFrame");
		goto fail;
	}

	ret->frameNum = framecounter++;
	ret->num_stack = umax(1,num_stacks);
	ret->num_local = umax(1,num_locals);

	if ((ret->stack = calloc(num_stacks, sizeof(Operand))) == NULL) {
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
		free(ret);
	}
	return NULL;
}

static void freeFrame(Frame *f)
{
	if (f->local) { 
		for (uint16_t i = 0; i < f->num_local; i++)
			clearAndFreeLocal(f, i);

		free(f->local); 
		f->local = NULL;
	}

	for (uint32_t i = 0; i < f->sp; i++)
	{
		if (f->stack[i])
			freeOperand((Operand *)f->stack[i]);
	}

	free(f);
}

static exception *readException(const uint32_t me, FILE *f, const cp_info **cp, const uint32_t cpcnt)
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

#ifdef DEBUG
void disasm(const uint8_t *code, const uint32_t len)
{
	const uint8_t *ptr = code;
	uint16_t tmp16;
	uint8_t tmp8,tmp82;

	while(ptr < code + len)
	{
		printf("%4ld: ", ptr - code);
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
				if (!read2(f, &ret->ovi.cpool_index))
					return NULL;
			}
			break;

		default:
			warnx("unknown tag: %d", tag);
			return NULL;
	}

	return ret;
}

static attribute_info *readAttribute(Thread *thread, const uint32_t me, FILE *f, 
		const cp_info **cp, const uint32_t cp_cnt)
{
	const JVM *jvm = thread->jvm;
	attribute_info *ret = NULL;
	uint16_t idx;
	uint32_t len;

	if (!read2(f, &idx)) return NULL;
	if (idx > cp_cnt || cp[idx] == NULL || cp[idx]->tag != CONSTANT_Utf8)
		warnx("[%3d] attribute: invalid attribute_name_index %d", me, idx);

	if (!read4(f, &len)) return NULL;

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
					if ((ret = calloc(1, sizeof(attribute_info))) == NULL) goto fail;
					ret->attribute_name_index = idx;
					ret->attribute_length = len;

					Code_attribute *attr = calloc(1, pa->size);
					if (attr == NULL) goto fail;
					if (!read2(f, &attr->max_stack)) goto fail;
					if (!read2(f, &attr->max_locals)) goto fail;
					if (!read4(f, &attr->code_length)) goto fail;

					uint8_t *code = NULL;
					if ((attr->code = code = calloc(1, attr->code_length)) == NULL) goto fail;

					/* FIXME attr->code memory leak */
					if (fread(code, 1, attr->code_length, f) != attr->code_length) goto fail;
					if (!read2(f, &attr->exception_table_length)) goto fail;
					if ((attr->exception_table = calloc(attr->exception_table_length+1U, 
									sizeof(exception *))) == NULL) goto fail;

					/* FIXME attr->exception_table memory leak */
					for (uint32_t j = 0; j < attr->exception_table_length; j++)
						if ((attr->exception_table[j] = 
									readException(j, f, cp, cp_cnt)) == NULL)
							goto fail;
					if (!read2(f, &attr->attributes_count)) goto fail;
					if ((attr->attributes = calloc(attr->attributes_count, 
									sizeof(attribute_info *))) == NULL) goto fail;
					/* FIXME attr->attributes memory leak */
					for (uint32_t j = 0; j < attr->attributes_count; j++) {
						if ((attr->attributes[j] = readAttribute(thread, j, f, cp, cp_cnt)) == NULL)
							goto fail;
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
						warn("readAttribute: CONSTANT_VALUE");
						goto fail;
					}
					ret->attribute_name_index = idx;
					ret->attribute_length = len;
					ConstantValue_attribute *attr = calloc(1, pa->size);
					if (attr == NULL) {
						warn("readAttribute: CONSTANT_VALUE");
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
									sizeof(stack_map_frame *))) == NULL) goto fail;
					ret->d.stackmap = attr;

					for(int i = 0; i < attr->number_of_entries; i++)
					{
						uint8_t tag;
						uint16_t offset_delta;
						if (!read1(f, &tag)) goto fail;

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
							if (vi == NULL) goto fail;
							memcpy(&smf->i.slsi.stack, vi, sizeof(union verification_type_info));
							free(vi);
						} else if (tag <= 246) {
							errx(EXIT_FAILURE, "unsupported tag: %d", tag);
						} else if (tag == 247) {
							errx(EXIT_FAILURE, "unsupported tag: %d", tag);
						} else if (tag <= 250) {
							/* CHOP */
							smf->type = FT_CHOP;
							if (!read2(f, &offset_delta)) goto fail;
							smf->i.cf.offset_delta = offset_delta;
						} else if (tag == 251) {
							smf->type = FT_SAME_FRAME_EXTENDED;
							if (!read2(f, &offset_delta)) goto fail;
							smf->i.sfe.offset_delta = offset_delta;
						} else if (tag <= 254) {
							/* APPEND */
							smf->type = FT_APPEND;
							uint32_t num_locals = tag - 251U;
							if ((smf->i.af.locals = calloc(num_locals, (size_t)sizeof(union verification_type_info *))) == NULL)
								goto fail;
							smf->i.af.local_num = (uint8_t)num_locals;
							if (!read2(f, &offset_delta)) goto fail;
							for (uint32_t j = 0; j < num_locals; j++)
							{
								if((smf->i.af.locals[j] = readVerificationTypeInfo(f)) == NULL)
									goto fail;
							}
						} else if (tag == 255) {
							/* FULL_FRAME */
							smf->type = FT_FULL_FRAME;
							if (!read2(f, &smf->i.ff.offset_delta)) goto fail;
							if (!read2(f, &smf->i.ff.number_of_locals)) goto fail;
							if ((smf->i.ff.locals = calloc(smf->i.ff.number_of_locals,
											(size_t)sizeof(union verification_type_info *))) == NULL)
								goto fail;
							for (uint32_t j = 0; j < smf->i.ff.number_of_locals; j++)
							{
								if((smf->i.ff.locals[j] = readVerificationTypeInfo(f)) == NULL)
									goto fail;
							}
							if (!read2(f, &smf->i.ff.number_of_stack_items)) goto fail;
							if ((smf->i.ff.stack = calloc(smf->i.ff.number_of_stack_items,
											(size_t)sizeof(union verification_type_info *))) == NULL)
								goto fail;
							for (uint32_t j = 0; j < smf->i.ff.number_of_stack_items; j++)
							{
								if((smf->i.ff.stack[j] = readVerificationTypeInfo(f)) == NULL)
									goto fail;
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
						if (!read2(f, &tmp)) goto fail;
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
		if ((ret = calloc(1, sizeof(attribute_info) + len)) == NULL) return NULL;
		ret->attribute_name_index = idx;
		ret->attribute_length = len;
		if (fread((uint8_t *)&ret->info, 1, len, f) != len) goto fail;
		ret->d.data = NULL;
		ret->name = cp[idx];
	} else {
		ret->name = cp[idx];
	}

	return ret;
fail:
	warnx("readAttribute: %d %d", me, pa->attr);
	if (ret) {
		if (ret->d.data) free((void *)ret->d.data);
		free(ret);
	}
	return NULL;
}

static const char *parseParamDescriptor(des_Field *desc, const char *str)
{
	const uint8_t *ptr = (uint8_t *)str;
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
				const uint8_t *tmp = ptr;
				while (*tmp && *tmp != ';') tmp++;
				if (!*tmp) {
					warnx("broken %s", str);
					return NULL;
				}
				tmp++; // skip ;
				desc->className = strndup((char *)ptr, (size_t)(tmp-ptr-1U));
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

static field_info *readField(Thread *thread, FILE *f, const cp_info **cp, const uint32_t cp_cnt, const ClassFile *cls)
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
		warn("parseParamDescriptor");
		goto fail;
	}

	for (uint32_t i = 0; i < ret->attributes_count; i++)
		if ((attrs[i] = readAttribute(thread, i, f, cp, cp_cnt)) == NULL) {
			warnx("readAttribute");
			goto fail;
		}

	ret->attributes = (const attribute_info **)attrs;

	if (ret->access_flags & ACC_STATIC)
		for (uint32_t i = 0; i < ret->attributes_count; i++)
			if (attrs[i] && !strcmp(attrs[i]->name->d.utf8.ascii, "ConstantValue"))
				ret->static_operand = dupOperand(&attrs[i]->d.constant->op);

	return ret;
fail:
	if (ret) {
		if (attrs) {
			for (int i = 0; i < ret->attributes_count; i++)
				if (attrs[i]) free(attrs[i]);
			free(attrs);
			ret->attributes = NULL;
		}
		free(ret);
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

	desc->params = (const des_Field **)ret;

	return true;
fail:
	if (ret)
	{
		for (size_t i = 0; i < num_fields; i++)
		{
			if (!ret[i]) continue;
			free(ret[i]);
		}
		free(ret);
	}
	return false;
}

static method_info *readMethod(Thread *thread, FILE *f, const cp_info **cp, 
		const uint32_t cp_cnt, const ClassFile *class)
{
	method_info *ret = NULL;
	attribute_info **attrs = NULL;

	if ((ret = calloc(1, sizeof(method_info))) == NULL) return NULL;
	ret->class = class;

	if (!read2(f, &ret->access_flags)) goto fail;
	if (!read2(f, &ret->name_index)) goto fail;
	if (!read2(f, &ret->descriptor_index)) goto fail;
	if (!read2(f, &ret->attributes_count)) goto fail;

	if (ret->descriptor_index >= cp_cnt || ret->name_index >= cp_cnt) goto fail;

	ret->name = cp[ret->name_index];
	ret->descriptor = cp[ret->descriptor_index];

	if (!parseMethodDescriptor(&ret->desc, ret->descriptor->d.utf8.ascii))
		goto fail;


	if ((attrs = calloc(ret->attributes_count+1U, sizeof(attribute_info *))) == NULL)
		goto fail;

	for (uint32_t i = 0; i < ret->attributes_count; i++)
		if ((attrs[i] = readAttribute(thread, i, f, cp, cp_cnt)) == NULL) {
			goto fail;
		}

	ret->attributes = (const attribute_info **)attrs;

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
		for (size_t i = 0; i < ret->attributes_count; i++)
			if (attrs[i]) free(attrs[i]);
		free(attrs);
	}
	if (ret) free(ret);
	return NULL;
}

static uint16_t *parseCpUtf8ToUtf16(const uint8_t *src, const uint16_t len, uint16_t *utf16len)
{
	uint16_t ptr = 0;
	uint16_t dst = 0;
	uint16_t *ret = NULL;
	uint16_t *tmp = NULL;

	if (len == 0) {
		*utf16len = 0;
		return NULL;
	}

	if ((ret = malloc(len * sizeof(uint16_t))) == NULL) return NULL;

	while (ptr < len)
	{
		uint8_t val = src[ptr++];

		if (val == 0) {					// 0000 0000
			warnx("parseCpUtf8ToUtf16: read 0");
			goto fail;
		} 
		else if (val < 0x80)			// 0xxx xxxx
		{
			ret[dst] = val;
			dst++;
		} 
		else if ((val & 0xe0) == 0xc0)	// 110x xxxx
		{ 
			ret[dst] = (uint16_t)(64U * (val & 0x1f));
			ret[dst] = (uint16_t)(ret[dst] + (src[ptr++] & 0x3F));
			dst++;
		} 
		else if ((val & 0xf0) == 0xe0)	// 1110 xxxx
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

static char *parseUtf16ToAscii(const uint16_t *src, const uint32_t len)
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

static bool fixupCpString(Thread *thread, cp_info *ci, const cp_info **pool, const uint16_t cp_len)
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
		freeOperand(ci->d.str.intern->data.array.data[i]);
		ci->d.str.intern->data.array.data[i] = newOperand(TYPE_CHAR, &ci->d.str.string->d.utf8.utf16[i]);
	}

	return true;
}

static bool fixupCpClass(cp_info *ci, const cp_info **pool, const uint16_t cp_len)
{
	if (ci->d.class.name)
		return true;

	if (ci->d.class.name_index >= cp_len) return false;
	if (pool[ci->d.class.name_index]->tag != CONSTANT_Utf8) return false;

	ci->d.class.name = pool[ci->d.class.name_index];

	return true;
}

static bool fixupCpFieldref(cp_info *ci, const cp_info **pool, const uint16_t cp_len)
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

static bool fixupCpNameAndType(cp_info *ci, const cp_info **pool, const uint16_t cp_len)
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

static bool fixupConstant(Thread *thread, cp_info *ci, const cp_info **pool, 
		const uint16_t cp_len)
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

static Operand *loadInnerClass(Thread *thread, const InnerClasses_attribute *ic,
		const ClassFile *cf)
{
	if(thread == NULL || ic == NULL || cf == NULL)
		errx(EXIT_FAILURE, "loadInnerClass: shit");

	char buf[BUFSIZ];
	for (int i = 0; i < ic->number_of_classes; i++)
	{
		inner_class_def *def = ic->classes[i];

		if(!def || !strcmp(def->inner_ci->d.class.name->d.utf8.ascii, 
					cf->this_class->d.class.name->d.utf8.ascii))
			continue;

		if(isClassLoaded(thread, def->inner_ci->d.class.name->d.utf8.ascii))
			continue;

		snprintf(buf, sizeof(buf), "%s.class", def->inner_ci->d.class.name->d.utf8.ascii);

		if ((def->inner_class = loadClass(thread, buf)) == NULL) {
			warnx("loadInnerClass: unable to load %s", buf);
			return throw(thread, "java/lang/ClassNotFoundException", NULL, 0);
		}
		if (thread->jvm->VM) linkClass(thread, (ClassFile *)def->inner_class);
		//addClass(thread, def->inner_class);
	}

	return NULL;
}

static Field *findFieldByName(const Object *o, const char *name)
{
	field_info *fi;

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
	assert(thread->jvm->VM == true);
	Frame *cur_frame = currentFrame(thread);

	/* validate inputs */
	if (buf == NULL) {
		warnx("newStringFromChar: buf == NULL");
		return throw(thread, "java/lang/NullPointerException", ERR_AT, 0);
	}
	
	if (buf->type != OBJ_ARRAY || buf->data.array.type != TYPE_CHAR) {
		warnx("newStringFromChar: buf not [C");
		return throw(thread, "java/lang/IllegalArguementException", ERR_AT, 0);
	}

	/* create the String */
	const ClassFile *str_cls = findClass(thread, "java/lang/String");
	Object *str_obj = newObject(thread, str_cls);
	const Operand *str_op = newOperand(TYPE_OREF, str_obj);
	push(cur_frame, str_op);

	const Operand *arr_op = newOperand(TYPE_AREF, buf);
	push(cur_frame, arr_op);

	/* invoke String.init(char []) */
	const method_info *mi = findMethodByClass(thread, str_cls,
			"<init>", "([C)V");
	Operand *ex_opr = NULL;
	if ((ex_opr = invokeMethod(thread, mi, true, 0)) != NULL) {
		warnx("newStringFromChar: <init> ([C)V threw");
		return ex_opr;
	}

	/* store the result if no exception thrown */
	*result = str_obj;
	return NULL;
}

/* not-jvm-safe */
static Operand *newString(Thread *thread, const char *text, Object **result)
{
	assert(thread->jvm->VM == true);

	*result = NULL;

	/* create the char[] */
	const Object *array = newArray(thread, TYPE_CHAR, (uint32_t)strlen(text), 
			NULL);

	if (array == NULL)
		return throw(thread, "java/lang/RuntimeException", ERR_AT, 0);

	for (uint32_t i = 0; text[i]; i++)
	{
		const uint16_t ch = (uint8_t)text[i];
		freeOperand(array->data.array.data[i]);
		array->data.array.data[i] = newOperand(TYPE_CHAR, &ch);
	}

	return newStringFromChar(thread, array, result);
}

static pthread_rwlock_t clslock;

typedef struct {
	const char		*canonical;
	const Object	*object;
} ary_map_t;

static uint32_t num_ary_maps = 0;
static ary_map_t *ary_maps = NULL;


static const Object *createClassForArray(Thread *thread, uint8_t type, 
		uint8_t dim, const ClassFile *compClass)
{
	assert(thread->jvm->VM == true);
	int rc;

	char buf[BUFSIZ] = {0};
	for(ssize_t i = 0; i < dim; i++)
		strcat(buf, "[");

	buf[dim] = (char)(type);
	buf[dim+1] = '\0';

	if(compClass) {
		strcat(buf, compClass->this_class->d.class.name->d.utf8.ascii);
		strcat(buf, ";");
	}

	if ((rc = pthread_rwlock_rdlock(&clslock)) != 0) {
		strerror_r(rc, buf, sizeof(buf));
		warnx("createClassForArray: pthread_rwlock_rdlock: %s", buf);
		return NULL;
	}

	for (ssize_t i = 0; i < num_ary_maps; i++)
		if (!strcmp(buf, ary_maps[i].canonical)) {
			pthread_rwlock_unlock(&clslock);
			return ary_maps[i].object;
		}

	pthread_rwlock_unlock(&clslock);

#ifdef DEBUG
	printf("Creating class for %s\n", buf);
#endif

	const ClassFile *class_cf = findClass2(thread, "java/lang/Class", false);
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
		warn("createClassForArray: realloc");
	}

	ary_maps = tmp;
	ary_maps[num_ary_maps].object = ret;
	ary_maps[num_ary_maps].canonical = strdup(buf);
	num_ary_maps++;

	pthread_rwlock_unlock(&clslock);

	const method_info *mi = findMethodByClass(thread, class_cf,
			"<init>", "()V");
	if (!push(cur_frame, newOperand(TYPE_OREF, ret))) {
		ret->lock--;
		warnx("createClassForArray: push");
		return NULL;
	}
	if (invokeMethod(thread, mi, true, 0))
		errx(EXIT_FAILURE, "createClassForArray: thrown");

	Field *fld = NULL;
	Object *str = NULL;
	Operand tmpOp;

	if ((fld = findFieldByName(ret, "isArray")) == NULL) {
		ret->lock--;
		warnx("createClass: isArray");
		return NULL;
	}
	if (fld->op) freeOperand(fld->op);
	tmpOp.val.vbool = true;
	fld->op = newOperand(TYPE_BOOL, &tmpOp.val.vbool);

	if ((fld = findFieldByName(ret, "name")) == NULL)
		errx(EXIT_FAILURE, "createClass: name");
	if (fld->op) freeOperand(fld->op);
	if ((newString(thread, buf, &str)) != NULL)
		errx(EXIT_FAILURE, "createClass: newString threw");
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
	if (fld->op) freeOperand(fld->op);
	if ((newString(thread, buf, &str)) != NULL)
		errx(EXIT_FAILURE, "createClass: newString threw");
	fld->op = newOperand(TYPE_OREF, str);

	if ((fld = findFieldByName(ret, "componentType")) == NULL)
		errx(EXIT_FAILURE, "createClass: componentType");
	if (fld->op) freeOperand(fld->op);
	if (compClass) {
		fld->op = newOperand(TYPE_OREF, getClassObject(thread, 
					(ClassFile *)compClass));
	} else
		fld->op = newOperand(TYPE_OREF, findObjForPrim(type));

	// TODO componentType for primitives

	return ret;
}

/* not-jvm-safe */
static Object *createClass(Thread *thread, ClassFile *cf)
{
	assert(thread->jvm->VM == true);

	const ClassFile *class_cf = findClass2(thread, "java/lang/Class", false);
	if (!class_cf) return NULL;

	Frame *cur_frame = currentFrame(thread);
	Object *ret = newObject(thread, class_cf);
	// lock-- in freeClass
	ret->lock++;
	const method_info *mi = findMethodByClass(thread, class_cf,
			"<init>", "()V");
	push(cur_frame, newOperand(TYPE_OREF, ret));
	if (invokeMethod(thread, mi, true, 0))
		errx(EXIT_FAILURE, "createClass: thrown");

	Field *fld = NULL;
	Object *str = NULL;

	if ((fld = findFieldByName(ret, "canonicalName")) == NULL)
		errx(EXIT_FAILURE, "createClass: canonicalName");
	if (newString(thread, cf->this_class->d.class.name->d.utf8.ascii, &str))
		errx(EXIT_FAILURE, "createClass: newString");
	if (fld->op) freeOperand(fld->op);
	fld->op = newOperand(TYPE_OREF, str);

	if ((fld = findFieldByName(ret, "simpleName")) == NULL)
		errx(EXIT_FAILURE, "createClass: simpleName");
	if (newString(thread, cf->this_class->d.class.name->d.utf8.ascii, &str))
		errx(EXIT_FAILURE, "createClass: newString");
	if (fld->op) freeOperand(fld->op);
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

	if (!read4(fp, &ret->magic)) err(EXIT_FAILURE, "parseClass");

	if (ret->magic != 0xcafebabe)
		errx(EXIT_FAILURE, "Bad magic: 0x%0x", ret->magic);

	if (!read2(fp, &ret->minor_version)) err(EXIT_FAILURE, "parseClass");
	if (!read2(fp, &ret->major_version)) err(EXIT_FAILURE, "parseClass");

	if (ret->major_version > 52) err(EXIT_FAILURE, "Unsupported JRE version");

	if (!read2(fp, &ret->constant_pool_count)) err(EXIT_FAILURE, "parseClass");
	if ((ret->constant_pool = calloc(ret->constant_pool_count+2U, 
					sizeof(cp_info *))) == NULL)
		err(EXIT_FAILURE, "parseClass");

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
		err(EXIT_FAILURE, "parseClass");

	ret->this_class = ret->constant_pool[ret->this_class_idx];
	ret->super_class = ret->constant_pool[ret->super_class_idx];

	if (!read2(fp, &ret->interfaces_count)) err(EXIT_FAILURE, "parseClass");
	if ((ret->interfaces = calloc(ret->interfaces_count+1U, 
					sizeof(uint16_t))) == NULL)
		err(EXIT_FAILURE, "parseClass");

	for(int i = 0; i < ret->interfaces_count; i++)
		if (!read2(fp, &ret->interfaces[i])) 
			err(EXIT_FAILURE, "read interface[%d]", i);

	if (!read2(fp, &ret->fields_count)) err(EXIT_FAILURE, "parseClass");
	if ((ret->fields = calloc(ret->fields_count+1U, 
					sizeof(field_info *))) == NULL)
		err(EXIT_FAILURE, "parseClass");

	for(int i = 0; i< ret->fields_count; i++)
		if ((ret->fields[i] = readField(thread, fp, ret->constant_pool, 
						ret->constant_pool_count, ret)) == NULL)
			errx(EXIT_FAILURE, "readField[%d]", i);

	if (!read2(fp, &ret->methods_count)) err(EXIT_FAILURE, "parseClass");
	if ((ret->methods = calloc(ret->methods_count+1U, 
					sizeof(method_info *))) == NULL)
		err(EXIT_FAILURE, "parseClass");

	for(int i = 0; i < ret->methods_count; i++)
		if ((ret->methods[i] = readMethod(thread, fp, ret->constant_pool, 
						ret->constant_pool_count, ret)) == NULL) {
			errx(EXIT_FAILURE, "readMethod[%d]", i);
		} else {
			for(int j = 0; j < ret->methods[i]->attributes_count; j++)
			{
				if (!strcmp(ret->methods[i]->attributes[j]->name->d.utf8.ascii, "Code"))
					((Code_attribute *)ret->methods[i]->attributes[j]->d.code)->method = ret->methods[i];
			}
		}

	if (!read2(fp, &ret->attributes_count)) err(EXIT_FAILURE, "parseClass");
	if ((ret->attributes = calloc(ret->attributes_count+1U, 
					sizeof(attribute_info *))) == NULL)
		err(EXIT_FAILURE, "parseClass");

	for(uint32_t i = 0; i < ret->attributes_count; i++)
		if ((ret->attributes[i] = readAttribute(thread, i, fp, 
						ret->constant_pool, 
						ret->constant_pool_count)) == NULL)
			errx(EXIT_FAILURE, "readAttribute[%d]", i);

	if (ret->super_class)
		ret->super_name = strdup(ret->super_class->d.class.name->d.utf8.ascii);

	if ((ret->interface_class = calloc(ret->interfaces_count, 
					sizeof(cp_info *))) == NULL)
			errx(EXIT_FAILURE, "parseClass");

	for(uint32_t i = 0; i < ret->interfaces_count; i++)
		ret->interface_class[i] = ret->constant_pool[ret->interfaces[i]];

	if ((rc = pthread_rwlock_wrlock(&ret->rwlock))) {
		strerror_r(rc, buf, sizeof(buf));
		errx(EXIT_FAILURE, "parseClass: pthread_rwlock_lock: %s", buf);
	}

	if (!addClass(thread, ret))
		return NULL;

	for(uint32_t i = 0; i < ret->attributes_count; i++)
		if (!strcmp(ret->attributes[i]->name->d.utf8.ascii, "InnerClasses")) {
			loadInnerClass(thread, ret->attributes[i]->d.inner, ret);	
		} else if (!strcmp(ret->attributes[i]->name->d.utf8.ascii, "SourceFile")) {
			ret->srcfile = ret->attributes[i]->d.srcfile->name;
		}

	ret->isLinked = false;
	pthread_rwlock_unlock(&ret->rwlock);
	return ret;
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

	//ret->class = (Object *)createClass(thread, ret);
	return true;
}

/*
   static ssize_t num_classes = 0;				// TODO move to JVM
   static const ClassFile **classes = NULL;	// TODO move to JVM
   */

static Operand *throw(Thread *thread, const char *name, const char *text, 
		int64_t pc)
{
	assert(thread->jvm->VM == true);

	warnx(ANSI_BYEL "throw: %s %s" ANSI_RESET, name, text);

	Frame *cur_frame = currentFrame(thread);
	while(cur_frame->sp)
		freeOperand(pop(cur_frame));

	Operand *ex_opr = NULL;
	const ClassFile *thrcls = findClass(thread, name);

	if (thrcls == NULL) {
		if(!strcmp(name, "java/lang/RuntimeException")) 
			errx(EXIT_FAILURE, 
					"Cannot find java/lang/RuntimeException in throw");
		else 
			return throw(thread, "java/lang/RuntimeException", name, pc);
	}

	const Object *thr = newObject(thread, thrcls);
	
	push(cur_frame, newOperand(TYPE_OREF, thr));
		
	if (text) {
		Object *str = NULL;
		if ((ex_opr = newString(thread, text, &str)) != NULL)
			return ex_opr;

		push(cur_frame, newOperand(TYPE_OREF,str));
	}

	const method_info *mi = findMethodByClass(thread, thrcls, 
			"<init>", text ? "(Ljava/lang/String;)V" : "()V");
	if ((ex_opr = invokeMethod(thread, mi, true, pc)) != NULL)
		return ex_opr;

	push(cur_frame, newOperand(TYPE_OREF, thr));
	mi = findMethodByClass(thread, thrcls,
			"fillInStackTrace", "()Ljava/lang/Throwable;");
	if ((ex_opr = invokeMethod(thread, mi, true, pc)) != NULL)
		return ex_opr;

	freeOperand(pop(cur_frame));

	return (Operand *)newOperand(TYPE_OREF, thr);
}

static bool isClassLoaded(const Thread *thread, const char *clsname)
{
	JVM *jvm = (JVM *)thread->jvm;

	if (pthread_rwlock_rdlock(&jvm->rwlock))
		errx(EXIT_FAILURE, "isClassLoaded: pthread_rwlock_rdlock");

	for (ssize_t i = 0; i < MAX_STACK; i++)
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
static const ClassFile *findClass2(Thread *thread, const char *clsname, 
		const bool load)
{
	const ClassFile *ret = NULL;
	JVM *jvm = (JVM *)thread->jvm;

	if (!jvm) return NULL;

	if (pthread_rwlock_rdlock(&jvm->rwlock))
		errx(EXIT_FAILURE, "findClass: pthread_rwlock_rdlock");
	for (ssize_t i = 0; i < MAX_STACK; i++)
	{
		if (!jvm->method_area[i])
			continue;
		if (!strcmp(jvm->method_area[i]->this_class->d.class.name->d.utf8.ascii, clsname)) {
			ret = jvm->method_area[i];
			break;
		}
	}
	if (pthread_rwlock_unlock(&jvm->rwlock))
		errx(EXIT_FAILURE, "findClass: pthread_rwlock_unlock");

	if (ret && !ret->isLinked) {
		warnx("findClass2: refusing to return %s as not linked", clsname);
		__asm__("int $3");
		return NULL;
	}

	if (!ret && load) {
		ClassFile *tmp = NULL;
		char buf[BUFSIZ];
		snprintf(buf, sizeof(buf), "%s.class", clsname);
		tmp = loadClass(thread, buf);
		if (!tmp) return NULL;
		if (thread->jvm->VM && !linkClass(thread, tmp)) return NULL;
		ret = tmp;
	}
	return ret;
}

static bool canCast(Thread *thread, const char *from, const char *to)
{
	if (!strcmp(from, to)) return true;
	if (!strcmp("java/lang/Object", to)) return true;


	const ClassFile *fromCls = findClass(thread, from);
	const ClassFile *toCls = findClass(thread, to);

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
static const ClassFile *findClass(Thread *thread, const char *clsname)
{
	assert(thread->jvm->VM == true);
	return findClass2(thread, clsname, true);
}

static const method_info *findMethodByClass(Thread *thread, 
		const ClassFile *cls, const char *mthname, const char *desc)
{
	//bool print = !strcmp(mthname, "add");
	if (cls == NULL)
		cls = findClass(thread, "java/lang/Object");
	//if (print) printf(" >> inside %s\n", cls->this_class->name->ascii);
	for (ssize_t i = 0; i < cls->methods_count; i++)
	{
		//if (print) {
		//	printf("    compare %s to %s && \n", cls->methods[i]->name->ascii, mthname);
		//	printf("     compare %s to %s\n", cls->methods[i]->descriptor->ascii, desc);
		//}
		if (!strcmp(cls->methods[i]->name->d.utf8.ascii, mthname) &&
				!strcmp(desc, cls->methods[i]->descriptor->d.utf8.ascii)) {
			//if (print) printf("    winner\n");
			return cls->methods[i];
		}
	}

	if (cls->interfaces_count > 0) {
		const method_info *mi = NULL;
		for (ssize_t i = 0; i < cls->interfaces_count; i++) {
			if ((mi = findClassMethod(thread, 
							cls->interface_class[i]->d.class.name->d.utf8.ascii, mthname, 
							desc)) != NULL)
				return mi;
		}
	} if (cls->super_name)
		return findClassMethod(thread, cls->super_name, mthname, desc);
	else
		return NULL;
}

static const method_info *findClassMethod(Thread *thread, const char *clsname, 
		const char *mthname, const char *desc)
{
	assert(thread->jvm->VM == true);
	//printf("findClassMethod: %s %s %s\n", clsname, mthname, desc);
	const ClassFile *cls = findClass(thread, clsname);
	if (cls == NULL) return NULL;

	return findMethodByClass(thread, cls, mthname, desc);
}

static const field_info *findClassField(Thread *thread, const char *clsname, 
		const char *fldname)
{
	assert(thread->jvm->VM == true);
	//printf("findClassField: %s %s\n", clsname, fldname);
	const ClassFile *cls = findClass(thread, clsname);
	if (cls == NULL) {
		warnx("findClassField: no such class %s", clsname);
		return NULL;
	}

	return findClassFieldForClass(thread, cls, fldname);
}

static const field_info *findClassFieldForClass(Thread *thread, 
		const ClassFile *cls, const char *fldname)
{
	for (ssize_t i = 0; i < cls->fields_count; i++) {
		if (!strcmp(cls->fields[i]->name->d.utf8.ascii, fldname)) {
			return cls->fields[i];
		}
	}

	if(cls->super_class)
		return findClassField(thread, cls->super_name, fldname);
	return NULL;
}

static const attribute_info *findAttribute(const attribute_info **attrs, 
		const uint16_t num, const char *name)
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
	const method_info *mi = NULL;
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
				const attribute_info *ai = cls->fields[i]->attributes[j];

				if (ai->d.constant->op.type == TYPE_OREF) {
					const field_info *fi = cls->fields[i];

					if (ai->d.constant->op.val.vref == NULL) {
						Object *tmp = NULL;
						const cp_String_info *si = &ai->d.constant->entry->d.str;
						if ((ex_opr = newStringFromChar(thread, si->intern, 
										&tmp)) != NULL)
							return ex_opr;
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
		ret = invokeMethod(thread, mi, false, 0);	
	else
		ret = NULL;

	pthread_rwlock_unlock(&cls->rwlock);

	return ret;
}

static Operand *getFieldInfo(Thread *thread, const ClassFile *cls, 
		uint16_t which, const field_info **fld)
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

	const cp_info *cp = cls->constant_pool[which];
	const field_info *fi = findClassField(thread, 
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

static inline uint16_t readShort(const uint8_t *code, int64_t *pc)
{
	const uint16_t ret = (uint16_t)(256U * code[(*pc)+1] + code[(*pc)+2]); 
	*pc += 2;
	return ret;
}

static inline int32_t readInt(const uint8_t *code, int64_t *pc)
{
	const int32_t ret = (int32_t)(16777216U * code[(*pc)] + 65536U * code[(*pc)+1] + 256U * code[(*pc)+2] + code[(*pc)+3]);
	*pc += 4;
	return ret;
}

static inline int16_t readBranch(const uint8_t *code, int64_t *pc, 
		const int16_t off)
{
	const int16_t bra = (int16_t)(256U * code[(*pc)+1] + code[(*pc)+2]); 
	*pc += 2;
	return (int16_t)(bra - 3);
}

static Operand *ldc(int idx, Frame *cur_frame, const ClassFile *cls, 
		Thread *thread, const int64_t pc)
{
	const cp_info *cp = cls->constant_pool[idx];
	Operand *ex_opr = NULL;

	switch(cp->tag)
	{
		case CONSTANT_String:
			{
				const ClassFile *strcls;
				const Object *o = newObject(thread,
						strcls = findClass(thread, 
							"java/lang/String"));

				push(cur_frame, newOperand(TYPE_OREF, o));
				push(cur_frame, 
						newOperand(TYPE_AREF, 
							cp->d.str.intern));

				const method_info *meth = 
					findMethodByClass(thread, 
							strcls, "<init>", "([C)V");

				if (meth == NULL)
					errx(EXIT_FAILURE, "ldc: noSuchMethod");

				if((ex_opr = invokeMethod(thread, meth, 
								true, pc)) != NULL)
					return ex_opr;

				push(cur_frame, newOperand(TYPE_OREF, o));
			}
			break;
		case CONSTANT_Class:
			{
#ifdef DEBUG
				printf("%s", cp->d.class.name->d.utf8.ascii);
#endif
				const ClassFile *cls = findClass(thread, cp->d.class.name->d.utf8.ascii);
				if (!cls || !getClassObject(thread, (ClassFile *)cls)) 
					return throw(thread, "java/lang/ClassNotFoundException", ERR_AT, pc);

				push(cur_frame, newOperand(TYPE_OREF, getClassObject(thread, (ClassFile *)cls)));
			}
			break;
		case CONSTANT_Float:
			{
#ifdef DEBUG
				printf("%f", cp->d.tfloat.c_float);
#endif
				push(cur_frame, newOperand(TYPE_FLOAT,
							&cp->d.tfloat.c_float));
			}
			break;
		case CONSTANT_Integer:
			{
				push(cur_frame, newOperand(TYPE_INT,
							&cp->d.tint.c_int));
			}
			break;
		case CONSTANT_Long:
			{
				push(cur_frame, newOperand(TYPE_LONG,
							&cp->d.tlong.c_long));
			}
			break;
		default:
			errx(EXIT_FAILURE, 
					"ldc: unknown constant pool tag: %d", 
					cp->tag);
			break;
	}
	return NULL;
}

static Operand *runCode(Thread *thread, const Code_attribute *attr, const int32_t pc_mod, int64_t *pcstore)
{
	int64_t pc		= 0 + pc_mod;
	uint16_t tmpw	= 0;
	uint8_t tmp		= 0;
	Operand *ex_opr	= NULL;
	bool running	= true;

	const uint8_t *code		= attr->code;
	const ClassFile *cls	= thread->cur_method->class;

	Operand tmpOp;

#ifdef DEBUG
	printf(ANSI_BMAG"(inside %s.%s)\n"ANSI_RESET, 
			cls->this_class->d.class.name->d.utf8.ascii,
			thread->cur_method->name->d.utf8.ascii
		  );
	//dumpFrame(currentFrame(thread));
#endif

	if (!cls->isInit) {
		Operand *ret = classInit(thread, (ClassFile *)cls);
		if (ret != NULL) return ret;
	}

	while(running)
	{
		if (pc < 0 || pc > attr->code_length) {
			warnx("runCode: %ld: invalid program counter", pc);
			__asm__("int $3");
		}

		Frame *cur_frame = currentFrame(thread);
		const uint8_t opc = code[pc];
#ifdef DEBUG
		printf("%3ld: [%2x] ", pc, opc);
#endif
		if (pcstore)
			*pcstore = pc;
		cur_frame->pc = (uint32_t)(pc);

		switch(opc)
		{
			case 0x00: /* nop */
				break;
			case 0x01: /* aconst_null */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " aconst_null\n" ANSI_RESET);
#endif
					push(cur_frame, newOperand(TYPE_NULL, NULL));
				}
				break;
			case 0x02: /* iconst_m1 */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " iconst_m1\n" ANSI_RESET);
#endif
					tmpOp.val.vint = -1;
					push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
				}
				break;
			case 0x03: /* iconst 0 */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " iconst_0\n" ANSI_RESET);
#endif
					tmpOp.val.vint = 0;
					push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
				}
				break;
			case 0x04: /* iconst 1 */
				{
#ifdef DEBUG
					printf(ANSI_INSTR" iconst_1\n" ANSI_RESET);
#endif
					tmpOp.val.vint = 1;
					push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
				}
				break;
			case 0x05: /* iconst 2 */
				{
#ifdef DEBUG
					printf(ANSI_INSTR" iconst_2\n" ANSI_RESET);
#endif
					tmpOp.val.vint = 2;
					push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
				}
				break;
			case 0x06: /* iconst 3 */
				{
#ifdef DEBUG
					printf(ANSI_INSTR" iconst_3\n" ANSI_RESET);
#endif
					tmpOp.val.vint = 3;
					push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
				}
				break;
			case 0x07: /* iconst 4 */
				{
#ifdef DEBUG
					printf(ANSI_INSTR" iconst_4\n" ANSI_RESET);
#endif
					tmpOp.val.vint = 4;
					push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
				}
				break;
			case 0x08: /* iconst 5 */
				{
#ifdef DEBUG
					printf(ANSI_INSTR" iconst_5\n" ANSI_RESET);
#endif
					tmpOp.val.vint = 5;
					push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
				}
				break;
			case 0x09: /* lconst 0 */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " lconst_0\n" ANSI_RESET);
#endif
					tmpOp.val.vlong = 0;
					push(cur_frame, newOperand(TYPE_LONG, &tmpOp.val.vlong));
				}
				break;
			case 0x0a: /* lconst 1 */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " lconst_1\n" ANSI_RESET);
#endif
					tmpOp.val.vlong = 1;
					push(cur_frame, newOperand(TYPE_LONG, &tmpOp.val.vlong));
				}
				break;
			case 0x0b: /* fconst_0 */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " fconst_0 " ANSI_RESET "\n");
#endif
					tmpOp.val.vfloat = 0.0f;
					push(cur_frame, newOperand(TYPE_FLOAT, &tmpOp.val.vfloat));
				}
				break;
			case 0x0c: /* fconst_1 */
				{
#ifdef	DEBUG
					printf(ANSI_INSTR " fconst_1 " ANSI_RESET "\n");
#endif
					tmpOp.val.vfloat = 1.0f;
					push(cur_frame, newOperand(TYPE_FLOAT, &tmpOp.val.vfloat));
				}
				break;
			case 0x0d: /* fconst_2 */
				{
#ifdef	DEBUG
					printf(ANSI_INSTR " fconst_2 " ANSI_RESET "\n");
#endif
					tmpOp.val.vfloat = 2.0f;
					push(cur_frame, newOperand(TYPE_FLOAT, &tmpOp.val.vfloat));
				}
				break;
			case 0x0e: /* dconst_0 */
				{
#ifdef	DEBUG
					printf(ANSI_INSTR " dconst_0 " ANSI_RESET "\n");
#endif
					tmpOp.val.vdouble = 0.0f;
					push(cur_frame, newOperand(TYPE_DOUBLE, &tmpOp.val.vdouble));
				}
				break;
			case 0x0f: /* dconst_1 */
				{
#ifdef	DEBUG
					printf(ANSI_INSTR " dconst_1 " ANSI_RESET "\n");
#endif
					tmpOp.val.vdouble = 1.0f;
					push(cur_frame, newOperand(TYPE_DOUBLE, &tmpOp.val.vdouble));
				}
				break;
			case 0x10: /* bipush */
				{
					const uint32_t big = code[++pc];
#ifdef DEBUG
					printf(ANSI_INSTR " bipush " ANSI_RESET "%d\n", big);
#endif
					push(cur_frame, newOperand(TYPE_INT, &big));
				}
				break;
			case 0x11: /* sipush */
				{
					tmpw = readShort(code, &pc);
#ifdef DEBUG					
					printf(ANSI_INSTR " sipush " ANSI_RESET " %d\n", tmpw);
#endif
					tmpOp.val.vint = tmpw;
					push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
				}
				break;
			case 0x12: /* ldc */
				{
					tmp = code[++pc];
#ifdef DEBUG
					printf(ANSI_INSTR " ldc " ANSI_RESET "#%d ", tmp);
#endif
					Operand *ex_opr = ldc(tmp, cur_frame, cls, thread, pc);
					if (ex_opr) return ex_opr;
#ifdef DEBUG
				printf("\n");
#endif
				}
				break;
			case 0x13: /* ldc_w */
				{
					tmpw = readShort(code, &pc);
#ifdef DEBUG
					printf(ANSI_INSTR "ldc_w " ANSI_RESET "%d\n", tmpw);
#endif
					Operand *ex_opr = ldc(tmpw, cur_frame, cls, thread, pc);
					if (ex_opr) return ex_opr;
#ifdef DEBUG
					printf("\n");
#endif
				}
				break;
			case 0x14: /* ldc2_w */
				{

					//tmpw = (uint16_t)(256U * code[pc+1] + code[pc+2]); pc += 2;
					tmpw = readShort(code, &pc);
#ifdef DEBUG
					printf(ANSI_INSTR " ldc2_w " ANSI_RESET "%d\n", tmpw);
#endif

					const cp_info *cp = cls->constant_pool[tmpw];

					switch(cp->tag)
					{
						case CONSTANT_Long:
							{
								push(cur_frame, newOperand(TYPE_LONG, &cp->d.tlong.c_long));
							}
							break;
						case CONSTANT_Double:
							{
								push(cur_frame, newOperand(TYPE_DOUBLE, &cp->d.tdouble.c_double));
							}
							break;
						default:
							errx(EXIT_FAILURE, "ldc2_w: unknown constant pool tag: %d", cp->tag);
					}
				}
				break;
			case 0x15: /* iload */
				{
					tmp = code[++pc];
#ifdef DEBUG
					printf(ANSI_INSTR " iload " ANSI_RESET "%d\n", tmp);
#endif
					if(!push(cur_frame, dupOperand(getLocal(cur_frame, tmp))))
						return throw(thread, "java/lang/VirtualMachineError", ERR_AT, pc);
				}
				break;
			case 0x16: /* lload */
				{
					tmp = code[++pc];
#ifdef DEBUG
                    printf(ANSI_INSTR " lload " ANSI_RESET "%d\n", tmp);
#endif
					if(!push(cur_frame, dupOperand(getLocal(cur_frame, tmp))))
						return throw(thread, "java/lang/VirtualMachineError", ERR_AT, pc);
				}
				break;
			case 0x17: /* fload */
				{
					tmp = code[++pc];
#ifdef DEBUG
                    printf(ANSI_INSTR " fload " ANSI_RESET "%d\n", tmp);
#endif
					if(!push(cur_frame, dupOperand(getLocal(cur_frame, tmp))))
						return throw(thread, "java/lang/VirtualMachineError", ERR_AT, pc);
				}
				break;
			case 0x18: /* dload */
				{
					tmp = code[++pc];
#ifdef DEBUG
                    printf(ANSI_INSTR " dload " ANSI_RESET "%d\n", tmp);
#endif
					if(!push(cur_frame, dupOperand(getLocal(cur_frame, tmp))))
						return throw(thread, "java/lang/VirtualMachineError", ERR_AT, pc);
				}
				break;
			case 0x19: /* aload */
				{
					tmp = code[++pc];
#ifdef DEBUG
                    printf(ANSI_INSTR " aload " ANSI_RESET "%d\n", tmp);
#endif
                    if(!push(cur_frame, dupOperand(getLocal(cur_frame, tmp))))
						return throw(thread, "java/lang/VirtualMachineError", ERR_AT, pc);
				}
				break;
			case 0x1a: /* iload_0 */
				{
					/* TODO check if int? */
#ifdef DEBUG
					printf(ANSI_INSTR " iload_0\n" ANSI_RESET);
#endif
					if(!push(cur_frame, dupOperand(getLocal(cur_frame,0))))
						return throw(thread, "java/lang/VirtualMachineError", ERR_AT, pc);
				}
				break;
			case 0x1b: /* iload_1 */
				{
					/* TODO check if int? */
#ifdef DEBUG
					printf(ANSI_INSTR" iload_1\n" ANSI_RESET);
#endif
					if(!push(cur_frame, dupOperand(getLocal(cur_frame,1))))
						return throw(thread, "java/lang/VirtualMachineError", ERR_AT, pc);
				}
				break;
			case 0x1c: /* iload_2 */
				{
					/* TODO check if int? */
#ifdef DEBUG
					printf(ANSI_INSTR" iload_2\n" ANSI_RESET);
#endif
					if(!push(cur_frame, dupOperand(getLocal(cur_frame,2))))
						return throw(thread, "java/lang/VirtualMachineError", ERR_AT, pc);
				}
				break;
			case 0x1d: /* iload_3 */
				{
					/* TODO check if int? */
#ifdef DEBUG
					printf(ANSI_INSTR" iload_3 " ANSI_RESET "%s %s\n", 
							printOpType(cur_frame->local[3]->type),
							printOpValue(cur_frame->local[3]));
#endif
					if(!push(cur_frame, dupOperand(getLocal(cur_frame,3))))
						return throw(thread, "java/lang/VirtualMachineError", ERR_AT, pc);
				}
				break;
			case 0x1e: /* lload_0 */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " lload_0 " ANSI_RESET "%s %s\n",
							printOpType(cur_frame->local[0]->type),
							printOpValue(cur_frame->local[0]));
#endif
					push(cur_frame, dupOperand(getLocal(cur_frame,0)));
				}
				break;
			case 0x1f: /* lload_1 */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " lload_1 " ANSI_RESET "%s %s\n",
							printOpType(cur_frame->local[1]->type),
							printOpValue(cur_frame->local[1]));
#endif
					push(cur_frame, dupOperand(getLocal(cur_frame,1)));
				}
				break;
			case 0x20: /* lload_2 */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " lload_2 " ANSI_RESET "%s %s\n",
							printOpType(cur_frame->local[2]->type),
							printOpValue(cur_frame->local[2]));
#endif
					push(cur_frame, dupOperand(getLocal(cur_frame,2)));
				}
				break;
			case 0x21: /* lload_3 */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " lload_3 " ANSI_RESET "%s %s\n",
							printOpType(cur_frame->local[3]->type),
							printOpValue(cur_frame->local[3]));
#endif
					push(cur_frame, dupOperand(getLocal(cur_frame,3)));
				}
				break;
			case 0x22: /* fload_0 */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " fload_0 " ANSI_RESET "%s %s\n",
							printOpType(cur_frame->local[0]->type),
							printOpValue(cur_frame->local[0]));
#endif
					push(cur_frame, dupOperand(getLocal(cur_frame,0)));
				}
				break;
			case 0x23: /* fload_1 */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " fload_1 " ANSI_RESET "%s %s\n",
							printOpType(cur_frame->local[1]->type),
							printOpValue(cur_frame->local[1]));
#endif
					push(cur_frame, dupOperand(getLocal(cur_frame,1)));
				}
				break;
			case 0x24: /* fload_2 */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " fload_2 " ANSI_RESET "%s %s\n",
							printOpType(cur_frame->local[2]->type),
							printOpValue(cur_frame->local[2]));
#endif
					push(cur_frame, dupOperand(getLocal(cur_frame,2)));
				}
				break;
			case 0x25: /* fload_3 */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " fload_3 " ANSI_RESET "%s %s\n",
							printOpType(cur_frame->local[3]->type),
							printOpValue(cur_frame->local[3]));
#endif
					push(cur_frame, dupOperand(getLocal(cur_frame,3)));
				}
				break;
			case 0x2a: /* aload_0 */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " aload_0 " ANSI_RESET " %s %s\n", 
							printOpType(cur_frame->local[0]->type),
							printOpValue(cur_frame->local[0]));
#endif
					if(!push(cur_frame, dupOperand(getLocal(cur_frame,0))))
						return throw(thread, "java/lang/VirtualMachineError", ERR_AT, pc);
				}
				break;
			case 0x2b: /* aload_1 */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " aload_1" ANSI_RESET " %s %s\n", 
							printOpType(cur_frame->local[1]->type),
							printOpValue(cur_frame->local[1]));
#endif
					if(!push(cur_frame, dupOperand(getLocal(cur_frame,1))))
						return throw(thread, "java/lang/VirtualMachineError", ERR_AT, pc);
				}
				break;
			case 0x2c: /* aload_2 */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " aload_2" ANSI_RESET " %s %s\n", 
							printOpType(cur_frame->local[2]->type),
							printOpValue(cur_frame->local[2]));
#endif
					if(!push(cur_frame, dupOperand(getLocal(cur_frame,2))))
						return throw(thread, "java/lang/VirtualMachineError", ERR_AT, pc);
				}
				break;
			case 0x2d: /* aload_3 */
				{
#ifdef DEBUG
					printf(ANSI_INSTR" aload_3" ANSI_RESET " %s %s\n", 
							printOpType(cur_frame->local[3]->type),
							printOpValue(cur_frame->local[3]));
#endif
					push(cur_frame, dupOperand(getLocal(cur_frame,3)));
				}
				break;
			case 0x32: /* aaload */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " aaload\n" ANSI_RESET);
#endif
					Operand *index = pop(cur_frame);
					Operand *array = pop(cur_frame);

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
#ifdef DEBUG
					printf(ANSI_INSTR " baload\n" ANSI_RESET);
#endif

					if (array->val.vref->data.array.data[index->val.vint])
						push(cur_frame, newOperand(TYPE_INT,
									&array->val.vref->data.array.data[index->val.vint]->val.vint));
					else {
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
#ifdef DEBUG
					printf(ANSI_INSTR " caload" ANSI_RESET " [%d] %s\n",
							index->val.vint,
							printOpType(array->val.vref->data.array.type)
							);
#endif

					if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->data.array.len)
						return throw(thread, "java/lang/IndexOutOfBoundsException", NULL, pc);

					if (array->val.vref->data.array.data[index->val.vint])
						push(cur_frame, newOperand(TYPE_INT, 
									&array->val.vref->data.array.data[index->val.vint]->val.vint));
					else {
						tmpOp.val.vint = 0;
						push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
					}

					freeOperand(index);
					freeOperand(array);
				}
				break;
			case 0x36: /* istore */
				{
					tmp = code[++pc];
					Operand *val = pop(cur_frame);
#ifdef DEBUG
					printf(ANSI_INSTR " istore "ANSI_RESET" %s.%s => %d\n", 
							printOpType(val->type),
							printOpValue(val),
							tmp);
#endif
					setLocal(cur_frame, tmp, val);
				}
				break;
			case 0x37: /* lstore */
				{
					tmp = code[++pc];
					Operand *val = pop(cur_frame);
#ifdef DEBUG
					printf(ANSI_INSTR " lstore "ANSI_RESET" %s.%s => %d\n", 
							printOpType(val->type),
							printOpValue(val),
							tmp);
#endif
					setLocal(cur_frame, tmp, val);
				}
				break;
			case 0x3a: /* astore */
				{
					tmp = code[++pc];
					Operand *val = pop(cur_frame);
#ifdef DEBUG
					printf(ANSI_INSTR " astore "ANSI_RESET"%s.%s => %d\n", 
							printOpType(val->type),
							printOpValue(val),
							tmp);
#endif
					setLocal(cur_frame, tmp, val);
				}
				break;
			case 0x3b: /* istore_0 */
				{
					Operand *a = pop(cur_frame);
#ifdef DEBUG
					printf(ANSI_INSTR" istore_0 "  ANSI_RESET);
					printf("<= %s %s\n", printOpType(a->type), printOpValue(a));
#endif
					setLocal(cur_frame,0,a);
				}
				break;
			case 0x3c: /* istore_1*/
				{
					Operand *a = pop(cur_frame);
#ifdef DEBUG
					printf(ANSI_INSTR " istore_1 "  ANSI_RESET);
					printf("<= %s %s\n", printOpType(a->type), printOpValue(a));
#endif
					setLocal(cur_frame,1,a);
				}
				break;
			case 0x3d: /* istore_2*/
				{
					Operand *a = pop(cur_frame);
#ifdef DEBUG
					printf(ANSI_INSTR" istore_2 " ANSI_RESET);
					printf("<= %s %s\n", printOpType(a->type), printOpValue(a));
#endif
					setLocal(cur_frame,2,a);
				}
				break;
			case 0x3e: /* istore_3*/
				{
					Operand *a = pop(cur_frame);
#ifdef DEBUG
					printf(ANSI_INSTR" istore_3 " ANSI_RESET);
					printf("<= %s %s\n", printOpType(a->type), printOpValue(a));
#endif
					setLocal(cur_frame,3,a);
				}
				break;
			case 0x4b: /* astore_0 */
				{
#ifdef DEBUG
					printf(ANSI_INSTR" astore_0\n" ANSI_RESET);
#endif
					setLocal(cur_frame,0,pop(cur_frame));
				}
				break;
			case 0x4c: /* astore_1 */
				{
#ifdef DEBUG
					printf(ANSI_INSTR" astore_1\n" ANSI_RESET);
#endif
					setLocal(cur_frame,1,pop(cur_frame));
				}
				break;
			case 0x4d: /* astore_2 */
				{
#ifdef DEBUG
					printf(ANSI_INSTR" astore_2\n" ANSI_RESET);
#endif
					if(setLocal(cur_frame,2,pop(cur_frame)) == NULL)
						return throw(thread, "java/lang/VirtualMachineError", ERR_AT, pc);
				}
				break;
			case 0x4e: /* astore_3 */
				{
#ifdef DEBUG
					printf(ANSI_INSTR" astore_3\n" ANSI_RESET);
#endif
					if(setLocal(cur_frame,3,pop(cur_frame)) == NULL)
						return throw(thread, "java/lang/VirtualMachineError", ERR_AT, pc);
				}
				break;
			case 0x4f: /* iastore */
				{
#ifdef DEBUG
					printf(ANSI_INSTR" iastore\n" ANSI_RESET);
#endif

					Operand *value = pop(cur_frame);
					Operand *index = pop(cur_frame);
					Operand *array = pop(cur_frame);

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
#ifdef DEBUG
					printf(ANSI_INSTR " aastore " ANSI_RESET "[%d]\n", 
							index->val.vint);
#endif
					if (value->type != TYPE_NULL && array->val.vref->data.array.type != value->type) {
						warnx("aastore: %d != %d", value->type, array->val.vref->data.array.type);
						warnx("aastore: invalid type");
						__asm__("int $3");
						return throw(thread, "java/lang/VirtualMachineError", ERR_AT, pc);
					}

					if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->data.array.len)
						return throw(thread, "java/lang/IndexOutOfBoundsException", ERR_AT, pc);

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
#ifdef DEBUG
					printf(ANSI_INSTR " bastore " ANSI_RESET "%s %s => [%d][%s]\n", 
							printOpType(value->type),
							printOpValue(value),
							index->val.vint,
							printOpType(array->val.vref->data.array.type));
#endif
					if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->data.array.len)
						return throw(thread, "java/lang/IndexOutOfBoundsException", ERR_AT, pc);

					if (value->type != TYPE_INT || (array->val.vref->data.array.type != TYPE_BYTE 
								&& array->val.vref->data.array.type != TYPE_BOOL) )
						return throw(thread, "java/lang/VirtualMachineError", ERR_AT, pc);

					tmpOp.val.vbyte = (int8_t)value->val.vint;

					if (array->val.vref->data.array.data[index->val.vint])
						freeOperand(array->val.vref->data.array.data[index->val.vint]);
					array->val.vref->data.array.data[index->val.vint] = newOperand(TYPE_BYTE, &tmpOp.val.vbyte);

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
#ifdef DEBUG
					printf(ANSI_INSTR " castore " ANSI_RESET "%s %s => [%d][%s]\n", 
							printOpType(value->type),
							printOpValue(value),
							index->val.vint,
							printOpType(array->val.vref->data.array.type));
#endif
					if (value->type != TYPE_INT || array->val.vref->data.array.type != TYPE_CHAR)
						return throw(thread, "java/lang/VirtualMachineError", ERR_AT, pc);

					if (index->val.vint < 0 || (uint32_t)(index->val.vint) > array->val.vref->data.array.len)
						return throw(thread, "java/lang/IndexOutOfBoundsException", ERR_AT, pc);

					tmpOp.val.vchar = (uint16_t)value->val.vint;

					if (array->val.vref->data.array.data[index->val.vint])
						freeOperand(array->val.vref->data.array.data[index->val.vint]);
					array->val.vref->data.array.data[index->val.vint] = newOperand(TYPE_CHAR, &tmpOp.val.vchar);
					freeOperand(array);
					freeOperand(index);
					freeOperand(value);
				}
				break;
			case 0x57: /* pop */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " pop\n" ANSI_RESET);
#endif
					Operand *op = pop(cur_frame);
					freeOperand(op);
				}
				break;
			case 0x59: /* dup */
				{
#ifdef DEBUG
					printf(ANSI_INSTR" dup\n" ANSI_RESET);
#endif
					const Operand *cur = cur_frame->stack[cur_frame->sp-1];
					const Operand *new = dupOperand(cur);
					push(cur_frame, new);
				}
				break;
			case 0x5a: /* dup_x1 */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " dup_x1\n" ANSI_RESET);
#endif
					const Operand *val1 = pop(cur_frame);
					const Operand *val2 = pop(cur_frame);
					push(cur_frame, dupOperand(val1));
					push(cur_frame, val2);
					push(cur_frame, val1);
				}
				break;
			case 0x5c: /* dup2 */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " dup2\n" ANSI_RESET);
#endif
					const Operand *val1 = pop(cur_frame);
					if (val1->type == TYPE_DOUBLE || val1->type == TYPE_LONG) {
						push(cur_frame, dupOperand(val1));
						push(cur_frame, val1);
					} else {
						const Operand *val2 = pop(cur_frame);
						push(cur_frame, dupOperand(val2));
						push(cur_frame, dupOperand(val1));
						push(cur_frame, val2);
						push(cur_frame, val1);
					}
				}
				break;
			case 0x60: /* iadd */
				{
					Operand *two = pop(cur_frame);
					Operand *one = pop(cur_frame);

					tmpOp.val.vint = one->val.vint + two->val.vint;
					Operand *res = newOperand(TYPE_INT, &tmpOp.val.vint);
#ifdef DEBUG
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
#ifdef DEBUG
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
			case 0x64: /* isub */
				{
					Operand *one = pop(cur_frame);
					Operand *two = pop(cur_frame);

					tmpOp.val.vint = two->val.vint - one->val.vint;
					Operand *res = newOperand(TYPE_INT, &tmpOp.val.vint);
#ifdef DEBUG
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
			case 0x68: /* imul */
				{
					Operand *one = pop(cur_frame);
					Operand *two = pop(cur_frame);

					tmpOp.val.vint = one->val.vint * two->val.vint;
					Operand *res = newOperand(TYPE_INT, &tmpOp.val.vint);
#ifdef DEBUG
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
			case 0x6a: /* fmul */
				{
					Operand *two = pop(cur_frame);
					Operand *one = pop(cur_frame);
#ifdef DEBUG
					printf(ANSI_INSTR " fmul" ANSI_RESET "\n");
#endif
					tmpOp.val.vfloat = one->val.vfloat * two->val.vfloat;
					push(cur_frame, newOperand(TYPE_FLOAT, &tmpOp.val.vfloat));

					freeOperand(one);
					freeOperand(two);
				}
				break;
			case 0x6c: /* idiv */
				{
					Operand *two = pop(cur_frame);
					Operand *one = pop(cur_frame);
					tmpOp.val.vint = one->val.vint / two->val.vint;
#ifdef DEBUG
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
#ifdef DEBUG
					printf(ANSI_INSTR " ldiv" ANSI_RESET " %ld/%ld=%ld\n",
							one->val.vlong, two->val.vlong,
							tmpOp.val.vlong);
#endif
					push(cur_frame, newOperand(TYPE_LONG, &tmpOp.val.vlong));
					freeOperand(one);
					freeOperand(two);
				}
				break;
			case 0x70: /* irem */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " irem\n" ANSI_RESET);
#endif
					Operand *two = pop(cur_frame);
					Operand *one = pop(cur_frame);

					int32_t value1 = one->val.vint;
					int32_t value2 = two->val.vint;

					if (value2 == 0) {
						return throw(thread, "java/lang/ArithmeticException", NULL, pc);
					}

					tmpOp.val.vint = value1 - (value1 / value2) * value2;

					push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
					freeOperand(one);
					freeOperand(two);
				}
				break;
			case 0x71: /* lrem */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " lrem\n" ANSI_RESET);
#endif
					Operand *two = pop(cur_frame);
					Operand *one = pop(cur_frame);

					int64_t value1 = one->val.vlong;
					int64_t value2 = two->val.vlong;

					if (value2 == 0) {
						return throw(thread, "java/lang/ArithmeticException", NULL, pc);
					}

					tmpOp.val.vlong = value1 - (value1 / value2) * value2;

					push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vlong));
					freeOperand(one);
					freeOperand(two);
				}
				break;
			case 0x74: /* ineg */
				{
#ifdef DEBUG
					printf(ANSI_INSTR "ineg\n" ANSI_RESET);
#endif
					Operand *one = pop(cur_frame);
					tmpOp.val.vint = -one->val.vint;
					push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
					freeOperand(one);
				}
				break;
			case 0x78: /* ishl */
				{
					Operand *two = pop(cur_frame);
					Operand *one = pop(cur_frame);
#ifdef DEBUG
					printf(ANSI_INSTR " ishl\n" ANSI_RESET);
#endif
					tmpOp.val.vint = one->val.vint << (two->val.vint & 0x1F);
					push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
					freeOperand(one);
					freeOperand(two);
				}
				break;
			case 0x7a: /* ishr */
				{
					Operand *two = pop(cur_frame);
					Operand *one = pop(cur_frame);
#ifdef DEBUG
					printf(ANSI_INSTR " ishr\n" ANSI_RESET);
#endif
					tmpOp.val.vint = one->val.vint >> (two->val.vint & 0x1F);
					push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));

					freeOperand(one);
					freeOperand(two);
				}
				break;
			case 0x7e: /* iand */
				{
#ifdef DEBUG
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
			case 0x80: /* ior */
				{
					Operand *two = pop(cur_frame);
					Operand *one = pop(cur_frame);

					tmpOp.val.vint = one->val.vint | two->val.vint;
					push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));

					freeOperand(one);
					freeOperand(two);
				}
				break;
			case 0x84: /* iinc */
				{
					tmp = code[++pc];
					const int8_t tmp2 = (int8_t)(code[++pc]);
#ifdef DEBUG
					printf(ANSI_INSTR " iinc " ANSI_RESET "#%d,%d\n", tmp, tmp2);
#endif
					cur_frame->local[tmp]->val.vint += tmp2;
				}
				break;
			case 0x85: /* i2l */
				{
#ifdef DEBUG
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
			case 0x88: /* l2i */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " l2i " ANSI_RESET "\n");
#endif
					Operand *value = pop(cur_frame);
					tmpOp.val.vint = (int32_t)value->val.vlong;
					push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vint));
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
			case 0x91: /* i2b */
				{
					Operand *val = pop(cur_frame);
					tmpOp.val.vint = (uint8_t)(val->val.vint);
#ifdef DEBUG
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
#ifdef DEBUG
					printf(ANSI_INSTR " i2c " ANSI_RESET "%d -> %u\n", val->val.vint,
							tmpOp.val.vint);
#endif
					push(cur_frame, newOperand(TYPE_INT, &tmpOp.val.vchar));
					freeOperand(val);
				}
				break;
			case 0x93: /* i2s */
				{
					Operand *val = pop(cur_frame);
					tmpOp.val.vint = (int16_t)(val->val.vint);
#ifdef DEBUG
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
#ifdef DEBUG
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
			case 0x99: /* ifeq */
				{
					const int16_t bra = readBranch(code, &pc, 3);
					Operand *o = pop(cur_frame);
#ifdef DEBUG
					printf(ANSI_INSTR " ifeq" ANSI_RESET " %s == 0 ? goto %lu\n", 
							printOpValue(o),
							pc + bra);
#endif

					switch(o->type)
					{
						case TYPE_BYTE:		if (o->val.vbyte == 0)		pc += bra; break;
						case TYPE_SHORT:	if (o->val.vshort == 0)		pc += bra; break;
						case TYPE_INT:		if (o->val.vint == 0)		pc += bra; break;
						case TYPE_LONG:		if (o->val.vlong == 0)		pc += bra; break;
						case TYPE_FLOAT:	if ((int32_t)(o->val.vfloat) == 0)	pc += bra; break;
						case TYPE_CHAR:		if (o->val.vchar == 0)		pc += bra; break;
						case TYPE_BOOL:		if (!o->val.vbool)			pc += bra; break;

						default:
							errx(EXIT_FAILURE, "runCode: ifeq: can't process type %d", o->type);
					}

					freeOperand(o);
				}
				break;
			case 0x9b: /* iflt */
				{
					const int16_t bra = readBranch(code, &pc, 3);
					Operand *o = pop(cur_frame);
#ifdef DEBUG
					printf(ANSI_INSTR " iflt" ANSI_RESET " %s < 0 ? goto %lu\n", 
							printOpValue(o),
							pc + bra);
#endif

					switch(o->type)
					{
						case TYPE_BYTE:		if (o->val.vbyte < 0)	pc += bra; break;
						case TYPE_SHORT:	if (o->val.vshort < 0)	pc += bra; break;
						case TYPE_INT:		if (o->val.vint < 0)	pc += bra; break;
						case TYPE_LONG:		if (o->val.vlong < 0)	pc += bra; break;
						case TYPE_CHAR:		if (o->val.vchar == 0)	pc += bra; break;

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
#ifdef DEBUG
					printf(ANSI_INSTR " ifge" ANSI_RESET " %s >= 0 ? goto %lu\n", 
							printOpValue(o),
							pc + bra);
#endif

					switch(o->type)
					{
						case TYPE_BYTE:		if (o->val.vbyte >= 0)	pc += bra; break;
						case TYPE_SHORT:	if (o->val.vshort >= 0)	pc += bra; break;
						case TYPE_INT:		if (o->val.vint >= 0)	pc += bra; break;
						case TYPE_LONG:		if (o->val.vlong >= 0)	pc += bra; break;
						case TYPE_FLOAT:	if (o->val.vfloat >= 0)	pc += bra; break;
						case TYPE_CHAR:		pc += bra; break;

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
#ifdef DEBUG
					printf(ANSI_INSTR " ifgt" ANSI_RESET " %s > 0 ? goto %lu\n", 
							printOpValue(o),
							pc + bra);
#endif

					switch(o->type)
					{
						case TYPE_BYTE:		if (o->val.vbyte > 0)	pc += bra; break;
						case TYPE_SHORT:	if (o->val.vshort > 0)	pc += bra; break;
						case TYPE_INT:		if (o->val.vint > 0)	pc += bra; break;
						case TYPE_LONG:		if (o->val.vlong > 0)	pc += bra; break;
						case TYPE_CHAR:		if (o->val.vchar > 0)	pc += bra; break;

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
#ifdef DEBUG
					printf(ANSI_INSTR " ifle" ANSI_RESET " %s <= 0 ? goto %lu\n", 
							printOpValue(o),
							pc + bra);
#endif

					switch(o->type)
					{
						case TYPE_BYTE:		if (o->val.vbyte <= 0)	pc += bra; break;
						case TYPE_SHORT:	if (o->val.vshort <= 0)	pc += bra; break;
						case TYPE_INT:		if (o->val.vint <= 0)	pc += bra; break;
						case TYPE_LONG:		if (o->val.vlong <= 0)	pc += bra; break;
						case TYPE_CHAR:		if (o->val.vchar == 0)	pc += bra; break;

						default:
							errx(EXIT_FAILURE, "runCode: ifle: can't process type %d", o->type);
					}

					freeOperand(o);
				}
				break;
			case 0x9a: /* ifne */
				{
					const int16_t bra = readBranch(code, &pc, 3);
					Operand *o = pop(cur_frame);
#ifdef DEBUG
					printf(ANSI_INSTR " ifne" ANSI_RESET " %s != 0 ? goto %lu\n", 
							printOpValue(o),
							pc + bra);
#endif

					switch(o->type)
					{
						case TYPE_BYTE:		if (o->val.vbyte != 0)	pc += bra; break;
						case TYPE_SHORT:	if (o->val.vshort != 0)	pc += bra; break;
						case TYPE_INT:		if (o->val.vint != 0)	pc += bra; break;
						case TYPE_LONG:		if (o->val.vlong != 0)	pc += bra; break;
						case TYPE_BOOL:		if (o->val.vbool)		pc += bra; break;
						case TYPE_CHAR:		if (o->val.vchar != 0)	pc += bra; break;

						default:
							errx(EXIT_FAILURE, "runCode: ifne: can't process type %d", o->type);
					}

					freeOperand(o);
				}
				break;
			case 0x9f: /* if_icmpeq */
				{
					const int16_t bra = readBranch(code, &pc, 3);

					Operand *two = pop(cur_frame);
					Operand *one = pop(cur_frame);

					if (one->val.vint == two->val.vint)
						pc += bra;
#ifdef DEBUG
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

					if (one->val.vint != two->val.vint)
						pc += bra;
#ifdef DEBUG
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

#ifdef DEBUG
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

					if (one->val.vint >= two->val.vint)
						pc += bra;
#ifdef DEBUG
					printf(ANSI_INSTR " if_icmpge" ANSI_RESET " %d >= %d ? goto %ld\n",
							one->val.vint, two->val.vint,
							pc + bra);
#endif

					freeOperand(one);
					freeOperand(two);
				}
				break;
			case 0xa3: /* if_icmpgt */
				{
					const int16_t bra = readBranch(code, &pc, 3);
					Operand *two = pop(cur_frame);
					Operand *one = pop(cur_frame);

					if (one->val.vint > two->val.vint)
						pc += bra;
#ifdef DEBUG
					printf(ANSI_INSTR " if_icmpgt" ANSI_RESET " %d > %d ? goto %ld\n",
							one->val.vint, two->val.vint,
							pc+bra);
#endif

					freeOperand(one);
					freeOperand(two);
				}
				break;
			case 0xa4: /* if_icmple */
				{
					int16_t bra = readBranch(code, &pc, 3);

					Operand *two = pop(cur_frame);
					Operand *one = pop(cur_frame);

#ifdef DEBUG
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

#ifdef DEBUG
					printf(ANSI_INSTR " if_acmpeq" ANSI_RESET " ref == ref ? goto %lu\n", 
							pc + bra);
#endif

					if (one->type == two->type &&
							one->val.vref == two->val.vref)
						pc += bra;

					freeOperand(one);
					freeOperand(two);
				}
				break;
			case 0xa7: /* goto */
				{
					int16_t bra = readBranch(code, &pc, 3);
#ifdef DEBUG
					printf(ANSI_INSTR " goto " ANSI_RESET " %d:%lu\n", bra, pc + bra);
#endif
					pc += bra;
				}
				break;
			case 0xab: /* lookupswitch */
				{
					const int64_t pc_save = pc++;
					const int64_t pad = 4 - (pc % 4); 
					pc += pad;
					int32_t def = readInt(code, &pc);
					const int32_t npairs = readInt(code, &pc);
					const Operand *v = pop(cur_frame);
#ifdef DEBUG	
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
#ifdef DEBUG
					printf(ANSI_INSTR " ireturn\n" ANSI_RESET);
#endif
					Operand *o = pop(cur_frame);
					cur_frame->ret = o;
					running = false;
				}
				break;
			case 0xad: /* lreturn */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " lreturn\n" ANSI_RESET);
#endif
					Operand *o = pop(cur_frame);
					cur_frame->ret = o;
					running = false;
				}
				break;
			case 0xae: /* freturn */
				{
#ifdef DEBUG
					printf(ANSI_INSTR "freturn\n" ANSI_RESET);
#endif
					Operand *o = pop(cur_frame);
					cur_frame->ret = o;
					running = false;
				}
				break;
			case 0xb0: /* areturn */
				{
					Operand *o = pop(cur_frame);
#ifdef DEBUG
					printf(ANSI_INSTR " areturn " ANSI_RESET);
					printf("%s %s\n", printOpType(o->type), printOpValue(o));
#endif
					cur_frame->ret = o;
					running = false;
				}
				break;
			case 0xb1: /* return */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " return\n" ANSI_RESET);
#endif
					running = false;
				}
				break;
			case 0xb2: /* getstatic */
				{
					//tmpw = (uint16_t)(256U * code[pc+1] + code[pc+2]); pc += 2;
					tmpw = readShort(code, &pc);
#ifdef DEBUG
					printf(ANSI_INSTR " getstatic" ANSI_RESET " #%d ", tmpw);
#endif
					const field_info *f = NULL;
					if ((ex_opr = getFieldInfo(thread, cls, tmpw, &f)) != NULL)
						return ex_opr;
					if (f == NULL)
						errx(EXIT_FAILURE, "getstatic: can't find field");
#ifdef DEBUG
					printf("got field_info '%s' '%s' ", 
							f->name->d.utf8.ascii, 
							f->descriptor->d.utf8.ascii
						  );
#endif
					if (f->static_operand) {
#ifdef DEBUG
						printf("contents %s %s\n", 
								printOpType(f->static_operand->type),
								printOpValue(f->static_operand));
#endif
						push(cur_frame, dupOperand(f->static_operand));
					} else {
						push(cur_frame, newOperand(TYPE_NULL, NULL));
					}
				}
				break;
			case 0xb3: /* putstatic */
				{
					//tmpw = (uint16_t)(256U * code[pc+1] + code[pc+2]); pc += 2;
					tmpw = readShort(code, &pc);
					field_info *f = NULL;
					if ((ex_opr = getFieldInfo(thread, cls, tmpw, (const field_info **)&f)) != NULL)
						return ex_opr;

#ifdef DEBUG
					printf(ANSI_INSTR " putstatic" ANSI_RESET " #%d ", tmpw);
#endif
					if (f == NULL)
						errx(EXIT_FAILURE, "putstatic: no field");
#ifdef DEBUG
					printf("got field_info '%s' '%s'\n",
							f->name->d.utf8.ascii,
							f->descriptor->d.utf8.ascii
						  );
#endif
					Operand *newop = pop(cur_frame);

					if (f->static_operand)
						freeOperand(f->static_operand);
					f->static_operand = newop;
				}
				break;
			case 0xb4: /* getfield */
				{
					//tmpw = (uint16_t)(256U * code[pc+1] + code[pc+2]); pc += 2;
					tmpw = readShort(code, &pc);
					const field_info *fi = NULL;
					if ((ex_opr = getFieldInfo(thread, cls, tmpw, &fi)) != NULL)
						return ex_opr;
#ifdef DEBUG
					printf(ANSI_INSTR " getfield" ANSI_RESET " #%d ", tmpw);
#endif
					if (fi == NULL) {
						errx(EXIT_FAILURE, "getfield: no field\n");
						break;
					}
#ifdef DEBUG
					printf("to: field_info '%s' '%s'\n",
							fi->name->d.utf8.ascii,
							fi->descriptor->d.utf8.ascii
						  );
#endif
					const Operand *o = pop(cur_frame);
					const Object *obj = o->val.vref;

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
						errx(EXIT_FAILURE, "getfield: stack full");
					freeOperand((Operand *)o);
				}
				break;
			case 0xb5: /* putfield */
				{
					//tmpw = (uint16_t)(256U * code[pc+1] + code[pc+2]); pc += 2;
					tmpw = readShort(code, &pc);
					field_info *fi = NULL;
					if ((ex_opr = getFieldInfo(thread, cls, tmpw, 
									(const field_info **)&fi)) != NULL)
						return ex_opr;
#ifdef DEBUG
					printf(ANSI_INSTR " putfield " ANSI_RESET " #%d ", tmpw);
#endif
					if (fi == NULL) {
						errx(EXIT_FAILURE, "putfield: no field");
						break;
					}
					Operand *value = pop(cur_frame);
#ifdef DEBUG
					printf(" from: %s %s", printOpType(value->type), printOpValue(value));
					printf(" to  : %s %s\n",
							fi->name->d.utf8.ascii,
							fi->descriptor->d.utf8.ascii
						  );
#endif
					Operand *o = pop(cur_frame);
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
#ifdef DEBUG
					printf(ANSI_INSTR " invokevirtual" ANSI_RESET " #%d ", tmpw);
#endif
					const cp_Fieldref_info *mi = &cls->constant_pool[tmpw]->d.fr;
#ifdef DEBUG
					printf("method_info %s %s %s\n", 
							mi->class->d.class.name->d.utf8.ascii, 
							mi->nametype->d.nt.name->d.utf8.ascii,
							mi->nametype->d.nt.descriptor->d.utf8.ascii);	
#endif
					/* used for num meths only */
					const method_info *meth = findClassMethod(thread,
							mi->class->d.class.name->d.utf8.ascii,
							mi->nametype->d.nt.name->d.utf8.ascii,
							mi->nametype->d.nt.descriptor->d.utf8.ascii);

					if (meth == NULL)
						return throw(thread, "java/lang/ClassNotFoundException",
								mi->class->d.class.name->d.utf8.ascii, pc);

					const Operand *op_obj = cur_frame->stack[cur_frame->sp - meth->desc.num_params - 1];

					if (op_obj->type == TYPE_NULL)
						return throw(thread, "java/lang/NullPointerException", 
								"invokevirtual: object is null: " ERR_AT, pc);

					Object *this = op_obj->val.vref;

					meth = findMethodByClass(thread,
							this->class, 
							mi->nametype->d.nt.name->d.utf8.ascii, 
							mi->nametype->d.nt.descriptor->d.utf8.ascii);

					if (meth == NULL) {
						warnx("findMethodByClass failed: %s %s %s", 
								this->class->this_class->d.class.name->d.utf8.ascii,
								mi->nametype->d.nt.name->d.utf8.ascii,
								mi->nametype->d.nt.descriptor->d.utf8.ascii);

						return throw(thread, "java/lang/AbstractMethodError", 
								NULL, pc);
					}

					if (!this->class) {
						Operand *ret = classInit(thread, (ClassFile *)this->class);
						if (ret != NULL) return ret;
					}

#ifdef DEBUG
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
#ifdef DEBUG
					printf(ANSI_INSTR " invokespecial" ANSI_RESET " #%d ", tmpw);
#endif
					const cp_Fieldref_info *mi = &cls->constant_pool[tmpw]->d.fr;
#ifdef DEBUG
					printf("method_info special: %s:%s %s\n",
							mi->class->d.class.name->d.utf8.ascii,
							mi->nametype->d.nt.name->d.utf8.ascii,
							mi->nametype->d.nt.descriptor->d.utf8.ascii);
#endif
					const method_info *meth = findClassMethod(thread,
							mi->class->d.class.name->d.utf8.ascii,
							mi->nametype->d.nt.name->d.utf8.ascii,
							mi->nametype->d.nt.descriptor->d.utf8.ascii);

					const Operand *op_obj = cur_frame->stack[cur_frame->sp - meth->desc.num_params - 1];

					if (op_obj->type == TYPE_NULL)
						return throw(thread, "java/lang/NullPointerException",
								"invokespecial: object is null", pc);

					Object *this = op_obj->val.vref;

					if (!this->class) {
						Operand *ret = classInit(thread, (ClassFile *)this->class);
						if (ret != NULL) return ret;
					}

					if (meth == NULL)
						return throw(thread, "java/lang/AbstractMethodError", 
								NULL, pc);
#ifdef DEBUG
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
#ifdef DEBUG
					printf(ANSI_INSTR " invokestatic" ANSI_RESET " #%d ", tmpw);
#endif
					const cp_Fieldref_info *mi = &cls->constant_pool[tmpw]->d.fr;
#ifdef DEBUG
					printf("method_info static %s:%s %s\n",
							mi->class->d.class.name->d.utf8.ascii,
							mi->nametype->d.nt.name->d.utf8.ascii,
							mi->nametype->d.nt.descriptor->d.utf8.ascii);
#endif
					/* class containing static mehtod */
					const ClassFile *class = (ClassFile *)findClass(thread, 
							mi->class->d.class.name->d.utf8.ascii);

					/* static method */
					const method_info *meth = findClassMethod(thread,
							mi->class->d.class.name->d.utf8.ascii, 
							mi->nametype->d.nt.name->d.utf8.ascii,
							mi->nametype->d.nt.descriptor->d.utf8.ascii);

					if (meth == NULL)
						return throw(thread, "java/lang/AbstractMethodError",
								ERR_AT, pc);

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
					const cp_Fieldref_info *mi = &cls->constant_pool[tmpw]->d.fr;
					pc++; // skip first field, which has a value
					pc++; // skip blank field
#ifdef DEBUG
					printf(ANSI_INSTR " invokeinterface " ANSI_RESET " #%d,%d\n", tmpw, tmp);
					printf("method_info interface %s:%s %s\n",
							mi->class->d.class.name->d.utf8.ascii,
							mi->nametype->d.nt.name->d.utf8.ascii,
							mi->nametype->d.nt.descriptor->d.utf8.ascii);

#endif
					/* interface method defintion (for param count) */
					const method_info *meth = findClassMethod(thread,
							mi->class->d.class.name->d.utf8.ascii,
							mi->nametype->d.nt.name->d.utf8.ascii,
							mi->nametype->d.nt.descriptor->d.utf8.ascii);

					if (meth == NULL) {
						warnx("invokeinterface: %s.%s%s missing",
								mi->class->d.class.name->d.utf8.ascii,
								mi->nametype->d.nt.name->d.utf8.ascii,
								mi->nametype->d.nt.descriptor->d.utf8.ascii);
						return throw(thread, "java/lang/AbstractMethodError",
								ERR_AT, pc);
					}

					const Operand *op_obj = cur_frame->stack[cur_frame->sp - meth->desc.num_params - 1];
					if (op_obj->type == TYPE_NULL)
						return throw(thread, "java/lang/NullPointerException",
								ERR_AT, pc);

					Object *this = op_obj->val.vref;

					/* actual method instance in implementing class */
					meth = findMethodByClass(thread,
							this->class,
							mi->nametype->d.nt.name->d.utf8.ascii,
							mi->nametype->d.nt.descriptor->d.utf8.ascii);
					if (meth == NULL)
						return throw(thread, "java/lang/AbstractMethodError", 
								ERR_AT, pc);

					if (!this->class) {
						Operand *ret = classInit(thread, (ClassFile *)this->class);
						if (ret != NULL) return ret;
					}

					if ((ex_opr = invokeMethod(thread, meth, true, pc)) != NULL)
						return ex_opr;

					//return NULL; // FIXME
				}
				break;
			case 0xbb: /* new */
				{
					tmpw = readShort(code, &pc);
					const cp_Class_info *ci = &cls->constant_pool[tmpw]->d.class;
#ifdef DEBUG
					printf(ANSI_INSTR " new " ANSI_RESET "#%d ", tmpw);
					printf("class %s\n", ci->name->d.utf8.ascii);
#endif
					const ClassFile *tmpcls = findClass(thread, ci->name->d.utf8.ascii);
					if (tmpcls == NULL) 
						return throw(thread, "java/lang/ClassNotFoundException", 
								ERR_AT, pc);
					Object *newObj = newObject(thread, tmpcls);
					Operand *newOp = newOperand(TYPE_OREF, newObj);

					if (!push(cur_frame, newOp))
						errx(EXIT_FAILURE, "Failed to push");
				}
				break;
			case 0xbc: /* newarray */
				{
					tmp = code[++pc];
#ifdef DEBUG
					printf(ANSI_INSTR " newarray " ANSI_RESET "#%d \n", tmp);
#endif
					Operand *cnt = pop(cur_frame);
					if (cnt == NULL || cnt->type == TYPE_NULL)
						errx(EXIT_FAILURE, "newarray: cnt is NULL");
					Object *arr = newArray(thread, atypemap[tmp], (uint32_t)cnt->val.vint, NULL);
					if (arr == NULL)
						errx(EXIT_FAILURE, "newarray: cannot make array");
					if (!push(cur_frame, newOperand(TYPE_AREF, arr)))
						errx(EXIT_FAILURE, "newarray: stack full");
					freeOperand(cnt);
				}
				break;
			case 0xbd: /* anewarray */
				{
					tmpw = readShort(code, &pc);
#ifdef DEBUG
					printf(ANSI_INSTR " anewarray " ANSI_RESET "#%d \n", tmpw);
#endif
					Operand *cnt = pop(cur_frame);
					if (cnt == NULL || cnt->type == TYPE_NULL)
						errx(EXIT_FAILURE, "anewarray: cnt is NULL");
					// FIXME store class from cp[tmpw]
					const ClassFile *class = findClass(thread, cls->constant_pool[tmpw]->d.class.name->d.utf8.ascii);
					if (class == NULL)
						return throw(thread, "java/lang/ClassNotFoundException",
								ERR_AT, pc);
					Object *arr = newArray(thread, TYPE_OREF, (uint32_t)cnt->val.vint, class);
					if (arr == NULL)
						errx(EXIT_FAILURE, "anewarray: cannot make array");
					if (!push(cur_frame, newOperand(TYPE_AREF, arr)))
						errx(EXIT_FAILURE, "anewarray: stack full");
					freeOperand(cnt);
				}
				break;
			case 0xbe: /* arraylength */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " arraylength " ANSI_RESET);
#endif
					Operand *op_array = pop(cur_frame);
					if (op_array->type == TYPE_NULL)
						return throw(thread, "java/lang/NullPointerException", ERR_AT, pc);

					Object *array = op_array->val.vref;
#ifdef DEBUG
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
#ifdef DEBUG
					printf(ANSI_INSTR " athrow " ANSI_RESET);
#endif
					Operand *ex = pop(cur_frame);
					Operand *ex_opr = NULL;
					Operand *tmpop = NULL;
				
					while(cur_frame->sp > 0 && (tmpop = pop(cur_frame)) != NULL) 
						freeOperand(tmpop);
					warnx("runCode: throw: %s", ex->val.vref->class->this_class->d.class.name->d.utf8.ascii);
					
					const method_info *mi = findMethodByClass(thread, ex->val.vref->class,
							"fillInStackTrace", "()Ljava/lang/Throwable;");

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
					const cp_Class_info *ci = &cls->constant_pool[tmpw]->d.class;
					Operand *ref = pop(cur_frame);
#ifdef DEBUG
					printf(ANSI_INSTR " checkcast " ANSI_RESET "%s >= %s\n",
							ref->val.vref->class->this_class->d.class.name->d.utf8.ascii,
							ci->name->d.utf8.ascii);
#endif
					if (ref->type != TYPE_NULL && 
							!canCast(thread, 
								ref->val.vref->class->this_class->d.class.name->d.utf8.ascii,
								ci->name->d.utf8.ascii))
						return throw(thread, "java/lang/ClassCastException", NULL, pc);
					push(cur_frame, ref);
				}
				break;
			case 0xc1: /* instanceof */
				{
					tmpw = readShort(code, &pc);
#ifdef DEBUG
					printf(ANSI_INSTR " instanceof\n" ANSI_RESET);
#endif
					Operand *ref = pop(cur_frame);
					const cp_Class_info *ci = &cls->constant_pool[tmpw]->d.class;
					bool match = ref->val.vref->class == findClass(thread, ci->name->d.utf8.ascii);

					push(cur_frame, newOperand(TYPE_BOOL, &match));

					freeOperand(ref);
				}
				break;
			case 0xc2: /* monitorenter */
				{
					Operand *lock = pop(cur_frame);
#ifdef DEBUG
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
						return throw(thread, "java/lang/IllegalMonitorStateException", buf, pc);
					}
					// FIXME list of locks in case thread dies?
					// FIXME if locked don't freeObject in gc
					freeOperand(lock);
				}
				break;
			case 0xc3: /* monitorexit */
				{
					Operand *lock = pop(cur_frame);
#ifdef DEBUG
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
						return throw(thread, "java/lang/IllegalMonitorStateException", buf, pc);
					}
					// FIXME list of locks in case thread dies?
					// FIXME if locked don't freeObject in gc
					freeOperand(lock);
				}
				break;
			case 0xc6: /* ifnull */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " ifnull\n" ANSI_RESET);
#endif
					int16_t bra = (int16_t)(256U * code[pc+1] + code[pc+2]); pc += 2;
					bra = (int16_t)(bra - 3);
					Operand *one = pop(cur_frame);
					if (one->type == TYPE_NULL)
						pc += bra;
					freeOperand(one);
				}
				break;
			case 0xc7: /* ifnonnull */
				{
#ifdef DEBUG
					printf(ANSI_INSTR " ifnonnull\n" ANSI_RESET);
#endif
					int16_t bra = readBranch(code, &pc, 3);
					Operand *one = pop(cur_frame);
					if (one->type != TYPE_NULL)
						pc += bra;
					freeOperand(one);
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
				printf("   [%2d]: %s [%s]\n", i,
						printOpType(f->local[i]->type),
						printOpValue(f->local[i]));
				if (f->local[i]->type == TYPE_LONG || 
						f->local[i]->type == TYPE_DOUBLE)
					printf("   [%2d]: --\n", i++);
			}
	} else
		printf("  local: <<none>>\n");

	if (f->num_stack) {
		if (f->sp) {
			printf("  stack:\n");
			for (uint16_t i = 0; i < f->sp; i++) {
				if (f->stack[i])
					printf("   [%2d]: %s [%s]\n", i,
							printOpType(f->stack[i]->type),
							printOpValue(f->stack[i]));
			}
		} else
		printf("  stack: <empty>\n");
	} else
		printf("  stack: <<none>>\n");
}
#endif
static Operand *invokeMethod(Thread *thread, const method_info *meth, 
		const bool withObject, int64_t old_pc)
{
	Frame *oldframe = thread->frame_free ? thread->stack[thread->frame_free-1] : NULL;
	uint32_t num_operands, num_locals;
	int64_t pc = 0;
	const Code_attribute *code = NULL;
	Operand *throw = NULL;
	Object *th_obj = NULL;

#ifdef DEBUG
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
		num_operands = meth->desc.num_params + 13U; // FIXME this should be a max(return#, params#)
		num_locals = meth->desc.num_params * 2U;
	} else if (meth->access_flags & ACC_NATIVE) {
		if (!meth->nativeMethod.mvirtual)
			errx(EXIT_FAILURE, "Missing virtual native method in %s %s %s", 
					meth->class->this_class->d.class.name->d.utf8.ascii,
					meth->name->d.utf8.ascii,
					meth->descriptor->d.utf8.ascii);
		num_operands = meth->desc.num_params + 13U; // FIXME this should be a max(return#, params#)
		num_locals = meth->desc.num_params * 2U;
	} else {
		const attribute_info *ai = findAttribute(meth->attributes, meth->attributes_count, "Code");
		if (!ai) errx(EXIT_FAILURE, "Missing code attribute for method (%s %s %s)", 
				meth->class->this_class->d.class.name->d.utf8.ascii,
				meth->name->d.utf8.ascii,
				meth->descriptor->d.utf8.ascii);

		if ((code = ai->d.code) == NULL) 
			errx(EXIT_FAILURE, "Missing code for method (%s %s %s)",
				meth->class->this_class->d.class.name->d.utf8.ascii,
				meth->name->d.utf8.ascii,
				meth->descriptor->d.utf8.ascii);

		/* this is +x to handle situations such as createClass() */
		num_operands = code->max_stack + 3U;
		num_locals = code->max_locals * 2U;
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
#ifdef DEBUG
	printf("(invoking method %s %s)\n", meth->name->d.utf8.ascii, 
			meth->descriptor->d.utf8.ascii);
#endif
	if ((meth->access_flags & (ACC_NATIVE|ACC_STATIC)) == (ACC_NATIVE|ACC_STATIC))
		throw = meth->nativeMethod.mstatic(thread, meth->class);
	else if (meth->access_flags & ACC_NATIVE) {
		if (!withObject) 
			errx(EXIT_FAILURE, "invokeMethod: mvirtual but no withObject");
		if (!newframe->local || !newframe->local[0])
			errx(EXIT_FAILURE, "invokeMethod: missing local[0]");
		if (newframe->local[0]->type != TYPE_OREF && newframe->local[0]->type != TYPE_AREF)
			errx(EXIT_FAILURE, "invokeMethod: not an Object" ERR_AT);
		throw = meth->nativeMethod.mvirtual(thread, meth->class, 
				newframe->local[0]->val.vref);
	} else
		throw = runCode(thread, code, 0, &pc);

redo:
#ifdef DEBUG
	//dumpFrame(newframe);
	printf(ANSI_BRED "(done method %s %s%s)"ANSI_RESET"\n", 
			meth->name->d.utf8.ascii, 
			meth->descriptor->d.utf8.ascii,
			throw ? " [throw Object]" : "");
#endif

	if (throw) {
		th_obj = throw->val.vref;
#ifdef DEBUG
		printf("(method threw - %s %s)\n", printObject(th_obj), th_obj->class->this_class->d.class.name->d.utf8.ascii);
#endif

		if (meth->code && meth->code->exception_table_length) {
			for (int i = 0; i < meth->code->exception_table_length; i++)
			{
				const exception *ex = meth->code->exception_table[i];

				if(pc < ex->start_pc || 
						pc > ex->end_pc ||
						strcmp(th_obj->class->this_class->d.class.name->d.utf8.ascii, 
							ex->catch_type->d.class.name->d.utf8.ascii))
					continue;

#ifdef DEBUG
				printf("(handler match)\n");
#endif
				while(newframe->sp) freeOperand(pop(newframe));
				push(newframe, newOperand(TYPE_OREF, th_obj));
				throw = runCode(thread, code, ex->handler_pc, &pc);
				goto redo;
			}
		}
		freeOperand(throw);
	} else
		th_obj = NULL;

	/* if the method returns something, push it into the previous frame */
	if (oldframe && !th_obj) switch(meth->desc.ret.baseType) {
		case TYPE_VOID: 
			break;
		default: 
			{
				if (newframe->ret) {
#ifdef DEBUG
					printf("(pushing return %s %s)\n",
							printOpType(newframe->ret->type),
							printOpValue(newframe->ret)
						  );
#endif
					push(oldframe, dupOperand(newframe->ret));
					freeOperand(newframe->ret);
					newframe->ret = NULL;
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

#ifdef DEBUG
	if (old_meth) {
		printf(ANSI_BMAG "(back to %s.%s %s)\n" ANSI_RESET, 
				old_meth->class->this_class->d.class.name->d.utf8.ascii,
				old_meth->name->d.utf8.ascii,
				old_meth->descriptor->d.utf8.ascii);
	} else
		printf(ANSI_BYEL "(no parent method)\n" ANSI_RESET);
#endif

	if (th_obj) {
#ifdef DEBUG
		printf("(returning with thrown object %s)\n", printObject(th_obj));
#endif
		return newOperand(TYPE_OREF, th_obj);
	} else
		return NULL;
}

static bool addClass(Thread *thread, const ClassFile *cf)
{
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

	for (; jvm->method_area[ptr] && ptr < MAX_STACK; ptr++) ;

	if (ptr == MAX_STACK)
		errx(EXIT_FAILURE, "addClass: method_area full");

	((JVM *)jvm)->method_area[ptr] = cf;
	((ClassFile *)cf)->heaploc = ptr;

	if (pthread_rwlock_unlock(&jvm->rwlock))
		errx(EXIT_FAILURE, "addClass: pthread_rwlock_unlock");

	//printf(ANSI_BYEL "addClass: %s\n" ANSI_RESET, cf->this_class->d.class.name->d.utf8.ascii);

	return true;
}

static Operand *dupOperand(const Operand *old)
{
	if (old == NULL) return NULL;

	Operand *new = malloc(sizeof(Operand));

	if (!new) 
		errx(EXIT_FAILURE, "dupOperand");

	memcpy(new, old, sizeof(Operand));

	if (new->type == TYPE_OREF || new->type == TYPE_AREF) {
		if (new->val.vref == NULL)
			return newOperand(TYPE_NULL, NULL);
		new->val.vref->lock++; // lock-- in freeOperand
	}

	return new;
}

static Operand *newOperand(const uint8_t type, const void *value)
{
	Operand *ret = calloc(1, sizeof(Operand));
	if (ret == NULL)
		err(EXIT_FAILURE, "newOperand");

	ret->type = type;

	if (value)
		switch (type)
		{
			case TYPE_BYTE:		ret->val.vbyte = *(int8_t *)value;		break;
			case TYPE_CHAR:		ret->val.vchar = *(uint16_t *)value;	break;
			case TYPE_DOUBLE:	ret->val.vdouble = *(double *)value;	break;
			case TYPE_FLOAT:	ret->val.vfloat = *(float *)value;		break;
			case TYPE_INT:		ret->val.vint = *(int32_t *)value;		break;
			case TYPE_LONG:		ret->val.vlong = *(int64_t *)value;		break;
			case TYPE_SHORT:	ret->val.vshort = *(int16_t *)value;	break;
			case TYPE_BOOL:		ret->val.vbool = *(bool *)value;		break;
			case TYPE_OREF:
								ret->val.vref = (Object *)value;
								ret->val.vref->lock++; // lock-- in freeOperand
								break;
			case TYPE_AREF:
								ret->val.vref = (Object *)value;
								ret->val.vref->lock++; // lock-- in freeOperand
								break;
			default:			
								warnx("newOperand: no idea how to create type: %x", type);
								__asm__("int $3");
								break;
		}

	return ret;
}

static void freeOperand(Operand *o)
{
	if (!o) return;
	switch(o->type)
	{
		case TYPE_AREF:
		case TYPE_OREF:
			if (o->val.vref) {
				if (o->val.vref->lock == 0)
					warnx("freeOperand: attempting to free an unlocked Object");
				else
					o->val.vref->lock--;
			}
			break;
		default:
			break;
	}
	o->type = TYPE_NULL;
	free(o);
}

static Operand *fileoutputstream_writeb(Thread *thread, const ClassFile *cls, 
		Object *this)
{
	const Frame *cur_frame = currentFrame(thread);
#ifdef DEBUG
	//dumpFrame(cur_frame);
#endif

	const Field *fld = findFieldByName(this, "fdObj");
	const Object *fdObj = fld->op->val.vref;

	fld = findFieldByName(fdObj, "fd");
	const int fd = fld->op->val.vint;

	uint8_t a = (uint8_t)(cur_frame->local[1]->val.vint & 0xFF);
	if (write(fd, &a, 1) == -1) {
		warnx("fileoutputstream_writeb");
		return throw(thread, "java/io/IOException", ERR_AT, thread->pc);
	}

	return NULL;
}

static Operand *fileoutputstream_writebarray(Thread *thread, const ClassFile *cls,
		Object *this)
{
	const Frame *cur_frame = currentFrame(thread);
#ifdef DEBUG
	//dumpFrame(cur_frame);
#endif
	const Field *fld = findFieldByName(this, "fdObj");
	const Object *array = NULL;
	const Object *fdObj = NULL;
	int fd = -1;
	int8_t *buf = NULL;

	if (fld == NULL || fld->op == NULL || fld->op->type == TYPE_NULL)
		goto fail;

	fdObj = fld->op->val.vref;
	fld = findFieldByName(fdObj, "fd");

	fd = fld->op->val.vint;
	if (fd == -1 || cur_frame->local[1] == NULL || cur_frame->local[1]->type == TYPE_NULL) {
		warnx("fileoutputstream_writeb: checks failed");
		goto fail;
	}

	array = cur_frame->local[1]->val.vref;
	if (array->data.array.len == 0)
		return NULL;

	if ((buf = malloc(array->data.array.len)) == NULL)
		goto fail;

	for (uint32_t i = 0; i < array->data.array.len; i++)
	{
		const Operand *op = array->data.array.data[i];
		if (!op)
			buf[i] = 0;
		else {
			if (op->type != TYPE_BYTE) {
				warnx("fileoutputstream_writeb: not TYPE_BYTE or NULL: %d [%d/%d]", 
						op ? op->type : -1,
						i,
						array->data.array.len);
				__asm__("int $3");
				goto fail;
			}
			buf[i] = op->val.vbyte;
		}
	}
	if (write(fd, buf, array->data.array.len) == -1) {
		warnx("fileoutputstream_writeb");
		goto fail;
	}

	free(buf);
	return NULL;
fail:
	warnx("fileoutputstream_writebarray");
	if (buf) free(buf);
	return throw(thread, "java/io/IOException", ERR_AT, thread->pc);
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

static Operand *javalangobject_getClass(Thread *thread, const ClassFile *cls, Object *this)
{
	Frame *cur_frame = currentFrame(thread);
	//printf("getClass: %s @ %p\n", this->class->this_class->d.class.name->d.utf8.ascii, this->class);
	cur_frame->ret = newOperand(TYPE_OREF, getClassObject(thread, (ClassFile *)this->class));
	//printf("getClass: ret: %p\n", cur_frame->ret->val.vref);

	return NULL;
}

static Operand *javalangobject_hashCode(Thread *thread, const ClassFile *cls, Object *this)
{
	Frame *cur_frame = currentFrame(thread);
	int32_t hash = (int32_t)((int64_t)this);
	cur_frame->ret = newOperand(TYPE_INT, &hash);
	return NULL;
}

static Operand *javalangthread_nativeStart(Thread *thread, const ClassFile *cls, Object *this)
{
	//Frame *cur_frame = currentFrame(thread);
	//Operand *op_th = cur_frame->local[0];
	Thread *newth = newThread((JVM *)thread->jvm, this);
	startThread(newth);
	return NULL;
}

static Operand *javalangthread_sleep(Thread *thread, const ClassFile *cls)
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

static Operand *javalangthread_currentThread(Thread *thread, const ClassFile *cls)
{
	Frame *cur_frame = currentFrame(thread);

	if (pthread_rwlock_rdlock(&thread->rwlock))
		err(EXIT_FAILURE, "pthread_rwlock_rdlock");

	push(cur_frame, newOperand(TYPE_OREF, thread->thread));

	pthread_rwlock_unlock(&thread->rwlock);
	return NULL;
}

static Operand *javalangreflectarray_newInstance(Thread *thread, const ClassFile *cls)
{
	return NULL;
}

static Operand *javalangsystem_exit(Thread *thread, const ClassFile *cls)
{
	Frame *cur_frame = currentFrame(thread);
	const Operand *rc_op = pop(cur_frame);
	const int rc = rc_op->val.vint;
	exit(rc); // FIXME exit other threads?
}

static Operand *javalangsystem_currentTimeMillis(Thread *thread, const ClassFile *cls)
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

static Operand *javalangclassloader_findClass(Thread *thread, const ClassFile *cls, Object *this)
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

	for(size_t i = 0; i < strlen(ascii); i++)
		if(ascii[i] == '.') ascii[i]='/';

	const ClassFile *cf = findClass(thread, ascii);
	free(ascii);

	if (cf) {
		cur_frame->ret = newOperand(TYPE_OREF, getClassObject(thread, (ClassFile *)cf));
	} else {
		cur_frame->ret = newOperand(TYPE_NULL, NULL);
	}

	return NULL;
}

static Operand *javalangclassloader_findLoadedClass(Thread *thread, const ClassFile *cls, Object *this)
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

	const ClassFile *cf = findClass2(thread, ascii, false);
	free(ascii);
	if (cf) {
		cur_frame->ret = newOperand(TYPE_OREF, getClassObject(thread, (ClassFile *)cf));
	} else {
		cur_frame->ret = newOperand(TYPE_NULL, NULL);
	}

	return NULL;
}

static Operand *javalangthrowable_fillInStackTrace(Thread *thread, const ClassFile *cls, Object *this)
{
	Frame *cur_frame = currentFrame(thread);

	const ClassFile *ste_c = findClass(thread, "java/lang/StackTraceElement");
	if (ste_c == NULL)
		return NULL;

	Field *fld = NULL;

	if ((fld = findFieldByName(this, "stackTrace")) == NULL)
		return NULL;

	if (fld->op) freeOperand(fld->op);

	Object *trace = newArray(thread, TYPE_OREF, thread->frame_free, ste_c);
	fld->op = newOperand(TYPE_AREF, trace);

	int arrpos = 0;
	const method_info *mi = findMethodByClass(thread, ste_c, "<init>", 
			"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V");
	Operand *ex_opr;
	Object *str = NULL;

	for (ssize_t sp = thread->frame_free - 2; sp >= 2; sp--, arrpos++)
	{
		Frame *fr = thread->stack[sp];
		if (fr == NULL) {
			warnx("fr == NULL @ %ld", sp);
			continue;
		}
		
		const Object *ste_o = newObject(thread, ste_c);
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

		int line = -1;

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
	.name = "java/lang/Object",
	.access = ACC_PUBLIC,
	.methods = {
		{.name = NULL}
	},
	.super = "java/lang/Object"
};

static const internalClass tmp_Class = {
	.name = "java/lang/Class",
	.access = ACC_PUBLIC,
	.methods = {
		{.name = NULL}
	},
	.super = "java/lang/Object"
};

static const internalClass tmp_String = {
	.name = "java/lang/String",
	.access = ACC_PUBLIC,
	.methods = {
		{.name = NULL}
	},
	.super = "java/lang/Object"
};
#endif

static const internalClass java_lang_Throwable = {
	.name = "java/lang/Throwable",
	.access = ACC_PUBLIC,
	.methods = {
		{
			.name = "fillInStackTrace",
			.desc = "()Ljava/lang/Throwable;",
			.access = ACC_PUBLIC|ACC_NATIVE,
			.meth.mvirtual = javalangthrowable_fillInStackTrace
		},
		{.name = NULL}
	}
};

static const internalClass java_lang_Object = {
	.name = "java/lang/Object",
	.access = ACC_PUBLIC,
	.methods = {
		{
			.name = "hashCode",
			.desc = "()I",
			.access = ACC_PUBLIC|ACC_NATIVE,
			.meth.mvirtual = javalangobject_hashCode
		},
		{
			.name = "getClass",
			.desc = "()Ljava/lang/Class;",
			.access = ACC_PUBLIC|ACC_NATIVE,
			.meth.mvirtual = javalangobject_getClass
		},
		{.name = NULL}
	}
};

static const internalClass java_io_FileOutputStream = {
	.name = "java/io/FileOutputStream",
	.access = ACC_PUBLIC,
	.methods = {
		{
			.name = "write",
			.desc = "(I)V",
			.access = ACC_PUBLIC|ACC_NATIVE,
			.meth.mvirtual = fileoutputstream_writeb
		},
		{
			.name = "write",
			.desc = "([B)V",
			.access = ACC_PUBLIC|ACC_NATIVE,
			.meth.mvirtual = fileoutputstream_writebarray
		},
		{.name = NULL}
	}
};

static const internalClass java_lang_Thread = {
	.name = "java/lang/Thread",
	.access = ACC_PUBLIC|ACC_FINAL,
	.methods = {
		{
			.name = "nativeStart",
			.desc = "()V",
			.access = ACC_PUBLIC|ACC_NATIVE,
			.meth.mvirtual = javalangthread_nativeStart
		},
		{
			.name = "sleep",
			.desc = "(JI)V",
			.access = ACC_PUBLIC|ACC_NATIVE,
			.meth.mstatic = javalangthread_sleep
		},
		{
			.name = "currentThread",
			.desc = "()Ljava/lang/Thread;",
			.meth.mstatic = javalangthread_currentThread
		},
		{.name = NULL}
	}
};

static const internalClass java_lang_reflect_Array = {
	.name = "java/lang/reflect/Array",
	.access = ACC_PUBLIC,
	.methods = {
		{
			.name = "newInstance",
			.desc = "(Ljava/lang/Class;[I)Ljava/lang/Object;",
			.access = ACC_STATIC|ACC_PUBLIC,
			.meth.mstatic = javalangreflectarray_newInstance
		},
		{.name = NULL}
	}
};

static const internalClass java_lang_System = {
	.name = "java/lang/System",
	.access = ACC_PUBLIC|ACC_FINAL,
	.methods = {
		{
			.name = "exit",
			.desc = "(I)V",
			.access = ACC_STATIC|ACC_PUBLIC,
			.meth.mstatic = javalangsystem_exit
		},
		{
			.name = "currentTimeMillis",
			.desc = "()J",
			.access = ACC_STATIC|ACC_PUBLIC,
			.meth.mstatic = javalangsystem_currentTimeMillis
		},
		{.name = NULL}
	}
};

static const internalClass java_lang_ClassLoader = {
	.name = "java/lang/ClassLoader",
	.access = ACC_ABSTRACT|ACC_PUBLIC,
	.methods = {
		{
			.name = "findLoadedClass",
			.desc = "(Ljava/lang/String;)Ljava/lang/Class;",
			.meth.mvirtual = javalangclassloader_findLoadedClass
		},
		{
			.name = "findClass",
			.desc = "(Ljava/lang/String;)Ljava/lang/Class;",
			.meth.mvirtual = javalangclassloader_findClass
		},
		{.name = NULL}
	}
};

ClassFile *buildInternalClass(const internalClass *ic)
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

	if (ic->super == NULL && strcmp(ic->name, "java/lang/Object")) {
		super = strdup("java/lang/Object");
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
	ret->constant_pool = (const cp_info **)tmp_cp;
	return ret;
fail:
	if (ret) {
		if (tmp_fi) {
			free(tmp_fi);
		}
		if (tmp_mi) {
			free(tmp_mi);
		}
		free(ret);
	}
	warnx("Failed to process internalClass");
	return NULL;
}

#ifdef DEBUG
static void dumpConstantPool(const int me, const cp_info *cp)
{
	printf(" [%02d]: ", me);

	switch(cp->tag)
	{
		case CONSTANT_Class:
			{
				const cp_Class_info *ent = (cp_Class_info *)cp;
				printf("class:  name[%d]=%s", ent->name_index, ent->name->d.utf8.ascii);
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
						ent->descriptor_index,
						ent->descriptor->d.utf8.ascii
					  );
			}
			break;
		case CONSTANT_String:
			{
				const cp_String_info *ent = &cp->d.str;
				printf("string: utf8[%d]='%s'", ent->string_index, ent->string->d.utf8.ascii);
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
#endif

#ifdef DEBUG
static enum std_attr_e lookupPredefinedAttribute(const char *name)
{
	for (int i = 0; predefinedAttrs[i].name; i++)
	{
		if (!strcmp(name, predefinedAttrs[i].name))
			return predefinedAttrs[i].attr;
	}
	return ATTR_INVALID;
}
#endif

#ifdef DEBUG
static void dumpAttribute(const attribute_info *ai)
{
	printf("attr=%s ", ai->name ? ai->name->d.utf8.ascii : "<<missing name>>");
	switch(lookupPredefinedAttribute(ai->name->d.utf8.ascii))
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
				printAccessFlags(ai->d.inner->classes[i]->inner_class_access_flags, buf);
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
			printf("<<not supported>>");
			break;
		case ATTR_EXCEPTIONS:
			printf("number=%d\n", ai->d.exception->number_of_exceptions);
			for (int i = 0; i < ai->d.exception->number_of_exceptions; i++)
			{
				printf("   [%2d]: exception=%s\n", i, 
						ai->d.exception->exception_index_table[i]->d.class.name->d.utf8.ascii);
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
		default:
			printf("<<dump not implemented>>");
			break;
	}
	printf("\n");
}

static void dumpField(const int me, const field_info *fi)
{
	char buf[BUFSIZ];

	printf(" [%2d]: ", me);
	printAccessFlags(fi->access_flags, buf);
	printf("access=%s name=%s descriptor=%s\n",
			fi->name->d.utf8.ascii,
			fi->descriptor->d.utf8.ascii,
			buf
		  );

	for (int i = 0; i < fi->attributes_count; i++)
	{
		printf("  [%2d]: ", i);
		dumpAttribute(fi->attributes[i]);
	}
}

static void dumpMethod(const int me, const method_info *mi)
{
	printf(" [%2d]: ", me);
	char buf[BUFSIZ];
	printAccessFlags(mi->access_flags, buf);
	printf("access=%s name=%s descriptor=%s\n",
			mi->name->d.utf8.ascii,
			mi->descriptor->d.utf8.ascii,
			buf
		  );

	for (int i = 0; i < mi->attributes_count; i++)
	{
		printf("  [%2d]: ", i);
		dumpAttribute(mi->attributes[i]);
	}
}
#endif
static bool findHeapSlot(Thread *thread, Object *obj)
{
	JVM *jvm = (JVM *)thread->jvm;

	pthread_rwlock_wrlock(&jvm->rwlock);

	for(int32_t i = 0; i < MAX_STACK; i++)
		if (jvm->heap[i] == NULL) {
			jvm->heap[i] = obj;
			obj->heaploc = i;
			if (pthread_rwlock_unlock(&jvm->rwlock)) {
				abort();
				err(EXIT_FAILURE, "findHeapSlot");
			}
			return true;
		}
	if (pthread_rwlock_unlock(&jvm->rwlock)) {
		abort();
		err(EXIT_FAILURE, "findHeapSlot");
	}
	return false;
}

// FIXME common code with newObject

static Object *newMultiArray(Thread *thread, const uint8_t type, 
		uint32_t *sizes, const uint8_t dim, const ClassFile *clsType)
{
	if (type == TYPE_NULL) {
		warnx("newMultiArray: type is NULL");
		__asm__("int $3");
	}

	uint32_t size = 0;
	Object *ret = NULL;
	char buf[BUFSIZ] = {0};
	long nowt = 0;
	int rc;
	bool pti = false;

	if ((ret = calloc(1, sizeof(Object)))== NULL) {
		warn("newArray: calloc");
		goto fail;
	}

	if ((rc = pthread_mutex_init(&ret->monitor, NULL)) != 0) {
		strerror_r(rc, buf, sizeof(buf));
		warnx("newArray: pthread_mutex_init: %s", buf);
		goto fail;
	}

	pti = true;

	if (!thread) {
		ret->heaploc = -1;
		warnx("newMultiArray: no thread");
		__asm__("int $3");
	} else if (!findHeapSlot(thread, ret)) {
		warnx("newMultiArray: findHeapSlot");
		abort();
		goto fail;
	}

	ret->type = OBJ_ARRAY;
	ret->data.array.type = type;

	for (ssize_t i = 0; i < dim; i++) {
		strcat(buf, "[");
		if (sizes[i] > (1<<30)) {
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
			warn("newArray: calloc");
			goto fail;
		}

		if ( (type != TYPE_AREF) && (type != TYPE_OREF) && (type != TYPE_VOID) && (type != TYPE_NULL) )
			for (ssize_t i = 0; i < size; i++)
				ret->data.array.data[i] = newOperand(type, &nowt);
	}
#ifdef DEBUG
	printf("newMultiArray: %s %s [", 
			printOpType(type),
			clsType ?  clsType->this_class->d.class.name->d.utf8.ascii : "<n/a>");
	for (ssize_t i = 0; i < dim ; i++)
		printf("%d%s",sizes[i], i < dim-1 ? "," : "" );
	printf("]\n");
#endif
	return ret;
fail:
	free(sizes);
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
	}
	return NULL;

}

static Object *newArray(Thread *thread, const uint8_t type, const uint32_t size, const ClassFile *clsType)
{
	uint32_t *sizes = calloc(1, sizeof(uint32_t));
	if (sizes == NULL) return NULL;

	sizes[0] = size;

	return newMultiArray(thread, type, sizes, 1U, clsType);
}

static uint16_t getSuperFieldCount(const Thread *thread, const ClassFile *cls)
{
	assert(thread->jvm->VM == true);
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

static Object *newObject(Thread *thread, const ClassFile *cls)
{
	uint32_t fldptr = 0;
	const ClassFile *super = cls;
	Field *fields = NULL;
	uint16_t field_count = 0;
	bool pti = false;
	int rc;
	char buf[BUFSIZ];

	if (cls == NULL) {
		warnx("newObject: NULL class");
		return NULL;
	}

	Object *ret = calloc(1, sizeof(Object));
	if (ret == NULL) {
		warn("newObject");
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
		warn("newObject");
		goto fail;
	}

	ret->data.obj.num_fields = field_count;

	
	while(super)
	{
		for (int i = 0; i < super->fields_count; fldptr++, i++)
		{
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
							newOperand(super->fields[i]->desc.baseType, NULL);
					break;
			}
		}

		if (super->super_class) {
			super = findClass(thread, super->super_name);
		} else
			super = NULL;
	}

#ifdef DEBUG
	printf("newObject: OBJ_CLASS heap_id=%d class=%s\n", 
			ret->heaploc,
			cls->this_class->d.class.name->d.utf8.ascii);
#endif


	return ret;
fail:
	warnx("Unable to create Object from %s", cls->this_class->d.class.name->d.utf8.ascii);
	if (ret) {
		if (pti) {
			pthread_mutex_destroy(&ret->monitor);
		}
		if (ret->type == OBJ_CLASS) {
			if (ret->data.obj.fields) {
				free(ret->data.obj.fields);
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
	}
	return NULL;
}
#ifdef DEBUG
void dumpClass(const ClassFile *cf)
{
	char buf[BUFSIZ];

	printf( "Class: %s\n", cf->this_class->d.class.name->d.utf8.ascii);
	printAccessFlags(cf->access_flags, buf);
	printf( "       version:%02d.%-2d access_flags:%-30s super:%s\n",
			cf->major_version,
			cf->minor_version,
			buf,
			cf->super_class ? cf->super_class->d.class.name->d.utf8.ascii : "(none)"
		  );
	printf( "Constant Pool [%d]:\n", cf->constant_pool_count);
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
	printf( "Fields: [%d]\n", cf->fields_count);
	for (int i = 0; i < cf->fields_count; i++)
	{
		dumpField(i, cf->fields[i]);
	}
	printf( "Methods: [%d]\n", cf->methods_count);
	for (int i = 0; i < cf->methods_count; i++)
	{
		dumpMethod(i, cf->methods[i]);
	}
	printf( "Attributes: [%d]\n", cf->attributes_count);
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
	"cp/java/io",
	"cp/spod",
	"test",
	NULL
};

static ClassFile *loadClass(Thread *thread, const char *file)
{
	ClassFile *ret;
	FILE *fp;
	char buf[BUFSIZ];

	if (strchr(file, '/')) {
		snprintf(buf, sizeof(buf), "cp/%s%s", file, 
				strstr(file, ".class") != NULL ? "" : ".class");
		fp = fopen(buf, "r");
	} else {
		snprintf(buf, sizeof(buf), "%s", file);

		if ((fp = fopen(buf, "r")) == NULL) {
			for (int i = 0; search[i]; i++)
			{
				snprintf(buf, sizeof(buf), "%s/%s", search[i], file);
				if ((fp = fopen(buf, "r")) != NULL) break;
			}
		}
	}

	if (fp == NULL) {
		warnx("loadClass: no such class: %s", buf);
		return NULL;
	}

	ret = parseClass(thread, fp);
	fclose(fp);

	return ret;
}

static void freeAttribute(attribute_info *ai)
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
				free((void *)ai->d.lineno);
				break;
			case ATTR_SOURCEFILE:
				found = true;
				free((void *)ai->d.srcfile);
				break;
			case ATTR_CODE:
				found = true;
				if(ai->d.code->code) 
					free((uint8_t *)ai->d.code->code);
				if(ai->d.code->attributes) {
					for (int j = 0; j < ai->d.code->attributes_count; j++)
					{
						if (ai->d.code->attributes[j])
							freeAttribute((attribute_info *)ai->d.code->attributes[j]);
					}
					free(ai->d.code->attributes);
				}
				if(ai->d.code->exception_table) {
					for (int j = 0; j < ai->d.code->exception_table_length; j++)
					{
						if (ai->d.code->exception_table[j])
							free((void *)ai->d.code->exception_table[j]);
					}
					free(ai->d.code->exception_table);
				}
				free((void *)ai->d.code);
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
											if (ai->d.stackmap->entries[j]->i.af.locals[k])
												free(ai->d.stackmap->entries[j]->i.af.locals[k]);
										}
										free(ai->d.stackmap->entries[j]->i.af.locals);
									}
								}
								break;
							case FT_FULL_FRAME:
								{
									if (ai->d.stackmap->entries[j]->i.ff.locals)
									{
										for (int k = 0; k < ai->d.stackmap->entries[j]->i.ff.number_of_locals; k++)
										{
											if (ai->d.stackmap->entries[j]->i.ff.locals[k])
												free(ai->d.stackmap->entries[j]->i.ff.locals[k]);											
										}
										free(ai->d.stackmap->entries[j]->i.ff.locals);
									}
									if (ai->d.stackmap->entries[j]->i.ff.stack)
									{
										for (int k = 0; k < ai->d.stackmap->entries[j]->i.ff.number_of_stack_items; k++)
										{
											if (ai->d.stackmap->entries[j]->i.ff.stack[k])
												free(ai->d.stackmap->entries[j]->i.ff.stack[k]);
										}
										free(ai->d.stackmap->entries[j]->i.ff.stack);
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
					}
					free(ai->d.stackmap->entries);
				}
				free((void *)ai->d.stackmap);
				break;
			case ATTR_INNERCLASSES:
				found = true;
				if (ai->d.inner->classes) {
					for (int j = 0; j < ai->d.inner->number_of_classes; j++)
					{
						free(ai->d.inner->classes[j]);
					}
					free(ai->d.inner->classes);
				}
				free((void *)ai->d.inner);
				break;
			case ATTR_EXCEPTIONS:
				found = true;
				if (ai->d.exception->exception_index_table)
				{
					free (ai->d.exception->exception_index_table);
				}
				free((void *)ai->d.exception);
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
#ifdef DEBUG
const char *printObject(const Object *o)
{
	static char buf[BUFSIZ];
	FILE *bfp = fmemopen(buf, sizeof(buf), "w");

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
static void freeObject(JVM *jvm, Object *o)
{
	if (o->lock || (pthread_mutex_trylock(&o->monitor) != 0))
		return;

	switch (o->type)
	{
		case OBJ_CLASS:
			{
				if (o->data.obj.fields) {
					for (int i = 0; i < o->data.obj.num_fields; i++)
					{
						if(o->data.obj.fields[i].op)
							freeOperand(o->data.obj.fields[i].op);
						o->data.obj.fields[i].op = NULL;
					}
					free(o->data.obj.fields);
				}

			}
			break;
		case OBJ_ARRAY:
			{
				for(uint32_t i = 0; i < o->data.array.len; i++) {
					if(!o->data.array.data[i]) continue;
					freeOperand(o->data.array.data[i]);
					o->data.array.data[i] = NULL;
				}
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

	free(o);
}

/* caller must have wrlock on jvm */
static void freeClass(JVM *jvm, ClassFile *cf)
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

			if(fi->static_operand) {
				freeOperand(fi->static_operand);
				fi->static_operand = NULL;
			}

			if(fi->desc.className) {
				free((void *)fi->desc.className);
				fi->desc.className = NULL;
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
						if (mi->desc.params[j]->className)
							free((void *)mi->desc.params[j]->className);
						free((void *)mi->desc.params[j]);
					}
				}
				free(mi->desc.params);
				mi->desc.params = NULL;
			}

			if (mi->desc.ret.className)
				free((void *)mi->desc.ret.className);

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
				case CONSTANT_String:
					{
						cp_String_info *str = &cp->d.str;
						if (str->intern) {
							// lock++ in fixupCpString
							((Object *)str->intern)->lock--;
							freeObject(jvm, (Object *)str->intern);
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

	if (cf->heaploc != -1)
		jvm->method_area[cf->heaploc] = NULL;

	pthread_rwlock_unlock(&cf->rwlock);
	pthread_rwlock_destroy(&cf->rwlock);

	free(cf);
}

static void freeThread(Thread *t)
{
	if (t->thread)
		t->thread->lock--; // lock++ in newThread

	/* checking for missing frames (probably a bug) or the initial frame (not a bug) */
	for (uint16_t i = 0; i < t->frame_free; i++)
		if (t->stack[i]) {
			freeFrame(t->stack[i]);
			t->stack[i] = NULL;
		}

	free(t);
}

#ifdef DEBUG
static void printThreadState(enum th_state_en st, char *buf)
{
	const char *tmp = NULL;
	switch(st)
	{
		case TH_NEW:		tmp = "TH_NEW";		break;
		case TH_RUNNING:	tmp = "TH_RUNNING";	break;
		case TH_DEAD:		tmp = "TH_DEAD";	break;
		case TH_FINISHED:	tmp = "TH_FINISHED";break;
		default: tmp = "<<unknown>>"; break;
	}
	snprintf(buf, BUFSIZ, "%s", tmp);
}
#endif

static JVM *jvm = NULL;

static void freeJVM(JVM *j)
{
	int rc;
	char buf[BUFSIZ] = {0};
#ifdef DEBUG
	printf("freeJVM: start\n");
#endif

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
#ifdef DEBUG
				printThreadState(j->threads[id]->state, buf);
				printf("freeJVM: thread #%d is %s\n", id, buf);
#endif
				if (j->threads[id]->state == TH_FINISHED) {
					pthread_join(j->threads[id]->pthread, (void **)&ex_op);
					j->threads[id]->state = TH_DEAD;
					freeThread(j->threads[id]);
					j->threads[id] = NULL;
					if (ex_op) {
						Object *ex_obj = ex_op->val.vref;
						if (ex_op->type == TYPE_OREF && ex_obj && ex_obj->class) {
							warnx("Uncaught exception %s", 
									ex_obj->class->this_class->d.class.name->d.utf8.ascii);
						} else {
							warnx("Uncaught exception (null)");
						}
						freeOperand(ex_op);
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
	}

	free(ary_maps);

	for (uint16_t i = 0; i < MAX_STACK; i++)
	{
		if (j->method_area[i])
			freeClass(j, (ClassFile *)j->method_area[i]);
	}


	for (int k = 0; k < 10; k++) {
		for (uint16_t i = 0; i < MAX_STACK; i++)
		{
			if (j->heap[i]) {
#ifdef DEBUG
			if (k == 9)
				printf("Trying on attempt #9: %s\n", printObject(j->heap[i]));
#endif
				freeObject(j, j->heap[i]);
			}
		}
	}


	if (jvm == j) jvm = NULL;
	pthread_rwlock_unlock(&j->rwlock);
	pthread_rwlock_destroy(&j->rwlock);
	free(j);
}


static void cleanup(void)
{
	if(jvm) {
		freeJVM(jvm);
		jvm = NULL;
	}
	fclose(stdin);
	fclose(stdout);
	fclose(stderr);
}

static ClassFile *processNatives(Thread *thread, const internalClass *ic)
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

	for (int i = 0; i < MAX_METHODS; i++)
	{
		if (!ic->methods[i].name) continue;

		if (ic->methods[i].name == NULL || ic->methods[i].desc == NULL) {
			warnx("processNatives: missing name/desc in %s[%d]", ic->name, i);
			continue;
		}

		const method_info *mi = findMethodByClass(thread, cf, 
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


static Thread *newThread(JVM *jvm, Object *thread)
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
							"java/lang/Thread"))) == NULL) goto fail;
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
	Thread *self = (Thread *)arg;

	int rc;
	char buf[BUFSIZ] = {0};
	if ((rc = pthread_rwlock_wrlock(&self->rwlock))) {
		strerror_r(rc, buf, sizeof(buf));
		warnx("threadinit: pthread_rwlock_wrlock: %s", buf);
		return NULL;
	}
	Frame *cur_frame = currentFrame(self);
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
			if (op) {
				freeOperand(op);
				op = NULL;
			}
		}
		const uint16_t oldlocal = local;
		if (op && (op->type == TYPE_LONG || op->type == TYPE_DOUBLE)) 
			local++;
		local++;
		clearAndFreeLocal(cur_frame,oldlocal);
	}

	self->state = TH_RUNNING;
	pthread_rwlock_unlock(&self->rwlock);

	void *ret = NULL;
	if (isStatic)
		ret = invokeMethod(self, self->cur_method, false, 0);
	else
		ret = invokeMethod(self, self->cur_method, true, 0);

	if (ret) {
		while (cur_frame->sp)
			freeOperand(pop(cur_frame));

		if (!push(cur_frame, dupOperand(ret))) {
			warnx("threadinit: failed to push thrown object: %d/%d", cur_frame->sp, cur_frame->num_stack);
		} else {
			const method_info *mi = findMethodByClass(self, 
					((Operand *)ret)->val.vref->class,
					"printStackTrace", "()V");
			freeOperand(ret);
			ret = NULL;
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

static Operand *startThread(Thread *thread)
{
	Frame *cur_frame = currentFrame(thread);
	int rc;
	char buf[BUFSIZ] = {0};

	if ((rc = pthread_rwlock_wrlock(&thread->rwlock))) {
		strerror_r(rc, buf, sizeof(buf));
		errx(EXIT_FAILURE, "startThread: wrlock: %s", buf);
	}
	Object *th_obj = thread->thread;

	if (thread->cur_method == NULL) {
		const method_info *meth = findMethodByClass(thread,
				th_obj->class, "run", "()V");
		thread->cur_method = meth;
	}

	Frame *first = newFrame(
			thread->cur_method->code->max_stack + 3U, 
			umax(thread->cur_method->desc.num_params, thread->cur_method->code->max_locals) + 2U
			);
	if (first == NULL || !first->local) {
		errx(EXIT_FAILURE, "startThread: unable to create frame");
	}
	thread->stack[thread->frame_free++] = first;

	first->local[0] = newOperand(TYPE_OREF, th_obj);

	if (thread->cur_method->desc.num_params > first->num_local) {
		errx(EXIT_FAILURE, "more parameters than local slots");
	}

	if (cur_frame) for (uint16_t i = 0; i < thread->cur_method->desc.num_params; i++)
	{
		// doesnt't work for long/double
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

int main(const int ac, const char *av[])
{

	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	setvbuf(stdin, NULL, _IONBF, 0);

	if (ac < 2)
		exit(EXIT_FAILURE);

	if ((jvm = calloc(1, sizeof(JVM))) == NULL)
		err(EXIT_FAILURE, "boot: calloc");

	jvm->VM = false;

	if (pthread_rwlock_init(&jvm->rwlock, NULL))
		err(EXIT_FAILURE, "boot: JVM.rwlock");

	if (pthread_rwlock_init(&clslock, NULL))
		err(EXIT_FAILURE, "boot: clslock");

	atexit(cleanup);

	ClassFile *cf = NULL;
	Thread *parent = NULL;

	if ((parent = newThread(jvm, NULL)) == NULL)
		errx(EXIT_FAILURE, "boot: newThread");

	if (parent->thread)
		errx(EXIT_FAILURE, "boot: thread not null");

	Frame *cur_frame = parent->stack[parent->frame_free++] = newFrame(3, 2);

	/*
	addClass(parent,cf = buildInternalClass(&tmp_Object)); cf->isLinked = true;
	addClass(parent,cf = buildInternalClass(&tmp_Class)); cf->isLinked = true;
	addClass(parent,cf = buildInternalClass(&tmp_String)); cf->isLinked = true;
*/

	ClassFile *cls_String, *cls_Class, *cls_Object, *cls_ClassLoader;

	/* load classes with circular references */
	if ((cls_Object = loadClass(parent, "java/lang/Object")) == NULL) errx(1, "unable to load java/lang/Object");
	if ((cls_String = loadClass(parent, "java/lang/String")) == NULL) errx(1, "unable to load java/lang/String");
	if ((cls_Class = loadClass(parent, "java/lang/Class")) == NULL) errx(1, "unable to load java/lang/Class");
	if ((cf = loadClass(parent, "java/lang/Thread")) == NULL) errx(1, "unable to load java/lang/Thread");
	if ((cls_ClassLoader = loadClass(parent, "java/lang/ClassLoader")) == NULL) errx(1, "unable to load java/lang/ClassLoader");
	/*
	if (loadClass(parent, "java/lang/String") == NULL) errx(1, "unable to load java/langString");
	if (loadClass(parent, "java/lang/ClassLoader") == NULL) errx(1, "unable to load java/langClassLoader");
	if ((cf = loadClass(parent, "java/lang/Class")) == NULL) errx(1, "unable to load java/langClass");
	if (loadClass(parent, "java/lang/Thread") == NULL) errx(1, "unable to load java/lang/Thread");
	if (loadClass(parent, "java/io/FileOutputStream") == NULL) errx(1, "unable to load java/io/FileOutputStream");
	*/
	jvm->VM = true;

	printf("boot: VM now enabled\n");

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

	printf("boot: linking early classes\n");
	/* ensure getClass() works on them */
	for (ssize_t i = 0; i < MAX_STACK; i++)
	{
		ClassFile *ret = (ClassFile *)jvm->method_area[i];
		if (!ret)
			continue;
		if (!ret->isLinked)
			linkClass(parent, ret);
	}

	printf("boot: patching natives\n");
	/* patch natives */
	cf = processNatives(parent, &java_lang_Object);
	if (cf == NULL) exit(1);
	cf = processNatives(parent, &java_io_FileOutputStream);
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



	/* load primitive classes */
	for (ssize_t i = 0; primitiveClassMap[i].c_name; i++)
	{
		const ClassFile *cls = findClass(parent, primitiveClassMap[i].c_name);
		if (cls == NULL) {
			warnx("boot: unable to load %s", primitiveClassMap[i].c_name);
			continue;
		}
		if (!cls->isInit) {
			if (classInit(parent, (ClassFile *)cls))
				errx(EXIT_FAILURE, "boot: error thrown in %s", primitiveClassMap[i].c_name);
		}

		const field_info *fi = findClassFieldForClass(parent, cls, "TYPE");
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

	createClass(parent, cls_String);
	createClass(parent, cls_Object);
	createClass(parent, cls_Class);

	if ((parent->thread = newObject(parent, findClass2(parent, "java/lang/Thread", false))) == NULL)
		errx(EXIT_FAILURE, "boot: unable to create Thread Object");
	parent->thread->lock++;

	cf = loadClass(parent, av[1]);
	printf("boot: VM loaded\n");
	if (!cf) errx(EXIT_FAILURE, "boot: no main class");
	linkClass(parent, cf);
	
	const char *clsname = ac > 2 ? av[2] : av[1];
	const method_info *m_Main = findClassMethod(parent, clsname, "main", "([Ljava/lang/String;)V");
	if (m_Main == NULL)
		errx(EXIT_FAILURE, "boot: Cannot find method main in class %s", clsname);
	
	parent->cur_method = m_Main;

	push(cur_frame, newOperand(TYPE_AREF, 
				newArray(parent, TYPE_OREF, 0, 
					findClass(parent, "java/lang/String"))));
	
	startThread(parent);

	const struct timespec rqtp = {
		.tv_sec = 0,
		.tv_nsec = 50000000L,
	};

	nanosleep(&rqtp, NULL);
#ifdef DEBUG
	printf("\n\nMain finished\n\n");
#endif
}
