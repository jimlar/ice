#
# Makefile for ICE, boot/kickstart parts...
# (c) Jimmy Larsson 1998
#

NASM86	=nasm -f bin
NASM    =nasm -f elf


kickstart: kickstart.S
	$(NASM) -o kickstart.o kickstart.S

boot:	boot.S
	(echo -n "SYS_SIZE equ  (";ls -l ../system.bin | grep system.bin \
		| cut -c35-41 | tr '\012' ' '; echo "+ 15) / 16") > tmp.S
	cat boot.S >> tmp.S
	$(NASM86) -o boot.b tmp.S
	rm -f tmp.S

clean:
	rm -f boot.b kickstart.o *~

