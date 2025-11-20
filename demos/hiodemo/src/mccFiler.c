/*---------------------------------------------------------------------*

Project:  mcc bitmap viewer - filer module
File:     mccFiler.c

(C) 2000 Nintendo

-----------------------------------------------------------------------*/

// ============================================================
//  MCC BITMAP VIEWER - filer module
// ============================================================
// [Description]
//  File operation module for MCC bitmap viewer.

//  HEADER INCLUDE
// ================
#include <dolphin.h>
#include <dolphin/tty.h>        // MCC:tty module.
#include <dolphin/fio.h>        // MCC:fio module.
#include <stdio.h>      // sprintf()
#include <string.h>
#include <ctype.h>

#include "corePAD.h"    // PAD input module for core system.
#include "bmp.h"        // Bitmap file analysis module.
#include "mccViewer.h"  // Viewer module.
#include "mccFiler.h"   // Filer module.

// INTERNAL DEFINES
// ==============================
#define FILER_BUFFER_SIZE   (0x00100000)    //1MBytes

// INTERNAL DEFINES
// ==============================
typedef struct{
    char name[128];
    BOOL isDir;
} FilerInfo;

// INTERNAL VARIABLES
// ==============================
static char msg[1024];
static FilerInfo filerInfoList[64];             // Directory information.
static u32  filerListItems  = 0;                // Number of entries in directory information.
static u32  filerCurItems   = 0;                // Currently selected entry in directory information.
static u32  filerDevice     = FILER_DEVICE_FIO; // Target device.
static char filerFioCurrentPath[FIO_MAX_PATH];  // Current path information for FIO.
static u8   filerBuffer[FILER_BUFFER_SIZE] ATTRIBUTE_ALIGN(32); // Buffer to store read files.

// CALLBACK PROTOTYPES
// ==============================
static void padCallback( u16 up, u16 down );

// FUNCTION PROTOTYPES
// ==============================
// Operation of directory information.
static void filerUpdateDirInfo(void);
static void filerClearDirItem(void);
static void filerAddDirItem( char* name, BOOL isDir );
static u32  filerGetNumberOfDirItems(void);
// File operation
static BOOL filerIsFileExt( char* fileName, char* ext );
// Display
static void filerShowDirList(void);
static void filerShowCurItem(void);
static void filerReport( char* string );
// DVD operation
static void filerChangeDirDVD(char *path);
static void filerListupDirDVD(void);
static void filerLoadBitmapDVD(char* fileName, u8* dest, u32* width, u32* height);
// FIO operation
static void filerChangeDirFIO(char *path);
static void filerListupDirFIO(void);
static void filerLoadBitmapFIO(char* fileName, u8* dest, u32* width, u32* height);


// ============================================================
//  API
// ============================================================

// Initializes filer
// ==============================
void filerInitialize()
{
    // Registers PAD callback.
    CorePAD_SetCallback( padCallback );
    // Clears FIO current path information.
    strcpy( filerFioCurrentPath, "" );
    // Obtains current directory information.
    filerUpdateDirInfo();
    // Displays current directory.
    filerShowDirList();
}

// Selects target device (DVD/FIO).
// ==============================
void filerSetDevice( FILER_DEVICE device )
{   filerDevice = device;
}

// Changes target directory.
// ==============================
void filerChangeDir(char *path)
{
    // Moves directory.
    if(filerDevice == FILER_DEVICE_DVD)
         filerChangeDirDVD( path );
    else filerChangeDirFIO( path );
    // Obtains directory information.
    filerUpdateDirInfo();
}

// Reads file.
// ==============================
void filerLoadFile(char *fileName)
{
    if(filerIsFileExt( fileName, "bmp" )){
        u32 bmpWidth,bmpHeight;
        // message
        filerReport("\n# This is Bitmap file.\n");
        // load bitmap
        if(filerDevice == FILER_DEVICE_DVD){
            // from DVD
            filerLoadBitmapDVD( fileName, filerBuffer, &bmpWidth, &bmpHeight );
        }else{
            char filePath[FIO_MAX_PATH];
            if(!strlen(filerFioCurrentPath)){
                strcpy(filePath,"");
            }else{
                strcpy(filePath,filerFioCurrentPath);
                strcat(filePath,"\\");
            }
            strcat( filePath,fileName );
            // from FIO
            filerLoadBitmapFIO( filePath, filerBuffer, &bmpWidth, &bmpHeight );
        }
        // draw bitmap
        viewerDrawBitmap( filerBuffer, bmpWidth, bmpHeight );
    }else if(filerIsFileExt( fileName, "txt" )){
        filerReport("\n# This is Text file.\n");
    }else{
        filerReport("\n# Unsupported file format.\n");
    }
}


// ============================================================
//  CALLBACKS
// ============================================================

