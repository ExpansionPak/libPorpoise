//----------------------------------------------------------------------
//	(C)2000-2001 Nintendo
//----------------------------------------------------------------------
#include <dolphin.h>
#include <dolphin/tty.h>
#include <dolphin/fio.h>
#include <string.h>
#include <ctype.h>
#include "bmp.h"
#include "viewerDraw.h"
#include "viewerMcc.h"

#define	VIEWER_MAX_FILENAME_LENGTH	(512)	// Must be a multiple of 32
#define	VIEWER_IMAGE_BUFFER_SIZE	(0x100000)

#define NTSC	// if you change here, you can see PAL or M/PAL
//#define NON_INTERLACE	// if you define this, you can see non-interlace
#if defined(NTSC)
# ifndef NON_INTERLACE
static GXRenderModeObj* rmode = &GXNtsc480Int;
# else
static GXRenderModeObj* rmode = &GXNtsc240Ds;
# endif
#elif defined(PAL)
# ifndef NON_INTERLACE
static GXRenderModeObj* rmode = &GXPal528Int;
# else
static GXRenderModeObj* rmode = &GXPal264Ds;
# endif
#else
# ifndef NON_INTERLACE
static GXRenderModeObj* rmode = &GXMpal480Int;
# else
static GXRenderModeObj* rmode = &GXMpal240Ds;
# endif
#endif

// Color data table
typedef struct {
	u32		Y;
	u32		Cb;
	u32		Cr;
} VIEWER_COLOR_S;

VIEWER_COLOR_S	viewerColor[] = {
	{ 180 , 128 , 128 } ,	// 0:white
	{ 162 ,  44 , 142 } ,	// 1:yellow
	{ 131 , 156 ,  44 } ,	// 2:light blue
	{ 112 ,  72 ,  58 } ,	// 3:green
	{  84 , 184 , 198 } ,	// 4:purple
	{  65 , 100 , 212 } ,	// 5:red
	{  35 , 212 , 114 } ,	// 6:blue
	{  16 , 128 , 128 } ,	// 7:black
};

enum {
	VIEWER_FILE_OTHERS = 0 ,
	VIEWER_FILE_BMP
};

static u32		fbWidth;
static u32		fbSize;
static u8*		xfb1;
static u8*		xfb2;
static u32		xfbSwitch;	// For frame buffer selection
static u8*		xfb;		//
static char		filenameBuffer[VIEWER_MAX_FILENAME_LENGTH] ATTRIBUTE_ALIGN(32);

static void allocateFB( u32 fbSize );
static u32 getScreenWidth( void );
static u32 getScreenHeight( void );
static void fbFill( u8* xfb , u32 colorVal );
static void clearScreen( void );
static u32 convertColorValue( u8 colorCode );
static BOOL bmpDraw( u8* fileDataBuffer );
static void drawImageData( u8 picFile , u8* imgDataBuffer , u32 imgWidth , u32 imgHeight );
static void bmpCopyImage( u8* xfb , u8* img , u32 drawX , u32 drawY , u32 imgX , u32 imgY );
static BOOL bmpDrawCheck( bmpInfo_s* bInfo );
static int myStrcmp( char* str1 , char* str2 );
static u8 filenameCheck( char* fname );
static BOOL checkFileBmp( char* dataBuf );

//----------------------------------------------------------------------
//	Rendering initialization
//
void viewerDrawInit( void ) {

	xfbSwitch = 0;

	fbWidth = (u32)VIPadFrameBufferWidth( rmode->fbWidth ) * VI_DISPLAY_PIX_SZ;
	fbSize = (u32)(fbWidth * rmode->xfbHeight);
	allocateFB( fbSize );	// Frame buffer assignment

	// VI setting
	VIConfigure( rmode );
	VIFlush();
	VIWaitForRetrace();
#ifdef NON_INTERLACE
	VIWaitForRetrace();
#endif

	VISetBlack( TRUE );
	VIFlush();
	clearScreen();
	VISetBlack( FALSE );
	VIFlush();

	OSReport("screen size(width : %d , height : %d)\n", getScreenWidth() , getScreenHeight());
	TTYPrintf("screen size(width : %d , height : %d)\n", getScreenWidth() , getScreenHeight());
}

