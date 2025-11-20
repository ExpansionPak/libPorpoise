//	HIO SAMPLE PROCRAM - color - VI module
//


////////////////
// INCLUDE

#include <dolphin.h>
#include <stdio.h>


////////////////
// VARIABLES

#define NTSC      // if you change here, you can see PAL or M/PAL
/* #define NON_INTERLACE */   // if you define this, you can see non-interlace
static GXRenderModeObj* rmode = 
#if defined(NTSC)
#	ifndef NON_INTERLACE
		&GXNtsc480Int;
#	else
		&GXNtsc240Ds;
#	endif
#elif defined(PAL)
#	ifndef NON_INTERLACE
		&GXPal528Int;
#	else
		&GXPal264Ds;
#	endif
#else
#	ifndef NON_INTERLACE
		&GXMpal480Int;
#	else
		&GXMpal240Ds;
#	endif
#endif

//for X frame buffer 
static u8	*xfb1,*xfb2,*xfb;
static u32	gFbWidth,gFbSize;
static BOOL gFirst=TRUE;
static BOOL gFrame=FALSE;

extern u16 font_table[(0x80-0x20)][24];

////////////////
// PROTOTYPES

void colorVi_Initialize( );
void colorVi_Paint( u8 nRed , u8 nGreen , u8 nBlue , BOOL bMode );

static void allocateFB( u32 fbSize );
static u32 makeColor( u8 colorR , u8 colorG , u8 colorB );
static void fillColor( u32 colorVal );
static void setPixel( u32 colorVal, u32 x, u32 y );
static void paintBox( u32 colorVal, u32 x, u32 y, u32 w, u32 h );
static void copyImaget( u8* image, u32 x, u32 y, u32 w, u32 h );

static void drawChar( u32 colorValue, u32 x, u32 y, u8 chr );
static void drawString( u32 colorValue, u32 x, u32 y, char *str );


////////////////////////////////
// PUBLIC FUNCTIONS

void colorVi_Initialize( )
{	// 320 x 480
	// Calculate frame buffer size.
	// Note that each line width should be a multiple of 16.
	gFbWidth = (u32)( VIPadFrameBufferWidth(rmode->fbWidth) * VI_DISPLAY_PIX_SZ );
	gFbSize	 = (u32)( gFbWidth * rmode->xfbHeight );

	allocateFB(gFbSize);

	VIConfigure(rmode);

	// Need to "flush" so that the VI changes so far takes effect
	// from the following field
	VIFlush();
	VIWaitForRetrace();

	// Since the tv mode is interlace after VIInit,
	// we need to wait for one more frame to make sure
	// that the mode is switched from interlace to non-interlace
#ifdef NON_INTERLACE
	VIWaitForRetrace();
#endif
}

void colorVi_Paint( u8 nRed , u8 nGreen , u8 nBlue , BOOL bMode )
{	
	xfb = (gFrame)? xfb2 : xfb1;

	fillColor( makeColor( 0, 0, 0 ) );
	paintBox( makeColor( nRed, nGreen, nBlue ), 40, 80, 240, 270 );
	drawString( makeColor( 255, 255, 255 ), 32, 32, "HIO Sample,COLOR" );


	if(bMode){
		char str[64];
		sprintf(str,"R:%03d G:%03d B:%03d",nRed,nGreen,nBlue);
		drawString( 
			makeColor( (u8)(255-nRed), (u8)(255-nGreen), (u8)(255-nBlue) ), 
			24, 320, str );
	}

	VISetNextFrameBuffer((void*)xfb);

	if (gFirst){
		VISetBlack(FALSE);
		gFirst = FALSE;
	}
	VIFlush();
	VIWaitForRetrace();

	gFrame = !gFrame;
}


////////////////////////////////
// PRIVATE FUNCTIONS

