
# set flags depending on cpu architecture
include arch.mk

V ?= 0

ifeq ($V, 1)
E =
P = @true
else
E = @
P = @echo
endif

SHELL = /bin/sh


top_srcdir	= .
srcdir		= .
prefix		= $(DESTDIR)//usr/local
exec_prefix	= $(DESTDIR)/${prefix}
bindir		= $(exec_prefix)/bin
mandir		= $(prefix)/share/man/man1
docdir		= $(prefix)/share/doc/cortex

CC		= gcc
INSTALL		= install
MKDIR		= mkdir -p

LIBS		=  -lopcodes -lbfd -liberty -lm
CFLAGS		+= -g -O2
LDFLAGS 	+=  -static
CPPFLAGS	+= 

ARCH		?= $(ARCH_x86_64)
ARCH_CFLAGS	?= $(CFLAGS_x86_64)
BFD_ARCH	?= $(BFD_ARCH_x86_64)
BFD_MACH	?= $(BFD_MACH_x86_64)

CFLAGS		+= -Isrc -Wall -Wextra -Wno-char-subscripts -Wno-unused-parameter -Wno-format
CFLAGS		+= $(ARCH_CFLAGS) -DBFD_ARCH=$(BFD_ARCH) -DBFD_MACH=$(BFD_MACH)

OBJ 		= src/cortex_elf.o \
			src/cortex_out.o \
			src/cortex_dis.o \
			src/cortex_main.o \
			src/arch/cortex_$(ARCH).o

TARGET		= cortex

all: $(TARGET)

$(TARGET): $(OBJ)
	$P '  LD       $@'
	$E $(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$P '  CC       $@'
	$E $(CC) $(CFLAGS) -c -o $@ $^

.PHONY: doc
doc:
	$P '  DOC      '
	$E doxygen doxygen.cfg

.PHONY: clean
clean:
	$P '  RM       TARGET'
	$E rm -f $(TARGET)
	$P '  RM       OBJS'
	$E find src/ -name "*.o" -exec rm -f {} \;
	$E rm -f $(HDR)

.PHONY: distclean
distclean: clean
	$P '  RM       Makefile'
	$E rm -f Makefile
	$P '  RM       doc'
	$E rm -fr doc
	$P '  RM       config.*'
	$E rm -f src/config.h config.status config.cache config.log
	$P '  RM       cache'
	$E rm -fr autom4te.cache

install:
	$P '  MKDIRS   '
	$E $(MKDIR) $(bindir)
	$E $(MKDIR) $(mandir)
	$E $(MKDIR) $(docdir)
	$P '  INSTALL  $(TARGET)'
	$E $(INSTALL) $(TARGET) $(bindir)
	$P '  INSTALL  README'
	$E $(INSTALL) README $(docdir)
	$P '  INSTALL  man'
	$E $(INSTALL) $(TARGET).1 $(mandir)

uninstall:
	$P '  UNINSTALL'
	$E rm -f $(bindir)/$(TARGET)
	$E rm -f $(mandir)/$(TARGET).1
	$E rm -f $(docdir)/README

