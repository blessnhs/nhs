/**
 *  file    Types.h
 *  date    2008/12/13
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   ���� �������� ����ϴ� ���� Ÿ���̳� �ڷᱸ���� ������ ��� ����
 */

#ifndef __TYPES_H__
#define __TYPES_H__

////////////////////////////////////////////////////////////////////////////////
//
// ��ũ��
//
////////////////////////////////////////////////////////////////////////////////
//==============================================================================
//  �⺻ Ÿ�� ���� ��ũ��
//==============================================================================
#define BYTE    unsigned char
#define WORD    unsigned short
#define DWORD   unsigned int
#define QWORD   unsigned long
#define BOOL    unsigned char

#define TRUE    1
#define FALSE   0
#define NULL    0

//==============================================================================
//  �ܼ� ���� ��ũ��
//==============================================================================
// �ܼ��� �ʺ�(Width)�� ����(Height),�׸��� ���� �޸��� ���� ��巹�� ����
#define CONSOLE_WIDTH                       80
#define CONSOLE_HEIGHT                      25


//==============================================================================
//  Ű���忡 ���õ� ��ũ��
//==============================================================================
// Ű ���¿� ���� �÷���
#define KEY_FLAGS_UP             0x00
#define KEY_FLAGS_DOWN           0x01
#define KEY_FLAGS_EXTENDEDKEY    0x02

// ��ĵ �ڵ� ���� ���̺��� ���� ��ũ��
#define KEY_NONE        0x00
#define KEY_ENTER       '\n'
#define KEY_TAB         '\t'
#define KEY_ESC         0x1B
#define KEY_BACKSPACE   0x08

#define KEY_CTRL        0x81
#define KEY_LSHIFT      0x82
#define KEY_RSHIFT      0x83
#define KEY_PRINTSCREEN 0x84
#define KEY_LALT        0x85
#define KEY_CAPSLOCK    0x86
#define KEY_F1          0x87
#define KEY_F2          0x88
#define KEY_F3          0x89
#define KEY_F4          0x8A
#define KEY_F5          0x8B
#define KEY_F6          0x8C
#define KEY_F7          0x8D
#define KEY_F8          0x8E
#define KEY_F9          0x8F
#define KEY_F10         0x90
#define KEY_NUMLOCK     0x91
#define KEY_SCROLLLOCK  0x92
#define KEY_HOME        0x93
#define KEY_UP          0x94
#define KEY_PAGEUP      0x95
#define KEY_LEFT        0x96
#define KEY_CENTER      0x97
#define KEY_RIGHT       0x98
#define KEY_END         0x99
#define KEY_DOWN        0x9A
#define KEY_PAGEDOWN    0x9B
#define KEY_INS         0x9C
#define KEY_DEL         0x9D
#define KEY_F11         0x9E
#define KEY_F12         0x9F
#define KEY_PAUSE       0xA0

//==============================================================================
//  �½�ũ�� �����ٷ� ���� ��ũ��
//==============================================================================
// �½�ũ�� �켱 ����
#define TASK_FLAGS_HIGHEST            0
#define TASK_FLAGS_HIGH               1
#define TASK_FLAGS_MEDIUM             2
#define TASK_FLAGS_LOW                3
#define TASK_FLAGS_LOWEST             4
#define TASK_FLAGS_WAIT               0xFF          

// �½�ũ�� �÷���
#define TASK_FLAGS_ENDTASK            0x8000000000000000
#define TASK_FLAGS_SYSTEM             0x4000000000000000
#define TASK_FLAGS_PROCESS            0x2000000000000000
#define TASK_FLAGS_THREAD             0x1000000000000000
#define TASK_FLAGS_IDLE               0x0800000000000000
#define TASK_FLAGS_USERLEVEL          0x0400000000000000

// �Լ� ��ũ��
#define GETPRIORITY( x )            ( ( x ) & 0xFF )
#define SETPRIORITY( x, priority )  ( ( x ) = ( ( x ) & 0xFFFFFFFFFFFFFF00 ) | \
        ( priority ) )
#define GETTCBOFFSET( x )           ( ( x ) & 0xFFFFFFFF )

// ��ȿ���� ���� �½�ũ ID
#define TASK_INVALIDID              0xFFFFFFFFFFFFFFFF

// ���μ��� ģȭ�� �ʵ忡 �Ʒ��� ���� �����Ǹ�, �ش� �½�ũ�� Ư���� �䱸������ ���� 
// ������ �Ǵ��ϰ� �½�ũ ���� �л� ����
#define TASK_LOADBALANCINGID        0xFF

//==============================================================================
//  ���� �ý��� ���� ��ũ��
//==============================================================================
// ���� �̸��� �ִ� ����
#define FILESYSTEM_MAXFILENAMELENGTH        24