//  Callback for PAD input
// ==============================
static void padCallback( u16 up, u16 down )
{   BOOL bShowDir=FALSE;

    //  Selects menu item. ([Y][X] ... [up][down])
    filerCurItems +=
        ((filerCurItems<(filerListItems-1))*((down & 0x0400)!=0))-
        ((filerCurItems>0)*((down & 0x0800)!=0));
    //  Displays items to select.
    if(up & 0x0C00) // [Y]/[X]
        filerShowCurItem();

    //  Selects item. ([A] ... ENTER : chdir / bitmap view.)
    if(up & 0x0100){

        // If the selected item is a directory, moves directory.
        if(filerInfoList[filerCurItems].isDir){
            filerChangeDir( filerInfoList[filerCurItems].name );
        }else{
            // If the selected item is a file.
            filerLoadFile( filerInfoList[filerCurItems].name );
        }
        // Displays current directory.
        bShowDir=TRUE;
    }

    // Displays current directory([B] ... [dir])
    if(up & 0x0200) bShowDir=TRUE;

    // Switches mode ([DVD]/[FIO]).
    if(up & 0x1000){
        filerDevice = (filerDevice==FILER_DEVICE_DVD)?FILER_DEVICE_FIO:FILER_DEVICE_DVD;
        // Updates directory information.
        filerUpdateDirInfo();
        // Displays current directory.
        bShowDir=TRUE;
    }

    // Displays current directory information.
    if(bShowDir) filerShowDirList();
}


// ============================================================
//  FUNCTIONS
// ============================================================

// ========================================
// Operation of directory information
// ========================================
// Updates internal information for current directory information.
// ==============================
static void filerUpdateDirInfo(void)
{
    if(filerDevice == FILER_DEVICE_DVD)
         filerListupDirDVD();
    else filerListupDirFIO();
}

// Clears directory information.
// ==============================
static void filerClearDirItem()
{   filerInfoList[0].isDir = TRUE;
    strcpy(filerInfoList[0].name,".");
    filerInfoList[1].isDir = TRUE;
    strcpy(filerInfoList[1].name,"..");
    filerListItems=2;
}

// Adds entry to directory information.
// ==============================
static void filerAddDirItem( char* name, BOOL isDir )
{   strcpy(filerInfoList[filerListItems].name, name);
    filerInfoList[filerListItems].isDir = isDir;
    filerListItems++;
}

// Returns the number of entries registered in directory information.
// ==============================
static u32 filerGetNumberOfDirItems()
{   return filerListItems;
}

// ========================================
//  File operation
// ========================================
// case insensitive strcmp
// ==============================
static int mystrcmp(char* str1, char* str2)
{
    while(*str1)
    {
        if (tolower(*str1) != tolower(*str2))
            return (tolower(*str1) - tolower(*str2));
        str1++;
        str2++;
    }

    return 0;
}

// Checks if the file extension is specified one.
// ==============================
static BOOL filerIsFileExt( char* fileName, char* ext )
{   int i;
    for(i=(int)strlen(fileName); i!=0; i--)
        if(fileName[i]=='.')
            return (mystrcmp( &fileName[i+1], ext )==0);
    return FALSE;
}

// ========================================
//  Console output
// ========================================
// Displays directory information on the console.
// ==============================
static void filerShowDirList()
{   char outBuffer[8192],tmp[1024];
    int i;
    //  make string
    strcpy(outBuffer,"\n----------\n");
    if(filerDevice==FILER_DEVICE_FIO)
         strcat(outBuffer,"[FIO]\n");
    else strcat(outBuffer,"[DVD]\n");
    for(i=0;i<filerGetNumberOfDirItems();i++){
        sprintf(tmp," %c %s\n",
            (filerInfoList[i].isDir)?'+':'|',
            filerInfoList[i].name);
        strcat(outBuffer,tmp);
    }
    strcat(outBuffer,"----------\n");
    //  out string
    filerReport(outBuffer);
    // Displays items to select.
    filerShowCurItem();
}

