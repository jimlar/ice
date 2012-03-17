;
; Boot Reccord-code for my OS project
; (C) Jimmy Larsson 1998
;
; Assembler is not my strong side (which you probably can see from this code), 
; but hopefully this works as intended.
;	
; This code loads the system, gets us into Protected Mode, and jumps to the 
; kickstart code... (kickstart.S)
;				 	
;	
; Make bootblock with:	
;	nasm -f bin -o boot.b boot.S
;
;
; SYS_SIZE has to be filled in with the size of the system, after it's
;	   been compiled.		
;	
; SYS_SIZE = (size of stripped linked binary + 15) / 16
;				
	
		
SYS_SEG	   equ	0x1000		    ; Where to load system	
END_SEG	   equ	SYS_SEG + SYS_SIZE  ; Last segment of the kernel binary
SYS_MOVE   equ	0x0000		    ; Where to move system after loading
LOAD_ADDR  equ	0x7c00		    ; Where we are loaded by bios	
BOOTSEG	   equ	0x07c0		    ; Same but segment
MOVE_ADDR  equ	0x90000		    ; Where to move ourselves	
INITSEG	   equ	0x9000		    ; Same but segment
SECTORS	   equ	18		    ; Sectors per track (1.44 floppy)


global _main	
section .text
						
_main:					
	org 0x0				
; Move us to a new location				

	mov	ax,BOOTSEG	; Move from Bootsegment
	mov	ds,ax
	mov	ax,INITSEG	; to Initsegment
	mov	es,ax
	mov	cx,256		; move 256 words
	sub	si,si
	sub	di,di
	rep
	movsw
	jmp	dword INITSEG:(copy-$$) ; Make a jump to the copy

copy:	mov	ax,cs		; Set all segments to code-segment
	mov	ds,ax	
	mov	es,ax
	mov	ss,ax
	mov	sp,0x4000	; Set stack pointer (to anything > 512) 

	

	lea si, [DS:LoadingMsg]
	call Print		; Print Loading message
	

; We have written the message,	
; it's time to load the system into memory...
	
	mov	ax,SYS_SEG      ; (9000:002a)
	mov	es,ax		; put start segement in es
	call	read_system	; read the stuff
	call	kill_motor	; stop floppy motor

	cli			; turn off interrupts

; Move the system code to where it should be

	mov	ax,SYS_MOVE	; Where to move system
	cld			; 'direction'=0, movs moves forward
move:
	mov	es,ax		; destination segment
	add	ax,0x1000
	cmp	ax,0x9000
	jz	move_done
	mov	ds,ax		; source segment
	sub	di,di
	sub	si,si
	mov 	cx,0x8000
	rep
	movsw
	jmp	move

; Ok, system moved, load the segment descriptors
	
move_done:	
	mov	ax,cs		; Put code-segment register in ax	
	mov	ds,ax		; put ax in data-segment register
	lidt	[DS:my_idt]	; load idt
	lgdt	[DS:my_gdt]	; load gdt

		
	
; Ok, time to turn on A20

	call	ckeck_kbd
	mov	al,0xD1		; Write
	out	0x64,al
	call	ckeck_kbd
	mov	al,0xDF		; A20 on
	out	0x60,al
	call	ckeck_kbd	
		

; Now, it's time for the real PM transfer,
; Jump to absolute address 0x00000, in 32-bit protected mode.
; 
	
jump_pm:		
	mov	ax,0x0001	; protected mode (PE) bit
	lmsw	ax		; This is it!

	jmp	dword 0x8:0x0	; far jump (to set CS correctly)
				; (GDT segment 0x8)


; This routine checks that the keyboard command queue is empty
; No timeout is used - if this hangs there is something wrong with
; the machine, and we probably couldn't proceed anyway.

ckeck_kbd:
	dw	0x00eb,0x00eb
	in	al,0x64		; 8042 status port
	test	al,2		; is input buffer full?
	jnz	ckeck_kbd	; yes - loop
	ret

; This routine loads the system at address SYS_SEG*16, making sure
; no 64kB boundaries are crossed. We try to load it as fast as
; possible, loading whole tracks whenever we can.
;
; es should contain staring segment
;
; This uses the SECTORS varible which is normally 18 for 1.44 floppys
	
		
sector:	dw 1			; sectors read of current track
head:	dw 0			; current head
track:	dw 0			; current track

read_system:
	mov ax,es
	test ax,0x0fff
.loop:	jne .loop			; es must be at 64kB boundary
	xor bx,bx		; bx is starting address within segment
read_loop:
	mov ax,es
	cmp ax,END_SEG		; have we loaded all yet?
	jb .read1

	lea si, [endReadMsg]
	call Print
	
	ret
.read1:
	mov ax,SECTORS
	sub ax,[sector]
	mov cx,ax
	shl cx,9
	add cx,bx
	jnc .read2
	je .read2
	xor ax,ax
	sub ax,bx
	shr ax,9
.read2:
	call read_track
	mov cx,ax
	add ax,[sector]
	cmp ax,SECTORS
	jne .read3
	mov ax,1
	sub ax,[head]
	jne .read4
	inc word [track]
.read4:
	mov [head],ax
	xor ax,ax
.read3:
	mov [sector],ax
	shl cx,9
	add bx,cx
	jnc read_loop
	mov ax,es
	add ax,0x1000
	mov es,ax
	xor bx,bx
	jmp read_loop

read_track:
	push ax			
	push bx
	push cx
	push dx
	push si
	
	mov dx,[track]
	mov cx,[sector]
	inc cx
	mov ch,dl
	mov dx,[head]
	mov dh,dl
	mov dl,0
	and dx,0x0100
	mov ah,2
	int 0x13
	jc bad_read_track
	
	lea si,[trackMsg]
	call Print

	pop si
	pop dx
	pop cx
	pop bx
	pop ax
	ret
bad_read_track:	
	lea si,[badReadTrack]
	call Print

	mov ax,0
	mov dx,0
	int 0x13
	pop dx
	pop cx
	pop bx
	pop ax
	jmp read_track

;
;  This procedure turns off the floppy drive motor, so
;  that we enter the kernel in a known state, and
;  don't have to worry about it later.
;
	
kill_motor:
	push dx
	mov dx,0x3f2
	mov al,0
	outb
	pop dx
	ret
	
; Printing subrotine
; Needs pointer to null terimnated string in si

Print:	lodsb			; Load char into AL
	cmp al,0		; End of string?
	je  EndPrint		; Yes! return
	mov cx,0x01
	mov ah,0x0e		; 0Eh = Teletype Putchar command for INT10
	mov bx,0x07		; Normal attribute
	int 0x10		; Print character
	jmp Print	
EndPrint: ret


	
LoadingMsg:	db  10,13
		db  "Loading"	
		db  0
		
trackMsg:	db  "."	
		db  0

badReadTrack:	db  "!"
		db  0	

endReadMsg:	db  "done",10,13,0
		

; Descriptor tables

	times 474-($-$$) db 0	; (aka org 474)	

my_gdt:
	dw	0x18		; gdt limit=2048, 256 GDT entries
	dw	gdt,0x9		; gdt base
							
gdt:
	dw	0,0,0,0		; dummy

	dw	0xFFFF		; 4Gb - limit
	dw	0x0000		; base address=0
	dw	0x9A00		; code read/exec
	dw	0x00C0		; granularity=4096, 386

	dw	0xFFFF		; 4Gb - limit
	dw	0x0000		; base address=0
	dw	0x9200		; data read/write
	dw	0x00C0		; granularity=4096, 386

my_idt:
	dw	0			; idt limit=0
	dw	0,0			; idt base=0L


; Boot signature	
boot_signature:	
	times 510-($-$$) db 0 ; (org 510)
	dw 0xaa55		; The boot signature