//----------------------------------------------------------------------
//	viewer draw main
//
void viewerDrawMain( void ) {

	BOOL		result = FALSE;
	u8			pictureFile;
	FIOHandle	fileHandle;
	u32			fileDataSize;
	u8*			fileDataBuffer;

	OSReport("\n");
	TTYPrintf("\n");

	clearScreen();

	// Get file name
	if( MCCRead( VIEWER_VIEWER_CHANNEL_ID , 0 , filenameBuffer , VIEWER_MAX_FILENAME_LENGTH , MCC_SYNC) == FALSE ) {
		OSReport("MCCRead() error\n");
		TTYPrintf("MCCRead() error\n");
	} else {
		OSReport("Received file name \"%s\"\n",filenameBuffer);
		TTYPrintf("Received file name \"%s\"\n",filenameBuffer);

		if( (fileHandle = FIOFopen( filenameBuffer , FIO_OPEN_RDONLY )) == FIO_INVALID_HANDLE ) {
			OSReport("FIOFopen() error\n");
			TTYPrintf("FIOFopen() error\n");
		} else {
			// Find data size
			fileDataSize = FIOFseek( fileHandle , 0 , FIO_SEEK_LAST );
			FIOFseek( fileHandle , 0 , FIO_SEEK_TOP );
			fileDataBuffer = (u8*)OSAlloc( OSRoundUp32B( fileDataSize ) );

			if( FIOFread( fileHandle , fileDataBuffer , fileDataSize ) == 0xFFFFFFFF ) {
				OSReport("FIOFread() error\n");
				TTYPrintf("FIOFread() error\n");
			} else {
				pictureFile = filenameCheck( filenameBuffer );
				switch( pictureFile ) {
					case	VIEWER_FILE_BMP:
						result = bmpDraw( fileDataBuffer );
						break;
					default:
						OSReport("Unsupported file\n");
						TTYPrintf("Unsupported file\n");
						break;
				}
			}
			// Close file
			if( FIOFclose( fileHandle ) == FALSE ) {
				OSReport("FIOFclose() error\n");
				TTYPrintf("FIOFclose() error\n");
				result = FALSE;
			}
			OSFree( fileDataBuffer );
		}
	}
	if( result == FALSE ) {
		viewerMccWrite( VIEWER_MCC_NOTIFY_ERROR );
	} else {
		viewerMccWrite( VIEWER_MCC_NOTIFY_COMPLETE );
	}
}

//----------------------------------------------------------------------
//
static void allocateFB( u32 fbSize ) {

	void*	arenaLo;
	void*	arenaHi;

	arenaLo = OSGetArenaLo();
	arenaHi = OSGetArenaHi();

	xfb1 = (u8*)OSRoundUp32B( arenaLo );
	xfb2 = (u8*)OSRoundUp32B( xfb1 + fbSize );

	arenaLo = (void*)(xfb1 + 2 * fbSize);
	OSSetArenaLo( arenaLo );

	arenaLo = OSInitAlloc( arenaLo, arenaHi, 1 );
	OSSetArenaLo( arenaLo );

	OSSetCurrentHeap( OSCreateHeap( (void*)OSRoundUp32B( arenaLo ),
									(void*)OSRoundDown32B( arenaHi ) ) );

	OSSetArenaLo( arenaLo = arenaHi );
}

//----------------------------------------------------------------------
//	Return display width
//
static u32 getScreenWidth( void ) {

	return( rmode->fbWidth );
}

//----------------------------------------------------------------------
//	Return display height
//
static u32 getScreenHeight( void ) {

	return( rmode->efbHeight );
}

//----------------------------------------------------------------------
//	Fill with specified color
//
static void fbFill( u8* xfb , u32 colorVal ) {

	u8*		ptr;

	for( ptr=xfb ; ptr<xfb + fbSize ; ptr += (VI_DISPLAY_PIX_SZ * 2) ) {
		*(u32*)ptr = colorVal;
	}
	DCStoreRange( (void*)xfb , fbSize );
}

//----------------------------------------------------------------------
//	Clear screen
//
static void clearScreen( void ) {

	xfb = (xfbSwitch & 0x01) ? xfb2 : xfb1;
	fbFill( xfb , convertColorValue( 7 ) );		// Fill with black
	VISetNextFrameBuffer( (void*)xfb );
	VIFlush();
	VIWaitForRetrace();
	xfbSwitch++;
}

//----------------------------------------------------------------------
//	Color code to color data conversion
//
static u32 convertColorValue( u8 colorCode ) {

	return( (viewerColor[colorCode].Y << 24) + \
			(viewerColor[colorCode].Cb << 16) + \
			(viewerColor[colorCode].Y << 8) + \
			viewerColor[colorCode].Cr );
}