// SEEK �ɼ� ����
#define SEEK_SET                            0
#define SEEK_CUR                            1
#define SEEK_END                            2

// MINT ���� �ý��� Ÿ�԰� �ʵ带 ǥ�� ������� Ÿ������ ������
#define size_t      DWORD       
#define dirent      DirectoryEntryStruct
#define d_name      vcFileName


//==============================================================================
//  GUI �ý��� ���� ��ũ��
//==============================================================================
// ���� �����ϴµ� ����� �ڷᱸ��, 16��Ʈ ���� ����ϹǷ� WORD�� ����
typedef WORD    COLOR;

// 0~255 ������ R, G, B�� 16��Ʈ �� �������� ��ȯ�ϴ� ��ũ��
// 0~255�� ������ 0~31, 0~63���� ����Ͽ� ����ϹǷ� ���� 8�� 4�� ��������� ��
// ������ 8�� ������ 4�� >> 3�� >> 2�� ��ü
#define RGB( r, g, b )      ( ( ( BYTE )( r ) >> 3 ) << 11 | \
                ( ( ( BYTE )( g ) >> 2 ) ) << 5 |  ( ( BYTE )( b ) >> 3 ) )

// ������ ������ �ִ� ����
#define WINDOW_TITLEMAXLENGTH       40

// ��ȿ���� ���� ������ ID
#define WINDOW_INVALIDID            0xFFFFFFFFFFFFFFFF

// �������� �Ӽ�
// �����츦 ȭ�鿡 ��Ÿ��
#define WINDOW_FLAGS_SHOW               0x00000001
// ������ �׵θ� �׸�
#define WINDOW_FLAGS_DRAWFRAME          0x00000002
// ������ ���� ǥ���� �׸�
#define WINDOW_FLAGS_DRAWTITLE          0x00000004
// ������ ũ�� ���� ��ư�� �׸�
#define WINDOW_FLAGS_RESIZABLE          0x00000008
// ������ �⺻ �Ӽ�, ���� ǥ���ٰ� �������� ��� �׸��� ȭ�鿡 �����츦 ���̰� ����
#define WINDOW_FLAGS_DEFAULT        ( WINDOW_FLAGS_SHOW | WINDOW_FLAGS_DRAWFRAME | \
                                      WINDOW_FLAGS_DRAWTITLE )

// ���� ǥ������ ����
#define WINDOW_TITLEBAR_HEIGHT      21
// �������� �ݱ� ��ư�� ũ��
#define WINDOW_XBUTTON_SIZE         19
// �������� �ּ� �ʺ�, ��ư 2���� �ʺ� 30�ȼ��� ���� ���� Ȯ��
#define WINDOW_WIDTH_MIN            ( WINDOW_XBUTTON_SIZE * 2 + 30 )
// �������� �ּ� ����, ���� ǥ������ ���̿� 30�ȼ��� ���� ���� Ȯ��
#define WINDOW_HEIGHT_MIN           ( WINDOW_TITLEBAR_HEIGHT + 30 )

// �������� ����
#define WINDOW_COLOR_FRAME                      RGB( 109, 218, 22 )
#define WINDOW_COLOR_BACKGROUND                 RGB( 255, 255, 255 )
#define WINDOW_COLOR_TITLEBARTEXT               RGB( 255, 255, 255 )
#define WINDOW_COLOR_TITLEBARACTIVEBACKGROUND   RGB( 79, 204, 11 )
#define WINDOW_COLOR_TITLEBARINACTIVEBACKGROUND RGB( 55, 135, 11 )
#define WINDOW_COLOR_TITLEBARBRIGHT1            RGB( 183, 249, 171 )
#define WINDOW_COLOR_TITLEBARBRIGHT2            RGB( 150, 210, 140 )
#define WINDOW_COLOR_TITLEBARUNDERLINE          RGB( 46, 59, 30 )
#define WINDOW_COLOR_BUTTONBRIGHT               RGB( 229, 229, 229 )
#define WINDOW_COLOR_BUTTONDARK                 RGB( 86, 86, 86 )
#define WINDOW_COLOR_SYSTEMBACKGROUND           RGB( 232, 255, 232 )
#define WINDOW_COLOR_XBUTTONLINECOLOR           RGB( 71, 199, 21 )

// ��� �������� ����
#define WINDOW_BACKGROUNDWINDOWTITLE            "SYS_BACKGROUND"

