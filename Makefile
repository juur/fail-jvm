CC			:=	gcc
JAVA_HOME	:=	/usr/lib/jvm/java-1.8.0
CFLAGS		:=	\
	-std=c99 \
	-O1 \
	-no-pie \
	-ggdb \
	-Wall \
	-Wextra \
	-pedantic \
	-Wno-unused-parameter \
	-Wformat=2 \
	-fsingle-precision-constant \
	-Wbad-function-cast \
	-Wfloat-equal -Winit-self \
	-Wjump-misses-init \
	-Wlogical-op \
	-Wmissing-declarations \
	-Wmissing-prototypes \
	-Wnested-externs \
	-Wold-style-definition \
	-Wpointer-arith \
	-Wredundant-decls \
	-Wstrict-prototypes \
	-Wstrict-aliasing=3 \
	-fstrict-aliasing \
	-Wstrict-overflow=5 \
	-fstack-protector-all \
	-fstrict-overflow \
	-D_FORTIFY_SOURCE=2 \
	-Wsuggest-attribute=format \
	-Wswitch-default \
	-Wsuggest-attribute=pure \
	-Wsuggest-attribute=const \
	-Wsuggest-attribute=noreturn \
	-Wswitch-enum \
	-Wunsuffixed-float-constants \
	-Wwrite-strings

LDFLAGS		:= -lpthread -lm
JAVAC		:=	$(JAVA_HOME)/bin/javac
JFLAGS		:=  -source 1.6 -target 1.6 -bootclasspath cp -classpath cp

JCP_SRCS	:=	$(shell find cp -type f -name "*.java")

VALGRIND	:=	valgrind
VALFLAGS	:=	--leak-check=full \
	--show-leak-kinds=all \
	--read-var-info=yes \
	--track-fds=yes \
	--track-origins=yes

J_SRCS		:=	$(wildcard test/*.java)
JCP_CLSS	:=	$(JCP_SRCS:.java=.class)
J_CLSS		:=	$(J_SRCS:.java=.class)

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

export JAVA_HOME

default:	all

$(TARGET):	$(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o:	%.c
	$(CC) $(CFLAGS) -c $<

count:
		@cat *.c | sed -n -e '/*runCode/,$$p' | tr -s '[ \n\t]' | sed -e 's,^\t,,' | grep 'case 0x'

list:
		@echo $(JCP_SRCS)

all:		$(TARGET) $(JCP_CLSS) $(J_CLSS)

%.class: 	%.java
	$(JAVAC) $(JFLAGS) $<

dist-clean: clean
	find cp -type f -name "*.class" -exec rm -f {} \;

clean:
	rm -f $(TARGET) *.o

scan:
	rm -f $(TARGET)
	scan-build $(SCAN_FLAGS)

.PHONY:	default clean all scan list

