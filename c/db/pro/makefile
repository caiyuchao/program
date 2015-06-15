#------定义程序需要的库和头文件目录------
ORA_INC=${ORACLE_HOME}/precomp/public
TUX_INC=${TUXDIR}/include
ORA_LIB=$(ORACLE_HOME)/lib
TUX_LIB=$(TUXDIR)/lib
LIB=$(HOME)/lib
TUX_PRE_LIB=$(TUXDIR)/precomp/lib
TUX_PRE_INC=$(TUXDIR)/precomp/public



#------定义Proc 命令参数和C 编译器命令参数-----------
PROC=	proc  lines=yes unsafe_null=yes dbms=v8 def_sqlcode=yes ltype=NONE
CC=	gcc -v

#CFLAGS= -I$(ORA_INC) -I$(TUX_INC)  -I$(HOME)/include
CFLAGS= -I$(HOME)/src/include -I$(ORA_INC) -I$(TUX_INC) -I$(TUX_PRE_INC) -g
CLIBS= -L$(ORA_LIB) -L$(TUX_LIB) -L/usr/lib -L$(LIB) -L$(TUX_PRE_LIB) -lclntsh -lm -lc 

#------定义文件类型编译倚赖关系------------
.SUFFIXES: .o .c .pc
.pc.c:
	$(PROC) iname=$<
.c.o:
	$(CC)  -c $(CFLAGS) $<
.pc.o:
	$(PROC) iname=$<
	$(CC) -c  $(CFLAGS) $(<:.pc=.c)


#------定义应用程序特有信息，对不同项目只需变动下面内容既可-----------------
all:test
OBJ=test.o
test: $(OBJ)
	$(CC) -o $@ $? $(CFLAGS) $(CLIBS) 
	rm -f $(OBJ)
	rm -f $@.c
	rm -f *.lis
	#rm -f tp*

clean:
	rm -rf *.o
	rm -f *.lis
	rm -f test.c
