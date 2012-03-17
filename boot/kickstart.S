;
; kickstart.S contains the 32-bit startup code.
;
; The startup code will be overwritten by the page directory.
;
; Compile with nasm -o kickstart.o kickstart.S
;              ld -Ttext 0x0 -o kickstart kickstart.o
;
; Written for NASM because GNU as really suck!
;		
; Seems as everthing from somewhere between 0x1000 & 0x2000 and
; until the end of the binary, everything is zeroed when loaded
; from bootblock !?!? 
; -FIXED, used only 16 sectors, stupid!
;

global _idt,_gdt,_pg_dir,_startup_32, printmess
extern stack_start, main

segment .text			
	
_start:		
_pg_dir:
_startup_32:
	
	mov  ax,boot_data32_idx	; load DS with GDT index from boot block  
	mov ds,ax

 	lidt [DS:idt_descr]

 	lgdt [DS:gdt_descr]
	jmp dword code32_idx:setup_gdt2  ;far jump to set CS 	
setup_gdt2:
		
	mov eax,data32_idx	; reload all the segment registers    
	mov ds,ax		; after changing gdt. CS was already  
	mov es,ax		; reloaded in 'setup_gdt'  
	mov fs,ax
	mov gs,ax

	;setup stack
	lss esp,[DS:stack_start] ; load stack_start from kernel/stack.cc
	
	xor eax,eax
.1	inc eax			; check that A20 really IS enabled  
	mov  [0x000000],eax
	cmp  [0x100000],eax
	je   .1
	mov  eax,cr0		; check math chip  
	and  eax,0x80000011	; Save PG,ET,PE  
	test eax,0x10
	jne  .2			; ET is set - 387 is present  
	or   eax,4		; else set emulate bit  
.2	mov  cr0,eax
	jmp  after_page_tables


; Space for page directories

	times 0x1000-($-$$) db 0 ; (org 0x1000)
pg0:

	times 0x2000-($-$$) db 0 ; (org 0x2000)
pg1:

	times 0x3000-($-$$) db 0 ; (org 0x3000)

pg2:		 ; This is not used yet, but if you 
 		 ; want to expand past 8 Mb, you'll have
  		 ; to use it.

	times 0x4000-($-$$) db 0 ; (org 0x4000)


after_page_tables:
	
 	push  byte 0		; These are the parameters to main  
 	push  byte 0
 	push  byte 0
 	push  LONG L6		; return address for main 
	push  LONG main		; main() from init/main.cc   
 	jmp   setup_paging 
L6: 
	jmp L6			; main should never return here, but  
 				; just in case, we know what happens.  


