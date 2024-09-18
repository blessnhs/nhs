
[BITS 32]               ; ������ �ڵ�� 32��Ʈ �ڵ�� ����

global ReadCPUID, Change64BitKernel

SECTION .text       ; text ����(���׸�Ʈ)�� ����

ReadCPUID:
    push ebp
    mov ebp, esp
    push eax
    push ebx
    push ecx
    push edx
    push esi
    
    ;1ù�� �Ķ���͸� eax ���ڷ� �ް� cpuid ��ɾ� ȣ��
    mov eax, dword [ ebp + 8 ]
    cpuid
    
    ;2��° �Ķ���͸� esi�� �ְ� ��������
    ;���� ������ �ּҿ� ��������
    mov esi, dword [ ebp + 12 ]
    mov dword [ esi ], eax

    mov esi, dword [ ebp + 16 ]
    mov dword [ esi ], ebx


    mov esi, dword [ ebp + 20 ]
    mov dword [ esi ], ecx


    mov esi, dword [ ebp + 24 ]
    mov dword [ esi ], edx
                                

    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax
    pop ebp
    ret
    
Change64BitKernel:

    mov eax, cr4		;CR4 ��Ʈ�� ���������� ���� EAX�� ����
    or eax, 0x620   	;PAE��Ʈ�� 1�� ����
    mov cr4, eax    	;PAE��Ʈ�� 1�� ������ ���� ����
    
    mov eax, 0x100000	;PM4���̺� �ּҰ��� CR3 �������Ϳ� ����
    mov cr3, eax
    
    mov ecx, 0xC0000080 ;IA32_EFFER MSR �������� ��巹�� ����
    rdmsr
        
    or eax, 0x0101		;EAX�������Ϳ� ����� MSR�� ���� 32��Ʈ���� LME ��Ʈ 1�� ����
    wrmsr

    mov eax, cr0		;CR0 �������� ����
    or eax, 0xE000000E

    xor eax, 0x60000004

    mov cr0, eax
    
    jmp 0x08:0x200000	;64��Ʈ ���׸�Ʈ �����ͷ� ��ü