static void allocateFB(u32 fbSize)
{
    void* arenaLo;

    arenaLo = OSGetArenaLo();

    // allocate memory for frame buffer here.
    xfb1 = (u8*)OSRoundUp32B(arenaLo);
    xfb2 = (u8*)OSRoundUp32B(xfb1 + fbSize);

    arenaLo = (void*)(xfb1 + 2 * fbSize);
    OSSetArenaLo(arenaLo);
}

#define CLAMP(x,l,h) ((x > h) ? h : ((x < l) ? l : x))
static u32 makeColor( u8 colorR , u8 colorG , u8 colorB ) 
{
	u32  colorY , colorCr , colorCb , colorVal;
	double Y,Cr,Cb;

	Y  =  0.257 * colorR + 0.504 * colorG + 0.098 * colorB +  16.0 + 0.5;
	Cb = -0.148 * colorR - 0.291 * colorG + 0.439 * colorB + 128.0 + 0.5;
	Cr =  0.439 * colorR - 0.368 * colorG - 0.071 * colorB + 128.0 + 0.5;

    Y  = CLAMP(Y , 16, 235);
    Cb = CLAMP(Cb, 16, 240);
    Cr = CLAMP(Cr, 16, 240);

	colorY = (u32)Y;
	colorCr = (u32)Cr;
	colorCb = (u32)Cb;

	colorVal = 
		( colorY << 24 ) +
		( colorCb << 16 ) +
		( colorY << 8 ) +
		colorCr;

	return( colorVal );
}

// fill screen
static void fillColor( u32 colorVal )
{	u8*	ptr;
	u32	pixSize = (VI_DISPLAY_PIX_SZ << 1);
	for (ptr = xfb; ptr < xfb + gFbSize; ptr += pixSize)
		*(u32*)ptr = colorVal;
    DCStoreRange((void*)xfb, gFbSize);
}

// set dot
static void setPixel(
	u32 colorVal, u32 x, u32 y )
{	u8	*ptr = xfb + ((VI_DISPLAY_PIX_SZ<<1) * x) + (gFbWidth * y);
	*(u32*)ptr = colorVal;
}

// draw fill box
static void paintBox(
	u32 colorVal, u32 x, u32 y, u32 w, u32 h )
{	u8	*ptr,*sPtr;
	u32	pixSize = (VI_DISPLAY_PIX_SZ << 1);
	u32	lineSize = pixSize * w;
	u32	i;

	for(i=0;i<h;i++){
		sPtr = xfb + (pixSize * x) + (gFbWidth * (y+i));
		for (ptr = sPtr; ptr < sPtr + lineSize; ptr += pixSize)
			*(u32*)ptr = colorVal;
	}
    DCStoreRange((void*)xfb, gFbSize);
}

//	copy image
static void copyImaget(
	u8* image, u32 x, u32 y, u32 w, u32 h )
{	u8	*ptr,*sPtr;
	u32	pixSize = (VI_DISPLAY_PIX_SZ << 1);
	u32	lineSize = pixSize * w;
	u32	i;

	for(i=0;i<h;i++){
		sPtr = xfb + (pixSize * x) + (gFbWidth * (y+i));
		for (ptr = sPtr; ptr < sPtr + lineSize; ptr += pixSize)
			*(u32*)ptr = *(u32*)image,
			image += pixSize;
	}
}

//	draw a character
static void drawChar( u32 colorValue, u32 x, u32 y, u8 chr )
{
	int line,bit;
	for( line=0; line<24; line++ ){
		u16 lineImage = font_table[chr-0x20][line];
		for( bit=0; bit<16; bit++ ){
			if(lineImage&0x8000)
				setPixel( colorValue, x+bit, y+line );
			lineImage<<=1;
		}
	}
}

//	draw a string
static void drawString( u32 colorValue, u32 x, u32 y, char *str )
{
	while(*str!='\0'){
		drawChar( colorValue, x, y, (u8)str[0] );
		x+=16;
		str++;
	}
}