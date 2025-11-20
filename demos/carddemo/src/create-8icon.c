/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     create-8icon.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/carddemo/src/create-8icon.c $
    
    2     7/02/01 11:32a Dante
    Removed second CreateFile becuase of bug. It was possible to create two
    files within the same second, hence with the same name.
    
    1     6/19/01 12:57p Hosogai
    Initial check-in
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
   create-8icon
	 A simple program that creates a memory card file with 8 icons
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <string.h>
#include <dolphin.h>
#include <dolphin/card.h>
#include <demo.h>

#include "create-8icon.h"

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void main();
static u32 MountCard( s32 chan );
static void CreateFile(s32 chan, u32 secSize, u32 blockSize, u32 format);
static void UnmountCard( s32 chan );
static void LoadIconC8    ( void* buffer, CARDStat* stat );
static void LoadIconRGB5A3( void* buffer, CARDStat* stat );
static void DetachCallback( void );
static void AttachCallback( void );
static void NewFileName( char* fileName );
static void PrintResult(s32 result);


/*---------------------------------------------------------------------------*
   Global data
 *---------------------------------------------------------------------------*/
u8 WRAM[CARD_WORKAREA_SIZE] ATTRIBUTE_ALIGN(32);

/*---------------------------------------------------------------------------*
   Application main 
 *---------------------------------------------------------------------------*/
void main()
{
	u32 sectorSize;

	DEMOInit(NULL);
	CARDInit();

	sectorSize = MountCard(CARD_CHAN);

	CreateFile(CARD_CHAN, sectorSize, FILE_BLOCK_SIZE, CARD_STAT_ICON_C8);

	UnmountCard(CARD_CHAN);

	OSReport("End of program\n");
}

/*---------------------------------------------------------------------------*
	Name:           LoadIconC8

	Description:    Loads a C8 icon and information into memcard file

	Arguments:      file		TPL file name with C8 data
					buffer		memcard data buffer
					stat		CARDStat for file

	Returns:        none
 *---------------------------------------------------------------------------*/
static void LoadIconC8( void* buffer, CARDStat* stat )
{
	static CARDHeaderC8 HeaderC8 ATTRIBUTE_ALIGN(32)= {
		"Greensburg Heiho\n",
		"8 icons (C8) sample\n",
		0,
		0
	};
	static TEXPalettePtr  tplIcons;
	static TEXDescriptorPtr tdpIcons;
	static u32 i;

	// Load the TPL file
	TEXGetPalette(&tplIcons, "/carddemo/heihoC8.tpl");

	// Copy the Texel and CLUT to the structure
	for (i=0; i<CARD_ICON_MAX; i++)
	{
		tdpIcons = TEXGet(tplIcons, (u32) i);
		memcpy(HeaderC8.Icons[i], tdpIcons->textureHeader->data, ICON_CI_SIZE);
	}
	memcpy(HeaderC8.CLUT,   tdpIcons->CLUTHeader->data,  CLUT_SIZE);

	CARDSetCommentAddress(stat, 0);
	CARDSetIconAddress   (stat, CARD_COMMENT_SIZE);
	CARDSetBannerFormat  (stat, CARD_STAT_BANNER_NONE);	// <<== No banner
	CARDSetIconAnim      (stat, CARD_STAT_ANIM_LOOP);
	for (i=0; i<CARD_ICON_MAX; i++)
	{
		CARDSetIconFormat    (stat, i, CARD_STAT_ICON_C8);
		CARDSetIconSpeed     (stat, i, CARD_STAT_SPEED_MIDDLE);
	}

	// Copy the icon to the buffer
	memcpy(buffer, &HeaderC8, sizeof(CARDHeaderC8));

	// Free the TexPalette
	TEXReleasePalette(&tplIcons);
}

/*---------------------------------------------------------------------------*
	Name:           LoadIconRGB5A3

	Description:    Loads a RGB5A3 icon and information into memcard file

	Arguments:      file		TPL file name with RGB5A3 data
					buffer		memcard data buffer
					stat		CARDStat for file

	Returns:        none
 *---------------------------------------------------------------------------*/