;	
; Setup_paging
;
; This routine sets up paging by setting the page bit
; in cr0. The page tables are set up, identity-mapping
; the first 8MB. The pager assumes that no illegal
; addresses are produced (ie >4Mb on a 4Mb machine).
;
; NOTE! Although all physical memory should be identity
; mapped by this routine, only the kernel page functions
; use the >1Mb addresses directly. All "normal" functions
; use just the lower 1Mb, or the local data space, which
; will be mapped to some other place - mm keeps track of
; that.
;
; For those with more memory than 8 Mb - tough luck. I've
; not got it, why should you :-) The source is here. Change
; it. (Seriously - it shouldn't be too difficult. Mostly
; change some constants etc. I left it at 8Mb, as my machine
; even cannot be extended past that (ok, but it was cheap :-)
; I've tried to show which constants to change by having
; some kind of marker at them (search for "8Mb"), but I
; won't guarantee that's all :-( )
;
; THIS IS PROBABLY A MESS!!!

	
	align 2
setup_paging:
 	mov  ecx,1024*3
 	xor  eax,eax
 	xor  edi,edi			; pg_dir is at 0x000    
 	cld    
 	mov  [_pg_dir],LONG pg0+7	; set present bit/user r/w    
 	mov  [_pg_dir+4],LONG pg1+7   	;  --------- " " ---------    
 	mov  edi,pg1+4092
 	mov  eax,0x7ff007		;  8Mb - 4096 + 7 (r/w user,p)    
 	std
.3:	stosd			; fill pages backwards - more efficient    
 	sub  eax,0x1000
 	jge  .3
 	xor  eax,eax		; pg_dir is at 0x0000  
 	mov  cr3,eax		; cr3 - page directory start    
 	mov  eax,cr0
 	or   eax,0x80000000
 	mov  cr0,eax		; set paging (PG) bit   
	ret			; this also flushes prefetch-queue    


; Print mess pointed to by stack
; Format: word x, word y, attribute byte, string, 0
;
; void printmess (char *mess, int x, int y, int attribute)
;
msgPtr: dd 0
msgX: dd 0
msgY: dd 0
msgAttr: dd 0

printmess:
	push	eax
	mov	eax,long [esp+8]
	mov	long [msgPtr],eax
	mov	eax,long [esp+12]
	mov	long [msgX],eax
	mov	eax,long [esp+16]
	mov	long [msgY],eax
	mov	eax,long [esp+20]
	mov	long [msgAttr],eax
	pop	eax
	
	pushad
	mov	esi,[msgPtr]
	mov	eax,core32_idx		; in protected mode, we have to use  
					; core memory to address the screen  
	mov	es,ax
	mov	di,word [msgY]		; get Y position  
	imul	edi,160
	add	di,word [msgX]		; add X position  
	add	di,word [msgX]
	add	edi,0xb8000		; physical address of text screen  
	mov	ah,byte [msgAttr]		; get attribute byte  
	;add	esi,5
write_loop:
	mov	al,[esi]
	or	al,al			; end of string?  
	jz	loop_end
	inc	esi
	mov	[edi],ax
	inc	edi
	inc	edi
	jmp	write_loop
loop_end:
	popad
	ret


; GDT offsets, matches those in kernel/descriptor.h	
code32_idx	equ	0x30	; offset of 32-bit code segment in GDT
boot_data32_idx equ     0x10	; offset of data in the bootblocks GDT
data32_idx	equ	0x18	; offset of 32-bit data segment in GDT
core32_idx	equ	0x18	; offset of 32-bit core segment in GDT

			
	align 2
	dw 0
	
idt_descr:
	dw 0x800		; limit  
	dd _idt			; base

_idt:	times 256 dw 0,0,0,0	; idt is uninitialized   


	align 2
	dw 0	

gdt_descr:
	dw 0x800	; limit  
	dd _gdt		; base address   

	; The indexing of this obey's bios standards
_gdt:	dd 1111111111, 0x00000000 	; NULL descriptor

	dd 0x00000000, 0x00000000       ; GDT descriptor
	dd 0x00000000, 0x00000000       ; IDT descriptor

	; G=1, D=1, P=1, DPL=0, DT=1, data read/write
	; Base=0, length=0x7ff * 0x1000 (8MB)
	dw 0x07ff ; Limit, 8MB for now, should use 4GB
	dw 0x0000 ; Base 0..15
	dw 0x9200 ; data read/write
	dw 0x00c0 ; granularity 4096, 386 (opsize 32)

	dd 0x00000000, 0x00000000       ; ES  descriptor
	dd 0x00000000, 0x00000000       ; SS  descriptor

	; G=1, D=1, P=1, DPL=0, DT=1, code exec-read,	
	; Base=0, length=0x7ff * 0x1000 (8MB)
	dw 0x07ff ; Limit, 8MB
	dw 0x0000 ; Base 0..15
	dw 0x9a00 ; code exec/read
	dw 0x00c0 ; granularity 4096, opsize 32

	dd 0x00000000, 0x00000000       ; BIOS temp descriptor

	times 248 dd 0,0		; space for LDT's and TSS's etc    

