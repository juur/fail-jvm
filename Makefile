CC			:=	gcc
# Override this if you're running a non-RHEL derived version of Linux
# e.g. under Ubuntu: make JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
JAVA_HOME	:=	/usr/lib/jvm/java-1.8.0
CFLAGS		:=	\
	-std=c2x \
	-Wall \
	-Wbad-function-cast \
	-Wextra \
	-Wfloat-equal \
	-Winit-self \
	-Wformat=2 \
	-Wjump-misses-init \
	-Wlogical-op \
	-Wmissing-declarations \
	-Wmissing-format-attribute \
	-Wmissing-noreturn \
	-Wmissing-prototypes \
	-Wnested-externs \
	-Wno-unused-function \
	-Wno-unused-parameter \
	-Wold-style-definition \
	-Wpointer-arith \
	-Wredundant-decls \
	-Wstrict-aliasing=3 \
	-Wstrict-overflow=5 \
	-Wstrict-prototypes \
	-Wsuggest-attribute=cold \
	-Wsuggest-attribute=const \
	-Wsuggest-attribute=format \
	-Wsuggest-attribute=malloc \
	-Wsuggest-attribute=noreturn \
	-Wsuggest-attribute=pure \
	-Wswitch-default \
	-Wswitch-enum \
	-Wunsuffixed-float-constants \
	-Wwrite-strings \
	-fsingle-precision-constant \
	-fstack-protector-all \
	-fstrict-aliasing \
	-fstrict-overflow \
	-Og \
	-D_FORTIFY_SOURCE=2 \
	-ggdb3

LDFLAGS		:=  -lpthread -lm
JAVAC		:=	$(JAVA_HOME)/bin/javac
JVER		:=  1.7
BOOTCPATH	:=  -bootclasspath
JFLAGS		:=  -source $(JVER) -target $(JVER) $(BOOTCPATH) cp -classpath cp -Xlint:all
JLFLAGS		:=  -source $(JVER) -target $(JVER) $(BOOTCPATH) cplite -classpath cplite -Xlint:all

JCP_SRCS	:=	$(shell find cp -type f -name "*.java")
JCPL_SRCS	:=  $(shell find cplite -type f -name "*.java")

J_SRCS		:=	$(wildcard test/*.java)
J_CLSS		:=	$(J_SRCS:.java=.class)

JCP_CLSS	:=	$(JCP_SRCS:.java=.class)
JCPL_CLSS	:=	$(JCPL_SRCS:.java=.class)

SRCS		:=	$(wildcard *.c)
OBJS		:=	$(SRCS:.c=.o)
TARGET		:=	fail-jvm

SCAN_FLAGS	:=	-enable-checker \
	alpha.core.CastSize,alpha.core.SizeofPtr,alpha.core.BoolAssignment,alpha.core.FixedAddr,alpha.core.IdenticalExpr,alpha.core.PointerArithm,alpha.deadcode.UnreachableCode,alpha.security.ArrayBoundV2,alpha.security.MallocOverflow,alpha.security.ReturnPtrRange,alpha.security.taint.TaintPropagation \
	-o ~/public_html/clang \
	make

ifeq ($(DEBUG),1)
	CFLAGS += -DDEBUG
endif

ifeq ($(DEBUG_CODE),1)
	CFLAGS += -DDEBUG_CODE
endif

export JAVA_HOME

default:	all

$(TARGET):	$(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o:	%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

count:
		@cat *.c | sed -n -e '/*runCode/,$$p' | tr -s '[ \n\t]' | sed -e 's,^\t,,' | grep 'case 0x' | grep -v PARTIAL

list:
		@echo $(JCP_SRCS)

rt.jar:	$(JCP_CLSS)
	jar cf rt.jar MANIFEST.MF -C cp/ .

all:		$(TARGET) $(JCP_CLSS) $(J_CLSS) $(JCPL_CLSS) rt.jar

%.class: 	%.java
	$(JAVAC) $(JFLAGS) $<

dist-clean: clean
	find cp -type f -name "*.class" -exec rm -f {} \;

clean-tests:
	rm -f test/*.class

clean:
	rm -f $(TARGET) *.o test/*.class

scan:
	rm -f $(TARGET)
	scan-build $(SCAN_FLAGS)

.PHONY:	default clean all scan list dist-clean clean-tests

