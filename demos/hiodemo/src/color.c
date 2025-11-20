//	HIO SAMPLE PROCRAM - color
//


////////////////
// INCLUDE
#include <dolphin.h>
#include <dolphin/hio.h>
#include <stdio.h>

////////////////
// DEFINES

// HIO
#define HIO_ACCESS_BUFFER_SIZE	(32)
#define HIO_ACCESS_RGB_ADDRESS	(0x1000)
#define HIO_ACCESS_MSG_ADDRESS	(HIO_ACCESS_RGB_ADDRESS+HIO_ACCESS_BUFFER_SIZE)

#define HIO_ACCESS_DATA_IDX_RED		0
#define HIO_ACCESS_DATA_IDX_GREEN	1
#define HIO_ACCESS_DATA_IDX_BLUE	2
#define HIO_ACCESS_DATA_IDX_ACK		3

#define HIO_ACCESS_CMD_REQMSG	0x0000000F
#define HIO_ACCESS_CMD_CLRMSG	0x00000000

typedef void (*CallbackProc)( void* );

////////////////
// PROTOTYPES
// this
static BOOL hioEnumCallback( s32 chan );
static void hioCallback();
// color_vi.c
void colorVi_Initialize( );
void colorVi_Paint( u8 nRed , u8 nGreen , u8 nBlue , BOOL bMode );

// HIO
static void procInitializeHIO();
static void procReadMailbox();
static void procReadColorFromMemory(u8 *nRed,u8 *nGreen,u8 *nBlue);
static void procWriteMessageToMemory(u8 nRed,u8 nGreen,u8 nBlue);

////////////////
// VARIABLES
// HIO
static u8	gBuffer[HIO_ACCESS_BUFFER_SIZE] ATTRIBUTE_ALIGN(32);
static s32	exiChan=0;
static BOOL	gMessageMode=TRUE;


////////////////
// CALLBACKS

BOOL hioEnumCallback( s32 chan )
{	OSReport("USBAdapter EXI.%d\n",chan);
	exiChan=chan;
	return FALSE;
}

void hioCallback()
{
	procReadMailbox();
}


////////////////
// FUNCTIONS

void main(void)
{
	u8 nRed,nGreen,nBlue;
	//	os init
	OSInit();
	VIInit();

	//	initialize
	procInitializeHIO();
	colorVi_Initialize();
	//	main loop
	while(TRUE){
		//	read rgb value
		procReadColorFromMemory(&nRed,&nGreen,&nBlue);
		//	write message
		if(gMessageMode)
			procWriteMessageToMemory(nRed,nGreen,nBlue);
		//	vi proc
		colorVi_Paint( nRed , nGreen , nBlue , !gMessageMode );
	}
}




////////////////////////////////
// FUNCTIONS FOR HIO ACCESS

void procInitializeHIO()
{
	//	initialize
	HIOEnumDevices( hioEnumCallback );
	if(!HIOInit( exiChan, hioCallback )){
		OSHalt("HIOInit.NG\n");
	}else{
		// need to HIOReadMailbox for receive EXIINT
		procReadMailbox();
	}
}

void procReadMailbox()
{
	u32	mailbox=0;
	if(!HIOReadMailbox( &mailbox )){
		OSReport("HIOReadMailbox.NG\n");
	}else{
		if(mailbox==HIO_ACCESS_CMD_REQMSG){
			gMessageMode = TRUE;
			OSReport("MESSAGE MODE:ON\n");
		}else if(mailbox==HIO_ACCESS_CMD_CLRMSG){
			gMessageMode = FALSE;
			OSReport("MESSAGE MODE:OFF\n");
		}else{
			OSReport("Receive unknowwn message.(%08X)\n",mailbox);
		}
	}
}

void procReadColorFromMemory(u8 *nRed,u8 *nGreen,u8 *nBlue)
{
	//	read rgb value
	if(!HIORead( HIO_ACCESS_RGB_ADDRESS, gBuffer, HIO_ACCESS_BUFFER_SIZE )){
		OSHalt("HIORead.NG\n");
	}else{
		//	invalidate memory after read.
		DCInvalidateRange(gBuffer,32);
		//	result
		*nRed	= gBuffer[HIO_ACCESS_DATA_IDX_RED];
		*nGreen	= gBuffer[HIO_ACCESS_DATA_IDX_GREEN];
		*nBlue	= gBuffer[HIO_ACCESS_DATA_IDX_BLUE];
	}
}

void procWriteMessageToMemory(u8 nRed,u8 nGreen,u8 nBlue)
{
	//	make message to write
	sprintf((char*)gBuffer,"R:%03d, G:%03d, B:%03d", nRed, nGreen, nBlue);
	//	flush memory before write
	DCFlushRange(gBuffer,32);
	//	write message
	if(!HIOWrite( HIO_ACCESS_MSG_ADDRESS, gBuffer, HIO_ACCESS_BUFFER_SIZE )){
		OSHalt("HIOWrite.NG\n");
	}else{
	}
}