// ������� ������ �Ŵ��� �½�ũ ���̿��� ���޵Ǵ� �̺�Ʈ�� ����
// ���콺 �̺�Ʈ
#define EVENT_UNKNOWN                                   0
#define EVENT_MOUSE_MOVE                                1
#define EVENT_MOUSE_LBUTTONDOWN                         2
#define EVENT_MOUSE_LBUTTONUP                           3
#define EVENT_MOUSE_RBUTTONDOWN                         4
#define EVENT_MOUSE_RBUTTONUP                           5
#define EVENT_MOUSE_MBUTTONDOWN                         6
#define EVENT_MOUSE_MBUTTONUP                           7
// ������ �̺�Ʈ
#define EVENT_WINDOW_SELECT                             8
#define EVENT_WINDOW_DESELECT                           9
#define EVENT_WINDOW_MOVE                               10
#define EVENT_WINDOW_RESIZE                             11
#define EVENT_WINDOW_CLOSE                              12
// Ű �̺�Ʈ
#define EVENT_KEY_DOWN                                  13
#define EVENT_KEY_UP                                    14

// ���� ��Ʈ�� �ʺ�� ����
#define FONT_ENGLISHWIDTH   8
#define FONT_ENGLISHHEIGHT  16

//==============================================================================
//  ��Ÿ ��ũ��
//==============================================================================
#define MIN( x, y )     ( ( ( x ) < ( y ) ) ? ( x ) : ( y ) )
#define MAX( x, y )     ( ( ( x ) > ( y ) ) ? ( x ) : ( y ) )

////////////////////////////////////////////////////////////////////////////////
//
// ����ü
//
////////////////////////////////////////////////////////////////////////////////
#pragma pack( push, 1 )

//==============================================================================
//  Ű���忡 ���õ� �ڷᱸ��
//==============================================================================
// Ű ť�� ������ ������ ����ü
typedef struct KeyDataStruct
{
    // Ű���忡�� ���޵� ��ĵ �ڵ�
    BYTE bScanCode;
    // ��ĵ �ڵ带 ��ȯ�� ASCII �ڵ�
    BYTE bASCIICode;
    // Ű ���¸� �����ϴ� �÷���(����/������/Ȯ�� Ű ����)
    BYTE bFlags;
} KEYDATA;

//==============================================================================
//  ���� �ý��ۿ� ���õ� �ڷᱸ��
//==============================================================================
// ���͸� ��Ʈ�� �ڷᱸ��
typedef struct DirectoryEntryStruct
{
    // ���� �̸�
    char vcFileName[ FILESYSTEM_MAXFILENAMELENGTH ];
    // ������ ���� ũ��
    DWORD dwFileSize;
    // ������ �����ϴ� Ŭ������ �ε���
    DWORD dwStartClusterIndex;
} DIRECTORYENTRY;

#pragma pack( pop )

// ������ �����ϴ� ���� �ڵ� �ڷᱸ��
typedef struct kFileHandleStruct
{
    // ������ �����ϴ� ���͸� ��Ʈ���� ������
    int iDirectoryEntryOffset;
    // ���� ũ��
    DWORD dwFileSize;
    // ������ ���� Ŭ������ �ε���
    DWORD dwStartClusterIndex;
    // ���� I/O�� �������� Ŭ�������� �ε���
    DWORD dwCurrentClusterIndex;
    // ���� Ŭ�������� �ٷ� ���� Ŭ�������� �ε���
    DWORD dwPreviousClusterIndex;
    // ���� �������� ���� ��ġ
    DWORD dwCurrentOffset;
} FILEHANDLE;

// ���͸��� �����ϴ� ���͸� �ڵ� �ڷᱸ��
typedef struct kDirectoryHandleStruct
{
    // ��Ʈ ���͸��� �����ص� ����
    DIRECTORYENTRY* pstDirectoryBuffer;
    
    // ���͸� �������� ���� ��ġ
    int iCurrentOffset;
} DIRECTORYHANDLE;




#define FATFS_MAX_LONG_FILENAME         	260
#define FAT_SECTOR_SIZE                     512


struct cluster_lookup
{
	unsigned int ClusterIdx;
	unsigned int CurrentCluster;
};

struct fat_node
{
    struct fat_node    *previous;
    struct fat_node    *next;
};

// ���ϰ� ���͸��� ���� ������ ����ִ� �ڷᱸ��
typedef struct kFileDirectoryHandleStruct
{

	unsigned int                  	parentcluster;
	unsigned int                  	startcluster;
	unsigned int                  	bytenum;
	unsigned int                  	filelength;
	int                     		filelength_changed;
	char                    		path[FATFS_MAX_LONG_FILENAME];
	char                    		filename[FATFS_MAX_LONG_FILENAME];
	unsigned char                   shortfilename[11];

	#ifdef FAT_CLUSTER_CACHE_ENTRIES
	//    uint32                  cluster_cache_idx[FAT_CLUSTER_CACHE_ENTRIES];
	//    uint32                  cluster_cache_data[FAT_CLUSTER_CACHE_ENTRIES];
	#endif

	    // Cluster Lookup
	    struct cluster_lookup   last_fat_lookup;

	    // Read/Write sector buffer
	    unsigned char                   file_data_sector[FAT_SECTOR_SIZE];
	    unsigned int                  file_data_address;
	    int                     file_data_dirty;

	    // File fopen flags
	    unsigned char                   flags;

	    struct fat_node         list_node;
} FILE;