// Displays items to select on the console.
// ==============================
static void filerShowCurItem()
{   char outBuffer[8192],tmp[1024];
    //  make string
    strcpy(outBuffer,"");
    strcat(outBuffer,"\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
    strcat(outBuffer,"\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
    strcat(outBuffer,"\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
    strcat(outBuffer,"                                        ");
    strcat(outBuffer,"\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
    strcat(outBuffer,"\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
    strcat(outBuffer,"\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
    sprintf(tmp,": %c %s",
        ((filerInfoList[filerCurItems].isDir)?'+':'|'),
        filerInfoList[filerCurItems].name);
    strcat(outBuffer,tmp);
    //  out string
    filerReport(outBuffer);
}

// Outputs on cosole.
// ==============================
static void filerReport( char* string )
{   OSReport(string);
    TTYPrintf(string), TTYFlush();
}

// ========================================
//  FUNCTIONS for DVD
// ========================================
//  Changes DVD directory.
// ==============================
static void filerChangeDirDVD(char *path)
{
    sprintf(msg,"\n> Change directory to %s/\n",path),filerReport(msg);
    //  Moves directory.
    if(!DVDChangeDir(path))
        filerReport("# DVDChangeDir.NG\n");
}

//  Obtains DVD directory information.
// ==============================
static void filerListupDirDVD()
{
    DVDDir dir;
    char pathName[256]=".";
    //  Initializes directory management information.
    filerClearDirItem();
    //  Updates directory management information.
    // Opens directory.
    if (FALSE == DVDOpenDir(pathName, &dir)){
        sprintf(msg,"Can't open dir %s\n", pathName),filerReport(msg);
    }else{
        DVDDirEntry dirent;
        BOOL result=FALSE;

        sprintf(msg,"> dir %s\n",pathName),filerReport(msg);
        sprintf(msg,"Dir: %08X\n",DVDTellDir( &dir )),filerReport(msg);
        // Reads the content of directory.
        do{
            if(FALSE == DVDReadDir(&dir, &dirent)){
                break;
            }else{
                // Adds to directory information.
                filerAddDirItem( dirent.name, dirent.isDir );
            }
        } while ( TRUE );
        // Closes directory.
        if(FALSE == DVDCloseDir(&dir)){
            sprintf(msg,"Could not close dir.\n"),filerReport(msg);
        }else{
        }
    }
    //  Moves selected item to the beginning.
    filerCurItems = 0;
}

//  Reads BMP file from DVD and returns the width, height and YCbCr information to the specified buffer.
// ==============================
static void filerLoadBitmapDVD(char* fileName, u8* dest, u32* width, u32* height)
{
    DVDFileInfo     finfo;
    u8*             buf;
    bmpInfo_s       bInfo;

    //  Opens DVD file.
    if (FALSE == DVDOpen(fileName, &finfo)){
        sprintf(msg,"Can't open file %s\n", fileName),filerReport(msg);
        return;
    }else{
        u32 length;
        //  Obtains file size.
        length = DVDGetLength(&finfo);
        sprintf(msg,"F %9d %s\n", length, fileName),filerReport(msg);
        //  Allocates memory for data.
        if( NULL == (buf = OSAlloc(OSRoundUp32B(length))) ){
            sprintf(msg,"Alloc failed. Exit\n"),filerReport(msg);
            return;
        }else{
            //  Reads data from DVD.
            if (OSRoundUp32B(length) !=
                DVDRead(&finfo, buf, (s32)OSRoundUp32B(length), 0)){
                sprintf(msg,"Error occurred when reading %s\n", fileName),filerReport(msg);
                OSHalt("");
            }else{
                //  Opens bitmap.
                if (FALSE == openBmp(&bInfo, buf)){
                    sprintf(msg,"Failed to analyze %s as a bmp file\n",fileName),filerReport(msg);
                    OSHalt("");
                }else{
                    //  Displays bitmap information.
                    sprintf(msg,"  bfOffBits: %d\n", bInfo.bfOffBits),filerReport(msg);
                    sprintf(msg,"  width: %d\n", bInfo.width),filerReport(msg);
                    sprintf(msg,"  height: %d\n", bInfo.height),filerReport(msg);
                    sprintf(msg,"  biBitCount: %d\n", bInfo.biBitCount),filerReport(msg);
                    sprintf(msg,"  biCompression: %d\n", bInfo.biCompression),filerReport(msg);
                    sprintf(msg,"  biSizeImage: %d\n", bInfo.biSizeImage),filerReport(msg);
                    sprintf(msg,"  paletteOff: %d\n", bInfo.paletteOff),filerReport(msg);
                    //  Converts bitmap to YCbCr.
                    if(!dest){
                            *width = (bInfo.width + 15) / 16 * 16;
                            *height = (bInfo.height + 1) / 2 * 2;
                    }else{
                        if (FALSE == bmpToYCbCr(&bInfo, buf, dest)){
                            sprintf(msg,"Failed to convert bmp to YCbCr\n"),filerReport(msg);
                            OSHalt("");
                        }else{
                            *width = (bInfo.width + 15) / 16 * 16;
                            *height = (bInfo.height + 1) / 2 * 2;
                        }
                    }
                }
            }
            OSFree(buf);
        }
        DVDClose(&finfo);
    }
}

// ========================================
//  FUNCTIONS for FIO
// ========================================
//  Changes FIO directory
// ==============================
static void filerChangeDirFIO(char *path)
{   BOOL bDirUpdate=FALSE;

    sprintf(msg,"\n> Change directory to %s/\n",path);
    filerReport(msg);
    // Moves directory.
    if(strcmp(path,".")==0){
        // Moves to current directory.
        //filerReport("to current dir.\n");
    }else if(strcmp(path,"..")==0){
        int i;
        // Moves to parent directory.
        //filerReport("to parent dir.\n");
        for(i=(int)strlen(filerFioCurrentPath); i!=0; i--)
            if(filerFioCurrentPath[i]=='\\')
                 filerFioCurrentPath[i] = '\0';
        if(i==0) filerFioCurrentPath[i] = '\0';
    }else{
        // Moves to the selected directory.
        if(strlen(filerFioCurrentPath))
            strcat(filerFioCurrentPath,"\\");
        strcat(filerFioCurrentPath,path);
    }
}

// Obtains FIO directory information.
// ==============================
static void filerListupDirFIO()
{
    FIOFindData finddata;
    char findPath[FIO_MAX_PATH];

    // Sets search path.
    if(!strlen(filerFioCurrentPath)){
        strcpy(findPath,"");
    }else{
        strcpy(findPath,filerFioCurrentPath);
        strcat(findPath,"\\");
    }
    strcat(findPath,"*");
    // Initializes directory management information.
    filerListItems=0;
    // Updates directory management information.
    if(!FIOFindFirst( findPath, &finddata )){
    }else{
        // Adds entry to directory information.
        filerAddDirItem(
            finddata.filename,
            ((finddata.stat.fileAttributes&FIO_ATTRIBUTE_DIRECTORY)!=FALSE));
        // Search next.
        while(FIOFindNext( &finddata )){
            // Adds entry to directory information.
            filerAddDirItem(
                finddata.filename,
                ((finddata.stat.fileAttributes&FIO_ATTRIBUTE_DIRECTORY)!=FALSE));
        }
    }
    // Moves the selected item to the beginning.
    filerCurItems = 0;
}

//  Reads BMP file from FIO and returns the width, height and YCbCr information to the specified buffer.
// ==============================
static void filerLoadBitmapFIO(char* fileName, u8* dest, u32* width, u32* height)
{
    FIOHandle   finfo;
    u8*         buf;
    bmpInfo_s   bInfo;

    //  Opens FIO file.
    if ((finfo = FIOFopen(fileName, FIO_OPEN_RDONLY))==FIO_INVALID_HANDLE){
        sprintf(msg,"Can't open file %s\n", fileName),filerReport(msg);
        return;
    }else{
        u32 length;
        //  Obtains file size.
        length = FIOFseek( finfo, 0, FIO_SEEK_LAST );
        FIOFseek( finfo, 0, FIO_SEEK_TOP );
        sprintf(msg,"F %9d %s\n", length, fileName),filerReport(msg);
        //  Allocates memory for data.
        if( NULL == (buf = OSAlloc(OSRoundUp32B( (u32)(((length+MCC_BLOCK_SIZE-1)/MCC_BLOCK_SIZE)*MCC_BLOCK_SIZE) ))) ){
            sprintf(msg,"Alloc failed. Exit\n"),filerReport(msg);
            return;
        }else{
            //  Reads data from FIO.
            if (length != FIOFread( finfo, buf, length )){
                sprintf(msg,"Error occurred when reading %s\n", fileName),filerReport(msg);
                OSHalt("");
            }else{
                //  Opens bitmap.
                if (FALSE == openBmp(&bInfo, buf)){
                    sprintf(msg,"Failed to analyze %s as a bmp file\n",fileName),filerReport(msg);
                    OSHalt("");
                }else{
                    //  Displays bitmap information.
                    sprintf(msg,"  bfOffBits: %d\n", bInfo.bfOffBits),filerReport(msg);
                    sprintf(msg,"  width: %d\n", bInfo.width),filerReport(msg);
                    sprintf(msg,"  height: %d\n", bInfo.height),filerReport(msg);
                    sprintf(msg,"  biBitCount: %d\n", bInfo.biBitCount),filerReport(msg);
                    sprintf(msg,"  biCompression: %d\n", bInfo.biCompression),filerReport(msg);
                    sprintf(msg,"  biSizeImage: %d\n", bInfo.biSizeImage),filerReport(msg);
                    sprintf(msg,"  paletteOff: %d\n", bInfo.paletteOff),filerReport(msg);
                    //  Converts bitmap to YCbCr.
                    if(!dest){
                            *width = (bInfo.width + 15) / 16 * 16;
                            *height = (bInfo.height + 1) / 2 * 2;
                    }else{
                        if (FALSE == bmpToYCbCr(&bInfo, buf, dest)){
                            sprintf(msg,"Failed to convert bmp to YCbCr\n"),filerReport(msg);
                            OSHalt("");
                        }else{
                            *width = (bInfo.width + 15) / 16 * 16;
                            *height = (bInfo.height + 1) / 2 * 2;
                        }
                    }
                }
            }
            OSFree(buf);
        }
        FIOFclose( finfo );
    }
}
