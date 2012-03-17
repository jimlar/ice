#
# Makefile for ICE
#

AR	=ar
NASM	=nasm -f elf
AS	=as
LD	=ld
LDFLAGS	=-s -x -Ttext 0x0 -e _startup_32
CC	=g++
CFLAGS	=-Wall -O2 -fstrength-reduce -fomit-frame-pointer -nostdinc
CPP	=gcc -E -nostdinc
OBJDUMP =objdump
OBJCOPY =objcopy

# The order of this shit is important!
SYSTEM_OBJS = boot/kickstart.o init/libinit.a kernel/libkernel.a

# All subdirs except boot.. =)
SUBDIRS = init kernel


all:	system

system:	$(SYSTEM_OBJS)
	$(LD) $(LDFLAGS) -o system.elf $(SYSTEM_OBJS)
	$(OBJCOPY) --remove-section=.note --remove-section=.comment --output-target=binary system.elf system.bin
	(cd boot; make boot)
	cat boot/boot.b system.bin > system
	rm -f system.elf system.bin
	sync

init/libinit.a:
	(cd init; make dep; make)

kernel/libkernel.a:
	(cd kernel; make dep; make)

boot/kickstart.o:
	(cd boot; make kickstart)

clean:
	for i in $(SUBDIRS); do (cd $$i && make clean); done;
	(cd boot; make clean)
	rm -f core system.elf system.bin system *~ tmp_make

dep:
	for i in $(SUBDIRS); do (cd $$i && make dep); done;

     




