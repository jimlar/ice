;
; io_ports.S
;
; simple port functions
; 
;
; (c) Jimmy Larsson 1998
;
; asembly with "nasm -o io_ports.o io_ports.S"
;
;

global out_byte, in_byte


align 16
out_byte:
	mov	edx, [4+esp]
	mov	al, [8+esp]
	out     dx,al
	ret

align 16
in_byte:
	mov	edx, [4+esp]
	sub	eax,eax
	in	al,dx
	ret
	