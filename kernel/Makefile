#
# Kernel Makefile 
# (c) Jimmy Larsson 1998
#

AR	=ar
NASM	=nasm -f elf
CC	=g++
CFLAGS	=-Wall -O2 -nostdinc -m486
CPP	=g++ -E -nostdinc
RANLIB	=ranlib

KERNEL_OBJS =  stack.o scheduler.o console.o interrupt.o io_ports.o descriptor.o interrupt_386.o sysinfo.o systemcall.o

.cc.o:
	$(CC) $(CFLAGS) -c -o $*.o $<

.S.o:
	$(NASM) -o $*.o $<


all:	kernel

kernel: $(KERNEL_OBJS)
	$(AR) rs libkernel.a $(KERNEL_OBJS)
	$(RANLIB) libkernel.a

clean:
	rm -f *~ *.o $(KERNEL_OBJS) libkernel.a tmp_make

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	(for i in *.cc;do echo -n `echo $$i | sed 's,\.c,\.s,'`" "; \
		$(CPP) -M $$i;done) >> tmp_make
	cp tmp_make Makefile
	rm -f tmp_make

### Dependencies:
console.sc console.o: console.cc interrupt.h types.h console.h
descriptor.sc descriptor.o: descriptor.cc types.h constants.h console.h interrupt.h \
 descriptor.h
interrupt.sc interrupt.o: interrupt.cc types.h constants.h console.h scheduler.h \
 io_ports.h descriptor.h interrupt.h
scheduler.sc scheduler.o: scheduler.cc constants.h types.h scheduler.h console.h
stack.sc stack.o: stack.cc constants.h descriptor.h types.h
sysinfo.sc sysinfo.o: sysinfo.cc sysinfo.h types.h io_ports.h console.h \
 interrupt.h
systemcall.sc systemcall.o: systemcall.cc systemcall.h types.h
