;
; interrupt_386.S
; Lowlevel parts of interrupt handling and exceptions
; (e.g. push regs etc...)
; (c) Jimmy Larsson 1998
;
;

; Interrupt stubs
global save, restart, hardware_int_00, hardware_int_01, hardware_int_02
global hardware_int_03, hardware_int_04, hardware_int_05, hardware_int_06
global hardware_int_07, hardware_int_08, hardware_int_09, hardware_int_10
global hardware_int_11, hardware_int_12, hardware_int_13, hardware_int_14
global hardware_int_15

; Exception stubs
global exception_divide, exception_step, exception_nmi, exception_breakpoint
global exception_overflow, exception_invalid_opcode, exception_bounds_check
global exception_coproc_not_availiable, exception_double_fault
global exception_coproc_seg_overrun, exception_invalid_tss
global exception_segment_not_present, exception_stack
global exception_general_protection, exception_page_fault
global exception_coproc_error, system_call

; Imported funtions/variables 
extern InterruptHandler_k_reenter, stack_start, InterruptHandler_held_int_head
extern InterruptHandler_unhold, Scheduler_current_proc_ptr
extern DescriptorHandler_tss, InterruptHandler_irq_table
extern InterruptHandler_exception, SystemCall_call

; Constants
INT1_CTL	equ   0x20
INT2_CTL	equ   0xa0
INT1_CTLMASK	equ   0x21
INT2_CTLMASK	equ   0xa1

ENABLE_INT	equ   0x20

; Indices in process table
RETADR		equ   48	
AXREG           equ   44
P_LDT_SEL	equ   0   
TSS3_S_SP0	equ   4

;Exception int. numbers
DIVIDE_INT	        equ   0  ; divide error
DEBUG_INT		equ   1	 ; single step
NMI_INT			equ   2	 ; non-maskable interrupt
BREAKPOINT_INT		equ   3	 ; software breakpoint
OVERFLOW_INT		equ   4	 ; overflow
BOUNDS_CHECK_INT	equ   5	 ; bounds check fail
INVALID_OPCODE_INT	equ   6	 ; invalid opcode
COPROC_NOT_AVAIL_INT	equ   7	 ; coprocessor not availiable
DOUBLE_FAULT_INT        equ   8	 ; double fault
COPROC_SEG_OVERRUN_INT  equ   9  ; coprocessor segment overrun
INVALID_TSS_INT		equ   10 ; invalid TSS
SEGMENT_NOT_PRESENT_INT equ   11 ; segment not present
STACK_FAULT_INT		equ   12 ; stack exception
GENERAL_PROTECTION_INT	equ   13 ; general protection fault
PAGE_FAULT_INT		equ   14 ; page fault
COPROC_ERROR_INT	equ   16 ; coprocessor error
 

; Save current task, happens on int's/irq's
align 16
save:
	cld
	pushad
	push  word ds
	push  word es
	push  word fs
	push  word gs
	mov   dx,ss
	mov   ds,dx		;set kernel DS
	mov   dx,es		
	mov   eax,esp		
	inc   byte [InterruptHandler_k_reenter]	;are we already in kernel code?
	jnz   set_restart1	;yes, don't setup kernel stack
	mov   esp, [stack_start] ;no, then setup kernel stack
	push  long restart	
	jmp   [RETADR + eax]	;RETADR is offset in tss

align 4
set_restart1:
	push  long restart1
	jmp   [RETADR + eax]	;RETADR is offset in tss



;
;
; System call handler (only lowlevel part, rest in c++)
;
;
align 16

system_call:
	cld
	sub	esp, 6*4
	push	ebp
	push	esi
	push	edi
	push	ds
	push	es
	push	fs
	push	gs
	mov	dx, ss
	mov	ds, dx
	mov	es, dx
	inc     byte [InterruptHandler_k_reenter]	
	mov	esi, esp
	mov	esp, [stack_start]
	xor	ebp, ebp
	
	sti
	
	; Setup parameters for sys_call() c++ function
	push    ebx
	push	eax
	push	ecx
	call	SystemCall_call

	mov	[AXREG + esi], eax
	cli


;
; restart, kick in possible new task after interrupt/sys_call
;
	
align 16
restart:
	cmp   long [InterruptHandler_held_int_head], 0
	jz    over_call_unhold
	call  InterruptHandler_unhold
over_call_unhold:
	mov   esp, (Scheduler_current_proc_ptr)
	lldt  [P_LDT_SEL + esp]
	lea   eax, [esp]
	mov   [DescriptorHandler_tss + TSS3_S_SP0], eax
restart1:
	dec   byte [InterruptHandler_k_reenter]
	pop   gs
	pop   fs
	pop   es
	pop   ds
	popad
	add   esp, 4
	iretd


;
; Hardware Interrupt stub-macro
;