static void LoadIconRGB5A3( void* buffer, CARDStat* stat )
{
	CARDHeaderRGB5A3 HeaderRGB5A3 ATTRIBUTE_ALIGN(32) = {
		"Redmond Heiho\n",
		"8 icons (RGB5A3) sample\n",
		0
	};
	static TEXPalettePtr  tplIcons;
	static TEXDescriptorPtr tdpIcons;
	static u32 i;

	// Load the TPL file
	TEXGetPalette(&tplIcons, "/carddemo/heiho.tpl");

	// Copy the Texels to the structure
	for (i=0; i<CARD_ICON_MAX; i++)
	{
		tdpIcons = TEXGet(tplIcons, (u32) i);

		// Copy the Texel and CLUT to the structure
		memcpy(HeaderRGB5A3.Icons[i], tdpIcons->textureHeader->data, 
				ICON_RGB5A3_SIZE);
	}

	CARDSetCommentAddress(stat, 0);
	CARDSetIconAddress   (stat, CARD_COMMENT_SIZE);
	CARDSetBannerFormat  (stat, CARD_STAT_BANNER_NONE);	// <<== No banner
	CARDSetIconAnim      (stat, CARD_STAT_ANIM_LOOP);
	for (i=0; i<CARD_ICON_MAX; i++)
	{
		CARDSetIconFormat    (stat, i, CARD_STAT_ICON_RGB5A3);
		CARDSetIconSpeed     (stat, i, CARD_STAT_SPEED_FAST);
	}

	// Copy the icon to the buffer
	memcpy(buffer, &HeaderRGB5A3, sizeof(HeaderRGB5A3));

	// Free the TexPalette
	TEXReleasePalette(&tplIcons);
}

/*---------------------------------------------------------------------------*
	Name:           CreateFile

	Description:    Creates a file on the memcard

	Arguments:      chan		EXI Channel
					secSize		memcard sector size
					blockSize	memcard block size
					format		memcard icon format (C8 or RGB5A3)

	Returns:        none
 *---------------------------------------------------------------------------*/
static void CreateFile(s32 chan, u32 secSize, u32 blockSize, u32 format)
{
	s32 result;
	CARDStat stat;
	CARDFileInfo FileInfo;
	void* CARDData;
	char FileName[CARD_FILENAME_MAX]={'\0'};

	// Create the new file name
	// based on the system clock
	NewFileName(FileName);

	// Create the file
	result = CARDCreate(chan, FileName, secSize*blockSize, &FileInfo);
	if (result < 0)
	{
		OSReport("Failed to create the file.\n");
		PrintResult(result);
	}
	else
	{
		OSReport("File created successfully name = [%s]\n", FileName);
	}

	// Get CARDStat of the file that was created
	result = CARDGetStatus(chan, FileInfo.fileNo, &stat);
	if (result < 0)
	{
		OSReport("Failed to get CARDStat\n");
		PrintResult(result);
	}
	else
	{
		OSReport("CARDStat successfully got.\n");
	}

	// Allocate the temp buffer
	// (same size with the file size that was created)
	CARDData = OSAlloc(stat.length);

	// Load the icon from TPL into the buffer
	if (format == CARD_STAT_ICON_C8)
	{
		LoadIconC8(CARDData, &stat);
	}
	else if (format == CARD_STAT_ICON_RGB5A3)
	{
		LoadIconRGB5A3(CARDData, &stat);
	}
	else
	{
		OSHalt("Illegal format options\n");
	}

	// Write the buffer to the memory card
	result = CARDWrite(&FileInfo, CARDData, (s32) stat.length, 0);
	if (result < 0)
	{
		OSReport("Failed to CARDWrite.\n");
		PrintResult(result);
	}
	else
	{
		OSReport("CARDWrite successfully.\n");
	}
	
	// Set the CARDStat which was modified.
	result = CARDSetStatus(chan, FileInfo.fileNo, &stat);
	if (result < 0)
	{
		OSReport("Failed to set CARDStat\n");
		PrintResult(result);
	}
	else
	{
		OSReport("CARDStat successfully set.\n");
	}

	// Free the temp buffer
	OSFree(CARDData);
}

/*---------------------------------------------------------------------------*
	Name:           MountCard

	Description:    Mounts and checks memory card

	Arguments:      chan		EXI Channel

	Returns:        none
 *---------------------------------------------------------------------------*/
static u32 MountCard( s32 chan )
{
	u32 sectorSize;
	s32 result;

	// Probe the card
	OSReport("Please insert the memory card in slot B\n");
	result = FALSE; 
	while (result==FALSE)
	{
		result = CARDProbe(chan);
	}

	// Mount the card
	result = CARDMount(chan, WRAM, (CARDCallback)DetachCallback);
	if (result < 0)
	{
		OSReport("CARD failed to mount\n");
		// Do not halt here.  
		// (needs to check unformatted card.)
	}
	else
	{
		OSReport("CARD mounted successfully\n");
	}

	// If the card is broken, reformat the card
	result = CARDCheck(chan);
	
	if (result == CARD_RESULT_BROKEN)
	{	
		// If the card is broken, format it
		result = CARDFormat(chan);		
		if (result < 0)
		{
			OSReport("Failed to format\n");
			PrintResult(result);
		}
		else
		{
			OSReport("CARD formatted successfully\n");
		}
	}
	else if(result==CARD_RESULT_ENCODING)
	{
		OSReport("Wrong Encoding.\n");
		PrintResult(result);
	}
	else
	{
		OSReport("CARD is preformatted.\n");
	}

	// Get the sector size
	result = CARDGetSectorSize(chan, &sectorSize);
	if (result < 0)
	{
		OSReport("CARD faile to Get the sector size.\n");
		PrintResult(result);
	}
	else
	{
		OSReport("sectorSize = %d\n", sectorSize);
	}
	return sectorSize;
}

/*---------------------------------------------------------------------------*
	Name:           UnmountCard

	Description:    Unmounts memory card

	Arguments:      chan		EXI Channel

	Returns:        none
 *---------------------------------------------------------------------------*/
static void UnmountCard(s32 chan)
{
	s32 result;
	// UnmountCard the card
	result = CARDUnmount(chan);
	if (result < 0)
	{
		OSReport("CARD failed to unmount\n");
		PrintResult(result);
	}
	else
	{
		OSReport("CARD Unmounted successfully\n");
	}
}

/*---------------------------------------------------------------------------*
	Name:           DetachCallback & AttachCallback

	Description:    Simple memory card callbacks

	Arguments:      none

	Returns:        none
 *---------------------------------------------------------------------------*/
static void DetachCallback( void )
{
	OSReport("Card was detached.\n");
}
static void AttachCallback( void )
{
	OSReport("Card was attached.\n");
}

/*---------------------------------------------------------------------------*
	Name:           NewFileName

	Description:    Creates a filename using the date

	Arguments:      fileName	pointer to allocated string

	Returns:        none
 *---------------------------------------------------------------------------*/
static void NewFileName(char* fileName)
{
	OSTime         time;
	OSCalendarTime ct;

	time = OSGetTime();
	OSTicksToCalendarTime(time, &ct);
	sprintf(fileName, "%04d/%02d/%02d %02d:%02d:%02d",
			ct.year, ct.mon + 1, ct.mday, ct.hour, ct.min, ct.sec);
}

/*---------------------------------------------------------------------------*
	Name:           PrintResult

	Description:    Prints out the defined result (and halt upon error)

	Arguments:      result		CARD API result 

	Returns:        none
 *---------------------------------------------------------------------------*/
static void PrintResult(s32 result)
{
	switch (result)
	{
		case CARD_RESULT_READY:
			OSReport("CARD_RESULT_READY\n");
			break;
		case CARD_RESULT_FATAL_ERROR:
			OSHalt("CARD_RESULT_FATAL_ERROR\n");
			break;
		case CARD_RESULT_WRONGDEVICE:
			OSHalt("CARD_RESULT_WRONGDEVICE\n");
			break;
		case CARD_RESULT_NOCARD:
			OSHalt("CARD_RESULT_NOCARD\n");
			break;
		case CARD_RESULT_BUSY:
			OSHalt("CARD_RESULT_BUSY\n");
			break;
		case CARD_RESULT_IOERROR:
			OSHalt("CARD_RESULT_IOERROR\n");
			break;
		case CARD_RESULT_BROKEN:
			OSHalt("CARD_RESULT_BROKEN\n");
			break;
		case CARD_RESULT_EXIST:
			OSHalt("CARD_RESULT_EXIST\n");
			break;
		case CARD_RESULT_NOENT:
			OSHalt("CARD_RESULT_NOENT\n");
			break;
		case CARD_RESULT_INSSPACE:
			OSHalt("CARD_RESULT_INSSPACE\n");
			break;
		case CARD_RESULT_NAMETOOLONG:
			OSHalt("CARD_RESULT_NAMETOOLONG\n");
			break;
		default:
			OSHalt("Illegal\n");
			break;
	}
}