typedef struct fs_dir_ent
{
    char                    filename[FATFS_MAX_LONG_FILENAME];
    unsigned char           is_dir;
    unsigned int            cluster;
    unsigned int            size;

#if FATFS_INC_TIME_DATE_SUPPORT
    uint16                  access_date;
    uint16                  write_time;
    uint16                  write_date;
    uint16                  create_date;
    uint16                  create_time;
#endif
}DIRECTORY;

//==============================================================================
//  GUI �ý��ۿ� ���õ� �ڷᱸ��
//==============================================================================
// �簢���� ������ ��� �ڷᱸ��
typedef struct kRectangleStruct
{
    // ���� ��(������)�� X ��ǥ
    int iX1;
    // ���� ��(������)�� Y ��ǥ
    int iY1;
    
    // ������ �Ʒ�(����)�� X ��ǥ
    int iX2;
    // ������ �Ʒ�(����)�� Y��ǥ
    int iY2;
} RECT;

// ���� ������ ��� �ڷᱸ��
typedef struct kPointStruct
{
    // X�� Y�� ��ǥ
    int iX;
    int iY;
} POINT;

// ���콺 �̺�Ʈ �ڷᱸ��
typedef struct kMouseEventStruct
{
    // ������ ID
    QWORD qwWindowID;

    // ���콺 X,Y��ǥ�� ��ư�� ����
    POINT stPoint;
    BYTE bButtonStatus;
} MOUSEEVENT;

// Ű �̺�Ʈ �ڷᱸ��
typedef struct kKeyEventStruct
{
    // ������ ID
    QWORD qwWindowID;
    
    // Ű�� ASCII �ڵ�� ��ĵ �ڵ�
    BYTE bASCIICode;
    BYTE bScanCode;    
    
    // Ű �÷���
    BYTE bFlags;
} KEYEVENT;

// ������ �̺�Ʈ �ڷᱸ��
typedef struct kWindowEventStruct
{
    // ������ ID
    QWORD qwWindowID;
    
    // ���� ����
    RECT stArea;
} WINDOWEVENT;

// �̺�Ʈ �ڷᱸ��
typedef struct kEventStruct
{
    // �̺�Ʈ Ÿ��
    QWORD qwType;
    
    // �̺�Ʈ ������ ������ ������ ����ü
    union
    {
        // ���콺 �̺�Ʈ ���� ������
        MOUSEEVENT stMouseEvent;

        // Ű �̺�Ʈ ���� ������
        KEYEVENT stKeyEvent;

        // ������ �̺�Ʈ ���� ������
        WINDOWEVENT stWindowEvent;

        // ���� �̺�Ʈ �ܿ� ���� �̺�Ʈ�� ���� ������
        QWORD vqwData[ 3 ];
    };
} EVENT;

//==============================================================================
//  JPEG ���ڴ��� ���õ� �ڷᱸ��
//==============================================================================
// ������ ���̺�
typedef struct{
    int elem; // ��� ����
    unsigned short code[256];
    unsigned char  size[256];
    unsigned char  value[256];
}HUFF;

// JPEG ���ڵ��� ���� �ڷᱸ��
typedef struct{
    // SOF
    int width;
    int height;
    // MCU
    int mcu_width;
    int mcu_height;

    int max_h,max_v;
    int compo_count;
    int compo_id[3];
    int compo_sample[3];
    int compo_h[3];
    int compo_v[3];
    int compo_qt[3];

    // SOS
    int scan_count;
    int scan_id[3];
    int scan_ac[3];
    int scan_dc[3];
    int scan_h[3];  // ���ø� ��� ��
    int scan_v[3];  // ���ø� ��� ��
    int scan_qt[3]; // ����ȭ ���̺� �ε���
    
    // DRI
    int interval;

    int mcu_buf[32*32*4]; // ����
    int *mcu_yuv[4];
    int mcu_preDC[3];
    
    // DQT
    int dqt[3][64];
    int n_dqt;
    
    // DHT
    HUFF huff[2][3];
    
    
    // i/o
    unsigned char *data;
    int data_index;
    int data_size;
    
    unsigned long bit_buff;
    int bit_remain;
    
}JPEG;

#endif /*TYPES_H_*/