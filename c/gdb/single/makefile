all: a a.debug core

a.out: a.c
	gcc -g -o $@ $<

a.debug: a.out
	strip a.out --only-keep-debug -o  a.debug

a: a.out
	strip --strip-all a.out -o a

core: a
	ulimit -c unlimited; ./a

clean:
	rm -f a a.debug a.out core

#Contents of section .interp:
#Contents of section .note.ABI-tag:
#Contents of section .note.gnu.build-id:
#Contents of section .gnu.hash:
#Contents of section .dynsym:
#Contents of section .dynstr:
#Contents of section .gnu.version:
#Contents of section .gnu.version_r:
#Contents of section .rela.dyn:
#Contents of section .rela.plt:
#Contents of section .init:
#Contents of section .plt:
#Contents of section .text:
#Contents of section .fini:
#Contents of section .rodata:
#Contents of section .eh_frame_hdr:
#Contents of section .eh_frame:
#Contents of section .init_array:
#Contents of section .fini_array:
#Contents of section .jcr:
#Contents of section .dynamic:
#Contents of section .got:
#Contents of section .got.plt:
#Contents of section .data:
#Contents of section .comment:
#Contents of section .debug_aranges:
#Contents of section .debug_info:
#Contents of section .debug_abbrev:
#Contents of section .debug_line:
#Contents of section .debug_str:
