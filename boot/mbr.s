! Master Boot Reccord-code (MBR) for my OS project
! (C) Jimmy Larsson 1997
!
! Make bootblock with:	
!	as86 -0 -a -w -o mbr.o mbr.S
!	ld86 -0 -s -o mbr.img mbr.o		
!	dd if=mbr.img of=mbr.b bs=32 skip=1
!
		
! Where we are loaded by bios	
LOAD_ADDR = 0x7c00
	
! Where to move ourselves	
MOVE_ADDR = 0x600			

BOOTSEG   = 0x07C0
INITSEG	  = 0x0060
			
! Set up stack, segment registers and flags.
! Move from 7c00h wwhere we have been loaded to 600h.
! Put new address (600h) in BP. Pass control to the copy.

	.globl _main	 
	.org 0
				
_main:	
_1:			

	cli			
	mov ax,#BOOTSEG		; AX = 0x7c0
	mov ss,ax		; Stack segment = 0
	mov sp,#LOAD_ADDR	; Stack top is below loading point
	mov ds,ax
	mov es,ax		; Data segments = 0
	sti

! Stack is set up, move us to new location 

	mov ax,#BOOTSEG		; Move from 0x7c0
	mov ds,ax
	mov ax,#INITSEG		; To #INITSEG
	mov es,ax
	mov cx,#256		; 256 words (One sector)
	sub si,si
	sub di,di
	cld			; Clear direction flag
	rep 
	movsw			; Perform the move
	mov bp,#INITSEG*16	; Address to the copy
	
		
	jmpi Next,INITSEG	; Pass control to the copy

Next:
	
! Check for the bootsignature (55AAh) at 1FEh, if not present,
! display an error message, wait for a key and reboot.
	 	
_2:	cmp word (bp+0x1fe),#0xaa55
	je  NoError
Error:	lea si,ErrorMsg		; Address of message
	call Print		; Print Errormessage
	jmp Reboot
	
NoError:
	lea si,LoadingMsg
	call Print		; Print Loading message

! Set a pointer PP (partition pointer) to 1BEh.
! Set a counter, L, to 4.
		
_3:	mov si,#0x1be
	mov bl,#4

! Check byte at PP, if its 80h (bootable partition), goto _7	

_4:	cmp byte (bp+si),#0x80	; Is partition active?
	je  _7			; Yes! goto _7

! Check all four partition entries
! If none is bootable, give error message
		
_5:	add si,#16		; Add 16 to PP, next partition
	dec bl			; all four done?
	jz  Error		; Yes! Error!

	jmp _4			; No! Try next partition

! Load registers:
! BX = 7c00h, AX = 0201h, CX = word(BP + PP + 2)
! DX = byte(PP + 1), DL = 80h (81h for second physical disk etc.)
! Call INT13 to load bootsector from active partition. If error, display message
		
_7:	mov bx,#0x7c00		; Load bootsector to 7c00h
	mov ax,#0x0201		; Read (02 in AH) one sector (01 in AL)
	mov cx,(bp+si+2)	; CX = partition start sector/cylinder
	mov dh,(bp+si+1)	; DH = partition start head
	mov dl,#0x80		; Load from first disk
	int 0x13
	jc  Error

! Check word at 7DFEh to be AA55h (boot signature). If not, error. If true,
! far jump to 0:7c00 with interrupts cleared.
		
_8:	cmp word [0x7dfe], #0xaa55 ; Valid boot signature?
	jne Error		; No! Error!
	cli			; Clear interrupts
	jmp far 0:0x7c00	; Jump to start of bootsector code

! Printing subrotine
! Needs pointer to null terimnated string in si
Print:	lodsb			; Load char into AL
	cmp al,#0		; End of string?
	je  EndPrint		; Yes! return
	mov cx,#0x01
	mov ah,#0x0e		; 0Eh = Teletype Putchar command for INT10
	mov bx,#0x07		; Normal attribute
	int 0x10		; Print character
	jmp Print	
EndPrint: ret

! Reset computer (in case of errors)		
Reboot:	xor ax,ax		; AX = 0, command Getkey for INT16
	int 0x16		; Wait for a key to be pressed
	jmp 0x0f000:0x0ffff	; Reset computer

! Small beep code
Beep:	mov ax,#0x0e07
	int 0x10
	ret


! Messages	
ErrorMsg:	.ascii "Error in MBR"
		db  0
LoadingMsg:	.ascii "Loading..."	
		db  0

	
	.org 510
! Boot signature	
boot_signature:	
	.word 0xaa55		; The boot signature