%macro hardware_int_master 1
       call   save
       in     al,INT1_CTLMASK
       or     al, 1 << %1
       out    INT1_CTLMASK, al
       mov    al, ENABLE_INT
       out    INT1_CTL, al
       sti
       push   long %1
       call   [InterruptHandler_irq_table + 4*%1]
       pop    ecx
       cli
       test   eax,eax
       jz     .end
       in     al,INT1_CTLMASK
       and    al, ~(1<<%1)
       out    INT1_CTLMASK, al
.end:  ret
%endmacro

%macro hardware_int_slave 1
       call   save
       in     al,INT2_CTLMASK
       or     al, 1 << %1-8
       out    INT2_CTLMASK, al
       mov    al, ENABLE_INT
       out    INT1_CTL, al
       jmp    $+2
       out    INT2_CTL, al
       sti
       push   long %1
       call   [InterruptHandler_irq_table + 4*%1]
       pop    ecx
       cli
       test   eax,eax
       jz     .end
       in     al,INT2_CTLMASK
       and    al, ~(1<<%1-8)
       out    INT2_CTLMASK, al
.end:  ret
%endmacro

;
; Hardware interrupt stubs
;

align 16
hardware_int_00:
	hardware_int_master 0 
align 16      
hardware_int_01:
	hardware_int_master 1       
align 16
hardware_int_02:
	hardware_int_master 2       
align 16
hardware_int_03:
	hardware_int_master 3 
align 16	      
hardware_int_04:
	hardware_int_master 4       
align 16
hardware_int_05:
	hardware_int_master 5       
align 16
hardware_int_06:
	hardware_int_master 6       
align 16
hardware_int_07:
	hardware_int_master 7       
align 16
hardware_int_08:
	hardware_int_slave 8       
align 16
hardware_int_09:
	hardware_int_slave 9       
align 16
hardware_int_10:
	hardware_int_slave 10       
align 16
hardware_int_11:
	hardware_int_slave 11      
align 16
hardware_int_12:
	hardware_int_slave 12      
align 16
hardware_int_13:
	hardware_int_slave 13      
align 16
hardware_int_14:
	hardware_int_slave 14      
align 16
hardware_int_15:
	hardware_int_slave 15


;
; exception handlers, pushes exception number
; and calls c++ code.
;

align 16
exception_divide:
	push	long DIVIDE_INT
	jmp	exception
align 16
exception_step:
	push    long DEBUG_INT
	jmp	exception
align 16
exception_nmi:
	push    long NMI_INT
	jmp	exception
align 16
exception_breakpoint:
	push    long BREAKPOINT_INT
	jmp	exception
align 16
exception_overflow:
	push    long OVERFLOW_INT
	jmp	exception
align 16
exception_bounds_check:
	push    long BOUNDS_CHECK_INT
	jmp	exception
align 16
exception_invalid_opcode:
	push    long INVALID_OPCODE_INT
	jmp	exception
align 16
exception_coproc_not_availiable:
	push    long COPROC_NOT_AVAIL_INT
	jmp	exception
align 16
exception_double_fault:
	push    long DOUBLE_FAULT_INT
	jmp	error_exception
align 16
exception_coproc_seg_overrun:
	push    long COPROC_SEG_OVERRUN_INT
	jmp	exception
align 16
exception_invalid_tss:
	push    long INVALID_TSS_INT
	jmp	error_exception
align 16
exception_segment_not_present:
	push    long SEGMENT_NOT_PRESENT_INT
	jmp	error_exception
align 16
exception_stack:
	push    long STACK_FAULT_INT
	jmp	error_exception
align 16
exception_general_protection:
	push    long GENERAL_PROTECTION_INT
	jmp	error_exception
align 16
exception_page_fault:
	push    long PAGE_FAULT_INT
	jmp	error_exception
align 16
exception_coproc_error:
	push    long COPROC_ERROR_INT
	jmp	exception


;
; exception, for exceptions without error code
;
trap_errno:	dd 0
ex_number:	dd 0
old_eip:	dd 0
old_cs:		dd 0
old_eflags:	dd 0

align 16
exception:
	mov	long [SS:trap_errno], 0
	pop	long [SS:ex_number]
	jmp	exception2

;
; exception, for exceptions with error code
;
align 16
error_exception:
	pop	long [SS:ex_number]
	;pop	long [SS:trap_errno]    ; This is what I should use, not the next line
	mov	long [SS:trap_errno], 0 ; but it doesnt seem to work.. =)
;
; actual exception code for both types
;

exception2:
	push	eax
	mov	eax, [4 + esp]
	mov	[SS:old_eip], eax
	mov	eax, [8 + esp]     ; movzx??????
	mov	[SS:old_cs], eax
	mov	eax, [12 + esp]
	mov	[SS:old_eflags], eax
	pop	eax

	call	save
	push	long [old_eflags]
	push	long [old_cs]
	push	long [old_eip]
	push	long [trap_errno]
	push	long [ex_number]
	call	[InterruptHandler_exception]

	add	esp, 5*4
	cli
	ret
	