//----------------------------------------------------------------------
//	BMP rendering
//
static BOOL bmpDraw( u8* fileDataBuffer ) {

	bmpInfo_s	bmpInfo;
	BOOL		result = FALSE;
	u8*			imageBuffer;

	// bmp file check
	if( checkFileBmp( (char*)fileDataBuffer ) == FALSE ) {
		OSReport("checkFileBmp() error\n");
		TTYPrintf("checkFileBmp() error\n");
	} else {
		// bmp file information setting
		if( openBmp( &bmpInfo , fileDataBuffer ) == FALSE ) {
			OSReport("openBmp() error\n");
			TTYPrintf("openBmp() error\n");
		} else {
			OSReport("width : %d , height : %d\n",bmpInfo.width , bmpInfo.height);
			TTYPrintf("width : %d , height : %d\n",bmpInfo.width , bmpInfo.height);
			if( bmpDrawCheck( &bmpInfo ) == FALSE ) {
			} else {
				imageBuffer = (u8*)OSAlloc( OSRoundUp32B(VIEWER_IMAGE_BUFFER_SIZE ) );
				if( bmpToYCbCr( &bmpInfo , fileDataBuffer , imageBuffer ) == FALSE ) {
					OSReport("Data conversion failed\n");
					TTYPrintf("Data conversion failed\n");
				} else {
					drawImageData( VIEWER_FILE_BMP , imageBuffer , bmpInfo.width , bmpInfo.height );
					result = TRUE;
				}
				OSFree( imageBuffer );
			}
		}
	}
	return( result );
}

//----------------------------------------------------------------------
//	Render BMP data
//
static void drawImageData( u8 picFile , u8* imgDataBuffer , u32 imgWidth , u32 imgHeight ) {

	u32		drawStartX , drawStartY;

	xfb = (xfbSwitch & 0x01) ? xfb2 : xfb1;
	fbFill( xfb , convertColorValue( 7 ) );

	drawStartX = ((rmode->fbWidth - imgWidth) / 2) & (~1);
	drawStartY = ((rmode->efbHeight - imgHeight) / 2) & (~1);

	switch( picFile ) {
		case	VIEWER_FILE_BMP:
			bmpCopyImage( xfb , imgDataBuffer , drawStartX , drawStartY , imgWidth , imgHeight );
			break;
		default:
			break;
	}

	VISetNextFrameBuffer( (void*)xfb );
	VIFlush();
	VIWaitForRetrace();
	xfbSwitch++;
}

//----------------------------------------------------------------------
//	Copy image data to frame buffer
//
static void bmpCopyImage( u8* xfb , u8* img , u32 drawX , u32 drawY , u32 imgX , u32 imgY ) {

	u8		*ptr;
	u32		x , y;

	imgX = ((imgX + 16 - 1) & ~(16 - 1));
	for( y=0 ; y<imgY ; y++ ){
		ptr = xfb + (2 * drawX) + (fbWidth * (drawY + y));
		for( x=0 ; x<(imgX / 2) ; x++ ) {
			*(u32*)ptr = *(u32*)img;
			img += (VI_DISPLAY_PIX_SZ * 2);
			ptr += (VI_DISPLAY_PIX_SZ * 2);
		}
	}
	DCStoreRange( (void*)xfb , fbSize );
}

//----------------------------------------------------------------------
//	Check whether rendering possible
//
static BOOL bmpDrawCheck( bmpInfo_s* bInfo ) {

	if( bInfo->biCompression != 0 ) {
		OSReport("Compression ratio (%d) not supported\n",bInfo->biCompression);
		TTYPrintf("Compression ratio (%d) not supported\n",bInfo->biCompression);
		return( FALSE );
	}

	if( (bInfo->width > getScreenWidth()) || (bInfo->height > getScreenHeight()) ) {
		OSReport("Too big for screen size\n");
		TTYPrintf("Too big for screen size\n");
		return( FALSE );
	}
	return( TRUE );
}

//----------------------------------------------------------------------
//	Determine file type from extension
//
static u8 filenameCheck( char* fname ) {

	s16		i;
	s16		period;

	period = -1;
	for( i=(s16)strlen( fname ) ; i>=0 ; i-- ) {
		if( fname[i] == '.' ) {
			period = i;
			break;
		}
	}
	if( period == -1 ) {
		return( VIEWER_FILE_OTHERS );
	}

	if( myStrcmp( &fname[period+1] , "bmp" ) == 0 ) {
		return( VIEWER_FILE_BMP );
	}
	return( VIEWER_FILE_OTHERS );
}

//----------------------------------------------------------------------
//	myStrcmp
//
static int myStrcmp( char* str1 , char* str2 ) {

	while( *str1 != '\0' ) {
		if( tolower( *str1 ) != tolower( *str2 ) ) {
			return( tolower( *str1 ) - tolower( *str2 ) );
		}
		str1++;
		str2++;
	}
	return( 0 );
}

//----------------------------------------------------------------------
//	BMP file header check
//
static BOOL checkFileBmp( char* dataBuf ) {

	if( myStrcmp( "BM" , dataBuf ) == 0 ) {
		return( TRUE );
	}
	return( FALSE );
}
