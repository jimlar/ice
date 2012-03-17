#
# Init Makefile for ICE
# (c) Jimmy Larsson 1998
#

AR	=ar
NASM	=nasm -f elf
CC	=g++
CFLAGS	=-Wall -nostdinc -m486 -O2
CPP	=g++ -E -nostdinc
RANLIB	=ranlib

INIT_OBJS =  main.o

.cc.o:
	$(CC) $(CFLAGS) -c -o $*.o $<

.S.o:
	$(NASM) -o $*.o $<

all:	init

init:	$(INIT_OBJS)	
	$(AR) r libinit.a $(INIT_OBJS)
	$(RANLIB) libinit.a	
clean:
	rm -f core *~ $(INIT_OBJS) libinit.a tmp_make

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	(for i in *.cc;do echo -n `echo $$i | sed 's,\.c,\.s,'`" "; \
		$(CPP) -M $$i;done) >> tmp_make
	cp tmp_make Makefile
	rm -f tmp_make

### Dependencies:
main.sc main.o: main.cc ../kernel/descriptor.h ../kernel/types.h \
 ../kernel/interrupt.h ../kernel/scheduler.h ../kernel/console.h \
 ../kernel/io_ports.h ../kernel/sysinfo.h
