# file		ISR.asm
# date      2009/01/24
# author	kkamagui 
#           Copyright(c)2008 All rights reserved by kkamagui
# brief		���ͷ�Ʈ ���� ��ƾ(ISR) ���õ� �ҽ� ����

[BITS 64]           ; ������ �ڵ�� 64��Ʈ �ڵ�� ����

SECTION .text       ; text ����(���׸�Ʈ)�� ����

; �ܺο��� ���ǵ� �Լ��� �� �� �ֵ��� ������(Import)
extern CommonExceptionHandler, CommonInterruptHandler, KeyboardHandler
extern TimerHandler, DeviceNotAvailableHandler, HDDHandler, MouseHandler

; C ���� ȣ���� �� �ֵ��� �̸��� ������(Export)
; ����(Exception) ó���� ���� ISR
global ISRDivideError, ISRDebug, ISRNMI, ISRBreakPoint, ISROverflow
global ISRBoundRangeExceeded, ISRInvalidOpcode, ISRDeviceNotAvailable, ISRDoubleFault,
global ISRCoprocessorSegmentOverrun, ISRInvalidTSS, ISRSegmentNotPresent
global ISRStackSegmentFault, ISRGeneralProtection, ISRPageFault, ISR15
global ISRFPUError, ISRAlignmentCheck, ISRMachineCheck, ISRSIMDError, ISRETCException

; ���ͷ�Ʈ(Interrupt) ó���� ���� ISR
global ISRTimer, ISRKeyboard, ISRSlavePIC, ISRSerial2, ISRSerial1, ISRParallel2
global ISRFloppy, ISRParallel1, ISRRTC, ISRReserved, ISRNotUsed1, ISRNotUsed2
global ISRMouse, ISRCoprocessor, ISRHDD1, ISRHDD2, ISRETCInterrupt

; ���ؽ�Ʈ�� �����ϰ� �����͸� ��ü�ϴ� ��ũ��
%macro SAVECONTEXT 0       ; �Ķ���͸� ���޹��� �ʴ� SAVECONTEXT ��ũ�� ����
    ; RBP �������ͺ��� GS ���׸�Ʈ �����ͱ��� ��� ���ÿ� ����
    push rbp
    mov rbp, rsp
    push rax
    push rbx
    push rcx
    push rdx
    push rdi
    push rsi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    
    mov ax, ds      ; DS ���׸�Ʈ �����Ϳ� ES ���׸�Ʈ �����ʹ� ���ÿ� ����
    push rax        ; ������ �� �����Ƿ�, RAX �������Ϳ� ������ �� ���ÿ� ����
    mov ax, es
    push rax
    push fs
    push gs	

    ; ���׸�Ʈ ������ ��ü
    mov ax, 0x10    ; AX �������Ϳ� Ŀ�� ������ ���׸�Ʈ ��ũ���� ����
    mov ds, ax      ; DS ���׸�Ʈ �����ͺ��� FS ���׸�Ʈ �����ͱ��� ���
    mov es, ax      ; Ŀ�� ������ ���׸�Ʈ�� ��ü
   	mov gs, ax
   	mov fs, ax
%endmacro           ; ��ũ�� ��


; ���ؽ�Ʈ�� �����ϴ� ��ũ��
%macro LOADCONTEXT 0   ; �Ķ���͸� ���޹��� �ʴ� SAVECONTEXT ��ũ�� ����
    ; GS ���׸�Ʈ �����ͺ��� RBP �������ͱ��� ��� ���ÿ��� ���� ����
    pop gs
    pop fs
    pop rax
    mov es, ax      ; ES ���׸�Ʈ �����Ϳ� DS ���׸�Ʈ �����ʹ� ���ÿ��� ����
    pop rax         ; ���� ������ �� �����Ƿ�, RAX �������Ϳ� ������ �ڿ� ����
    mov ds, ax
    
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rsi
    pop rdi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    pop rbp        
%endmacro       ; ��ũ�� ��

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;	���� �ڵ鷯
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; #0, Divide Error ISR
ISRDivideError:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���� ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 0
    call CommonExceptionHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����

; #1, Debug ISR
ISRDebug:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���� ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 1
    call CommonExceptionHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����

; #2, NMI ISR
ISRNMI:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���� ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 2
    call CommonExceptionHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����

; #3, BreakPoint ISR
ISRBreakPoint:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���� ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 3
    call CommonExceptionHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����

; #4, Overflow ISR
ISROverflow:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���� ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 4
    call CommonExceptionHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����

; #5, Bound Range Exceeded ISR
ISRBoundRangeExceeded:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���� ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 5
    call CommonExceptionHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����

; #6, Invalid Opcode ISR
ISRInvalidOpcode:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���� ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 6
    call CommonExceptionHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����

; #7, Device Not Available ISR
ISRDeviceNotAvailable:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���� ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 7
    call DeviceNotAvailableHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����

; #8, Double Fault ISR
ISRDoubleFault:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���� ��ȣ�� ���� �ڵ带 �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 8
    mov rsi, qword [ rbp + 8 ]
    call CommonExceptionHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    add rsp, 8      ; ���� �ڵ带 ���ÿ��� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����

; #9, Coprocessor Segment Overrun ISR
ISRCoprocessorSegmentOverrun:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���� ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 9
    call CommonExceptionHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����

