#ifndef __PIC_H__
#define __PIC_H__

#include "Types.h"

// I/O ��Ʈ ����
#define PIC_MASTER_PORT1    0x20
#define PIC_MASTER_PORT2    0x21
#define PIC_SLAVE_PORT1     0xA0
#define PIC_SLAVE_PORT2     0xA1

// IDT ���̺��� ���ͷ�Ʈ ���Ͱ� ���۵Ǵ� ��ġ, 0x20
#define PIC_IRQSTARTVECTOR  0x20

void InitializePIC( void );
void MaskPICInterrupt( WORD wIRQBitmask );
void SendEOIToPIC( int iIRQNumber );

#endif /*__PIC_H__*/
