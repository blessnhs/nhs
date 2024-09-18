# file      BootLoader.asm
# date      2017/09/15
# author    blessnhs
#           Copyright(c)2017 All rights reserved by blessnhs
# brief     JG OS BootLoader

[ORG 0x00]
[BITS 16]

SECTION .text

jmp 0x07C0:LEVEL1

TOTALSECTORCOUNT:	 dw	0x01	;maxium boot loader size
KERNEL32SECTORCOUNT: dw 0x02    ;32bit kernel sector size
BOOTSTRAPPROCESSOR:  db 0x01    ;is bsp processor
ISGRAPHICMODE:       db 0x01  ;is graphic mode

LEVEL1:

	xor ax,ax					; ax register
    mov ax, 0x07C0  			;
    mov ds, ax      			; ds
    mov ax, 0xB800
    mov es, ax      			; ES	segment address 0x07c0�� ����

    mov ax, 0x0000
    mov ss, ax      			; init stack 0 ~ 0xfffe(64k)
    mov sp, 0xFFFE
    mov bp, 0xFFFE

    mov byte[ BOOTDRIVE ], dl  ; 

	mov ax, 0x1112
    int 0x10

;	mov si,    0

.SCREENCLEARLOOP:   			;clear screen

    mov byte [ es: si ], 0
    mov byte [ es: si + 1 ], 0x07

    add si, 2

    cmp si, 80 * 50 * 2

    jl .SCREENCLEARLOOP
    push STARTUP_MESSAGE		; message
    push 0						; x
    push 0						; y
    call PRINT_STRING			; call print
    add  sp, 6

 	mov si, IMAGE_LOADING_MESSAGE
    call PRINT_STRING

LEVEL2:


    ; 0 IS FLOPPY DISK
    mov ax, 0
    mov dl, byte [ BOOTDRIVE ]
    int 0x13
    jc  HANDLEDISKERROR



	; read disk parameter
    mov     ah, 0x08
    mov     dl, byte [ BOOTDRIVE ]		; read driver
    int     0x13
    jc      HANDLEDISKERROR

    mov     byte [ MAXHEAD ], dh        ; set max header count
    mov     al, cl                      ; sector track save info
    and     al, 0x3f					; ���� 6��Ʈ ���� ����

    mov     byte [ MAXSECTOR ], al      ;
    mov     byte [ MAXTRACK ], ch       ; Ʈ�� ���� ���� 8��Ʈ

    mov si, 0x1000						; ������ ���� �ּ�

    mov es, si							; 0x1000:0000(0x10000)
    mov bx, 0x0000


    mov di, word [ TOTALSECTORCOUNT ]
    ;mov di, 1146

READDATA:

    cmp di, 0							; ������ ���͸� �� �о����� ����
    je  READEND
    sub di, 0x1							; �ϳ� ����

    mov ah, 0x02						; ���̿��� read �Լ� ȣ�� ���� ��ȣ 2�� read sector
    mov al, 0x1							; sector 1 �о����
    mov ch, byte [ TRACKNUMBER ]		; Ʈ����ȣ
    mov cl, byte [ SECTORNUMBER ]		; ���� ��ȣ
    mov dh, byte [ HEADNUMBER ]			; ��� ��ȣ
    mov dl, byte [ BOOTDRIVE ]			; ��Ʈ ��ȣ
    int 0x13
    jc HANDLEDISKERROR

    add si, 0x0020						;512����Ʈ �о����� ����
    mov es, si							;es ���׸�Ʈ �������Ϳ� �ݿ�

    mov al, byte [ SECTORNUMBER ]		;���� ��ȣ ����
    add al, 0x01						;���� ��ȣ ����
    mov byte [ SECTORNUMBER ], al		;������Ų ���� ��ȣ ����
    cmp al, byte [ MAXSECTOR ]			;������ ���� ��ȣ �ΰ�
    jle READDATA

    add byte [ HEADNUMBER ], 0x01		; ������ ���ͱ��� �о��ٸ� ��� ����
    mov byte [ SECTORNUMBER ], 0x01

    mov al, byte [ MAXHEAD ]			;
    cmp byte [ HEADNUMBER ], al			; ������ ��� ��ȣ�̸� Ʈ�� ����
    jg .ADDTRACK

    jmp READDATA

.ADDTRACK:
    mov byte [ HEADNUMBER ], 0x00		; ��� ��ȣ�� �ʱ�ȭ ��Ű��
    add byte [ TRACKNUMBER ], 0x01		; Ʈ����ȣ�� ���� ��Ų��.
    jmp READDATA

READEND:

	mov si, LOADING_COMPLETE_MESSAGE
    call PRINT_STRING

	;���� ��� ���� ���̿��� �Լ� ȣ��
    mov ax, 0x4F01						;���� ��� ���� ȣ�� ��ȣ
    mov cx, 0x117						;1024x768 �ػ� 16��Ʈ ����
    mov bx, 0x07E0
    mov es, bx
    mov di, 0x00

    int 0x10
    cmp ax, 0x004F
    jne VBEERROR

    cmp byte [ ISGRAPHICMODE ], 0x00	;���� ���� �������� �ʴ´ٸ�
    je JUMPTOPROTECTEDMODE				;�ٷ� 32bit ��ȣ ���� ����

    mov ax, 0x4F02						;���� ��� ��ȯ ȣ���ȣ
    mov bx, 0x4117						;1024x768 �ػ� 16��Ʈ

    int 0x10
    cmp ax, 0x004F
    jne VBEERROR						;���� �߻��� �̵�

    jmp JUMPTOPROTECTEDMODE

VBEERROR:
    push CHANGE_GRAPHIC_MODE_FAIL
    push 2
    push 0
    call PRINT_STRING
    add  sp, 6
    jmp $

JUMPTOPROTECTEDMODE:
    jmp 0x1000:0x0000		;64k ��ġ�� �̵�

HANDLEDISKERROR:
    push DISK_ERROR_MESSAGE
    push 1
    push 20
    call PRINT_STRING

    jmp $

PRINT_STRING:
    mov ah, 0x0E                    ; BIOS function 0x0E: teletype
    .loop:
        lodsb                       ; grab a byte from SI
        test al, al                 ; NULL?
        jz .done                    ; if zero: end loop
        int 0x10                    ; else: print character to screen
        jmp .loop
    .done:
        ret


STARTUP_MESSAGE:    		db  	'JG OS Boot Loader Begin.', 0 ; ����� �޽��� ����
DISK_ERROR_MESSAGE:       	db  	'DISK Error', 0
IMAGE_LOADING_MESSAGE:    	db  	'OS Image Loading...', 0
LOADING_COMPLETE_MESSAGE: 	db  	'OS Image Complete', 0
CHANGE_GRAPHIC_MODE_FAIL:  	db  	'Change Graphic Mode Fail', 0

SECTORNUMBER:           	db  	0x02
HEADNUMBER:             	db  	0x00
TRACKNUMBER:            	db  	0x00

BOOTDRIVE:              	db 		0x00
MAXSECTOR:             		db 		0x00
MAXHEAD:               		db 		0x00
MAXTRACK:              		db 		0x00

times 510 - ( $ - $$ )    	db    	0x00

db 0x55
db 0xAA