; #10, Invalid TSS ISR
ISRInvalidTSS:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���� ��ȣ�� ���� �ڵ带 �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 10
    mov rsi, qword [ rbp + 8 ]
    call CommonExceptionHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    add rsp, 8      ; ���� �ڵ带 ���ÿ��� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����

; #11, Segment Not Present ISR
ISRSegmentNotPresent:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���� ��ȣ�� ���� �ڵ带 �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 11
    mov rsi, qword [ rbp + 8 ]
    call CommonExceptionHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    add rsp, 8      ; ���� �ڵ带 ���ÿ��� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����

; #12, Stack Segment Fault ISR
ISRStackSegmentFault:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���� ��ȣ�� ���� �ڵ带 �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 12
    mov rsi, qword [ rbp + 8 ]
    call CommonExceptionHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    add rsp, 8      ; ���� �ڵ带 ���ÿ��� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����

; #13, General Protection ISR
ISRGeneralProtection:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���� ��ȣ�� ���� �ڵ带 �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 13
    mov rsi, qword [ rbp + 8 ]
    call CommonExceptionHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    add rsp, 8      ; ���� �ڵ带 ���ÿ��� ����    
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����

; #14, Page Fault ISR
ISRPageFault:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���� ��ȣ�� ���� �ڵ带 �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 14
    mov rsi, qword [ rbp + 8 ]
    call CommonExceptionHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    add rsp, 8      ; ���� �ڵ带 ���ÿ��� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����

; #15, Reserved ISR
ISR15:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���� ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 15
    call CommonExceptionHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����

; #16, FPU Error ISR
ISRFPUError:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���� ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 16
    call CommonExceptionHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����

; #17, Alignment Check ISR
ISRAlignmentCheck:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���� ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 17
    mov rsi, qword [ rbp + 8 ]
    call CommonExceptionHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    add rsp, 8      ; ���� �ڵ带 ���ÿ��� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����

; #18, Machine Check ISR
ISRMachineCheck:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���� ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 18
    call CommonExceptionHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����

; #19, SIMD Floating Point Exception ISR
ISRSIMDError:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���� ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 19
    call CommonExceptionHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����

; #20~#31, Reserved ISR
ISRETCException:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���� ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 20
    call CommonExceptionHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;	���ͷ�Ʈ �ڵ鷯
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; #32, Ÿ�̸� ISR
ISRTimer:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���ͷ�Ʈ ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 32    
    call TimerHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����

; #33, Ű���� ISR
ISRKeyboard:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���ͷ�Ʈ ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 33
    call KeyboardHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����

; #34, �����̺� PIC ISR
ISRSlavePIC:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���ͷ�Ʈ ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 34
    call CommonInterruptHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����
    
; #35, �ø��� ��Ʈ 2 ISR
ISRSerial2:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���ͷ�Ʈ ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 35
    call CommonInterruptHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����
    
; #36, �ø��� ��Ʈ 1 ISR
ISRSerial1:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���ͷ�Ʈ ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 36
    call CommonInterruptHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����
    
; #37, �з��� ��Ʈ 2 ISR
ISRParallel2:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���ͷ�Ʈ ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 37
    call CommonInterruptHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����
    
; #38, �÷��� ��ũ ��Ʈ�ѷ� ISR
ISRFloppy:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���ͷ�Ʈ ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 38
    call CommonInterruptHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����
    
; #39, �з��� ��Ʈ 1 ISR
ISRParallel1:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���ͷ�Ʈ ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 39
    call CommonInterruptHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����
    
; #40, RTC ISR
ISRRTC:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���ͷ�Ʈ ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 40
    call CommonInterruptHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����
    
; #41, ����� ���ͷ�Ʈ�� ISR
ISRReserved:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���ͷ�Ʈ ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 41
    call CommonInterruptHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����
    
; #42, ��� ����
ISRNotUsed1:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���ͷ�Ʈ ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 42
    call CommonInterruptHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����
    
; #43, ��� ����
ISRNotUsed2:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���ͷ�Ʈ ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 43
    call CommonInterruptHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����
    
; #44, ���콺 ISR
ISRMouse:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���ͷ�Ʈ ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 44
    call MouseHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����
    
; #45, �����μ��� ISR
ISRCoprocessor:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���ͷ�Ʈ ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 45
    call CommonInterruptHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����
    
; #46, �ϵ� ��ũ 1 ISR
ISRHDD1:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���ͷ�Ʈ ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 46
    call HDDHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����
    
; #47, �ϵ� ��ũ 2 ISR
ISRHDD2:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���ͷ�Ʈ ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 47
    call HDDHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����
      
; #48 �̿��� ��� ���ͷ�Ʈ�� ���� ISR              
ISRETCInterrupt:
    SAVECONTEXT    ; ���ؽ�Ʈ�� ������ �� �����͸� Ŀ�� ������ ��ũ���ͷ� ��ü
    
    ; �ڵ鷯�� ���ͷ�Ʈ ��ȣ�� �����ϰ� �ڵ鷯 ȣ��
    mov rdi, 48
    call CommonInterruptHandler
    
    LOADCONTEXT    ; ���ؽ�Ʈ�� ����
    iretq           ; ���ͷ�Ʈ ó���� �Ϸ��ϰ� ������ �����ϴ� �ڵ�� ����
