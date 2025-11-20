/*---------------------------------------------------------------------------*
  Project:  CARD utilities
  File:     cardutil.c

  Copyright 2000-2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/carddemo/src/cardutil.c $
    
    6     11/26/01 11:54 Shiki
    Modified DoSave() to add CARD_XFER_WRITE bytes to CardControl.length.

    5     9/25/01 18:27 Shiki
    Fixed DoList() to perform the same CARD_STAT_ICON_NONE animation as IPL
    does.

    4     5/18/01 5:24p Shiki
    Added CardUtilGetProgress().

    3     01/04/26 10:29 Shiki
    Fixd CardUtilDrawIcon() to setup TexGen.

    2     01/04/25 14:32 Shiki
    Added size param to CardUtilDrawIcon().

    1     01/04/23 17:15 Shiki
    Initial check-in.
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <string.h>
#include <demo.h>
#include <dolphin/card.h>
#include "cardutil.h"

typedef struct ControlBlock
{
    OSMutex         mutex;          // mutex for control block
    OSCond          cond;

    s32             slot;
    s32             command;
    s32             fileNo;
    void*           param;
    s32             result;

    s32             byteNotUsed;
    s32             filesNotUsed;
    u32             sectorSize;

    s32             xferred;        // the initial xferred bytes for the current command
    s32             length;         // the total xfer bytes for the current command

    OSMutex         mutexDirectory; // mutex for directory and numFiles
    CardUtilDirent* directory;
    s32             numFiles;
} ControlBlock;

static OSThread      CardThread;
static ControlBlock  CardControl;

/*---------------------------------------------------------------------------*
  Name:         CardUtilNumFiles

  Description:  Returns the current number of own game files in the directory

  Arguments:    None.

  Returns:      The current number of own game files in the directory
 *---------------------------------------------------------------------------*/
s32 CardUtilNumFiles(void)
{
    return CardControl.numFiles;
}

/*---------------------------------------------------------------------------*
  Name:         CardUtilLockDirectory

  Description:  Locks the directory and returns the current number of own
                game files in the directory. Card utility thread waits
                updating the directory until CardUtilUnlockDirectory() is
                called.

  Arguments:    None.

  Returns:      The current number of own game files in the directory.
 *---------------------------------------------------------------------------*/
s32 CardUtilLockDirectory(void)
{
    OSLockMutex(&CardControl.mutexDirectory);
    return CardControl.numFiles;
}

/*---------------------------------------------------------------------------*
  Name:         CardUtilUnlockDirectory

  Description:  Unlocks the directory

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CardUtilUnlockDirectory(void)
{
    OSUnlockMutex(&CardControl.mutexDirectory);
}

/*---------------------------------------------------------------------------*
  Name:         CardUtilByteNotUsed

  Description:  Returns the number of free bytes available in the current
                memory card

  Arguments:    None.

  Returns:      The number of free bytes available in the current
                memory card
 *---------------------------------------------------------------------------*/
s32 CardUtilByteNotUsed(void)
{
    return CardControl.byteNotUsed;
}

/*---------------------------------------------------------------------------*
  Name:         CardUtilBlocksNotUsed

  Description:  Returns the number of free blocks available in the current
                memory card

  Arguments:    None.

  Returns:      The number of free blocks available in the current
                memory card
 *---------------------------------------------------------------------------*/
s32 CardUtilBlocksNotUsed(void)
{
    if (CardControl.sectorSize)
    {
        return (s32) (CardControl.byteNotUsed / CardControl.sectorSize);
    }
    return 0;
}

/*---------------------------------------------------------------------------*
  Name:         CardUtilFilesNotUsed

  Description:  Returns the number of free directory entries available in the
                current memory card

  Arguments:    None.

  Returns:      The number of free directory entries available in the
                current memory card
 *---------------------------------------------------------------------------*/
s32 CardUtilFilesNotUsed(void)
{
    return CardControl.filesNotUsed;
}

/*---------------------------------------------------------------------------*
  Name:         CardUtilSectorSize

  Description:  Returns the sector size of the current memory card

  Arguments:    None.

  Returns:      The sector size of the current memory card
 *---------------------------------------------------------------------------*/
s32 CardUtilSectorSize(void)
{
    return (s32) CardControl.sectorSize;
}

/*---------------------------------------------------------------------------*
  Name:         DoMount

  Description:  Performs the mount operation
 *---------------------------------------------------------------------------*/
static s32 DoMount(s32 slot, void* workArea)
{
    s32 result;
    s32 resultSectorSize;

    CardUtilLockDirectory();
        CardControl.numFiles = 0;
    CardUtilUnlockDirectory();

    CardControl.byteNotUsed = CardControl.filesNotUsed = 0;

    // Mound and check
    CardControl.length = CARD_XFER_MOUNT;
    result = CARDMount(slot, workArea, 0);
    switch (result)
    {
      case CARD_RESULT_READY:
      case CARD_RESULT_BROKEN:
        resultSectorSize = CARDGetSectorSize(slot, &CardControl.sectorSize);
        if (resultSectorSize < 0)
        {
            return resultSectorSize;
        }
        result = CARDCheck(slot);
        break;
      case CARD_RESULT_ENCODING:
        resultSectorSize = CARDGetSectorSize(slot, &CardControl.sectorSize);
        if (resultSectorSize < 0)
        {
            return resultSectorSize;
        }
        break;
      default:
        // Memory card is not mounted.
        break;
    }

    // Free blocks
    if (result == CARD_RESULT_READY)
    {
        result = CARDFreeBlocks(slot, &CardControl.byteNotUsed, &CardControl.filesNotUsed);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         DoMount

  Description:  Performs the unmount operation
 *---------------------------------------------------------------------------*/
static s32 DoUnmount(s32 slot)
{
    CardUtilLockDirectory();
        CardControl.numFiles  = 0;
    CardUtilUnlockDirectory();

    return CARDUnmount(slot);
}

/*---------------------------------------------------------------------------*
  Name:         DoFormat

  Description:  Performs the format operation
 *---------------------------------------------------------------------------*/
static s32 DoFormat(s32 slot)
{
    s32 result;

    CardUtilLockDirectory();
        CardControl.numFiles  = 0;
    CardUtilUnlockDirectory();

    // Format
    CardControl.length = CARD_XFER_FORMAT;
    result = CARDFormat(slot);

    // FreeBlocks
    if (result == CARD_RESULT_READY)
    {
        result = CARDFreeBlocks(slot, &CardControl.byteNotUsed, &CardControl.filesNotUsed);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         DoErase

  Description:  Performs the erase operation and updates the directory
 *---------------------------------------------------------------------------*/
static s32 DoErase(s32 slot, s32 fileNo)
{
    s32 result;

    // Delete
    CardControl.length = CARD_XFER_DELETE;
    result = CARDFastDelete(slot, fileNo);
    if (result < 0)
    {
        return result;
    }

    // Update directory
    if (CardControl.directory)
    {
        CardUtilDirent* ent;

        for (ent = CardControl.directory;
             ent < &CardControl.directory[CardControl.numFiles];
             ++ent)
        {
            if (ent->fileNo == fileNo)
            {
                CardUtilLockDirectory();
                    memmove(ent, ent + 1, (u32) &CardControl.directory[CardControl.numFiles] - (u32) (ent + 1));
                    --CardControl.numFiles;
                    DCStoreRange(ent, (u32) &CardControl.directory[CardControl.numFiles] - (u32) ent);
                CardUtilUnlockDirectory();
            }
        }
    }

    // FreeBlocks
    return CARDFreeBlocks(slot, &CardControl.byteNotUsed, &CardControl.filesNotUsed);
}

/*---------------------------------------------------------------------------*
  Name:         DoList

  Description:  Updates the directory.

  Note:         DoList() works with DoSave() to support reliable file
                save. If temporary files, whose name begin with '~', remains,
                DoList() rescues and/or erases temporary files if possible.

                If the memory card becomes unavailable here due to power
                failure, etc. while saving the data, the following senario
                can happen.

                1. Both temp file and original file exist.

                   -> DoList() removes the temp file.

                2. Only temp file exits.

                   -> If the temp file is valid, DoList() renames the temp
                      file to the original file name.
                   -> If the temp file data is invalid, DoList removes the
                      temp file. Probably, the player copied the broken
                      temp file to another memory card.
 *---------------------------------------------------------------------------*/
static s32 DoList(s32 slot, CardUtilDirent* directory)
{
    s32          fileNo;
    CARDFileInfo fileInfo;
    s32          result;
    s32          offset;
    s32          length;

    int          i;
    int          j;
    int          cIcon;
    int          speed;

    DVDDiskID*   diskID = DVDGetCurrentDiskID();

    result = CARD_RESULT_READY;

    CardUtilLockDirectory();
        CardControl.directory = directory;
        CardControl.numFiles  = 0;
    CardUtilUnlockDirectory();

    if (directory == 0)
    {
        return result;
    }

    memset(directory, 0, CARD_MAX_FILE * sizeof(CardUtilDirent));
    for (fileNo = 0; fileNo < CARD_MAX_FILE; ++fileNo)
    {
        CardUtilDirent* ent = &directory[CardControl.numFiles];

        // Skip open entries and other game entries.
        if (CARDGetStatus(slot, fileNo, &ent->stat) < 0 ||
            memcmp(ent->stat.gameName, diskID->gameName, sizeof(diskID->gameName)) != 0 ||
            memcmp(ent->stat.company,  diskID->company,  sizeof(diskID->company))  != 0)
        {
            continue;
        }

        // Rescue/erase temp file
        if (ent->stat.fileName[0] == '~')
        {
            char fileName[CARD_FILENAME_MAX + 1];
            char tempName[CARD_FILENAME_MAX + 1];

            strncpy(tempName, ent->stat.fileName, CARD_FILENAME_MAX);
            tempName[CARD_FILENAME_MAX] = '\0';
            strncpy(fileName, &tempName[1],       CARD_FILENAME_MAX);
            fileName[CARD_FILENAME_MAX] = '\0';

            if (ent->stat.commentAddr <= ent->stat.length - CARD_COMMENT_SIZE &&    // Comment available?
                CARDRename(slot, tempName, fileName) == CARD_RESULT_READY)
            {
                // Temp file was rescued as save data.
                --fileNo;   // Retry with the current fileNo.
            }
            else if (// Original file still exists or file is invalid
                     (result = CARDFastDelete(slot, fileNo)) < 0 ||
                     // Update free blocks
                     (result = CARDFreeBlocks(slot, &CardControl.byteNotUsed, &CardControl.filesNotUsed)) < 0)
            {
                return result;
            }
            continue;
            // NOT REACHED HERE
        }

        // Read comment
        memset(ent->comment, 0, CARD_COMMENT_SIZE);
        if (ent->stat.commentAddr <= ent->stat.length - CARD_COMMENT_SIZE)
        {
            result = CARDFastOpen(slot, fileNo, &fileInfo);
            if (result < 0)
            {
                return result;
            }

            offset = (s32) ent->stat.commentAddr & ~(CARD_READ_SIZE - 1);
            length = (s32) (ent->stat.commentAddr + CARD_COMMENT_SIZE - offset);
            length = (length + (CARD_READ_SIZE - 1)) & ~(CARD_READ_SIZE - 1);
            result = CARDRead(&fileInfo, ent, length, offset);
            CARDClose(&fileInfo);
            if (result < 0)
            {
                return result;
            }

            offset = (s32) (ent->stat.commentAddr & (CARD_READ_SIZE - 1));
            memmove(ent->comment, (u8*) ent + offset, CARD_COMMENT_SIZE);
        }

        // Read banner and icon data
        if ((ent->stat.bannerFormat || ent->stat.iconFormat) &&
            ent->stat.offsetData <= ent->stat.length &&
            ent->stat.iconAddr < ent->stat.offsetData)
        {
            result = CARDFastOpen(slot, fileNo, &fileInfo);
            if (result < 0)
            {
                return result;
            }

            offset = (s32) ent->stat.iconAddr & ~(CARD_READ_SIZE - 1);
            length = (s32) (ent->stat.offsetData - offset);
            length = (length + (CARD_READ_SIZE - 1)) & ~(CARD_READ_SIZE - 1);
            result = CARDRead(&fileInfo, ent, length, offset);
            CARDClose(&fileInfo);
            if (result < 0)
            {
                return result;
            }

            offset = (s32) (ent->stat.iconAddr & (CARD_READ_SIZE - 1));
            memmove(ent, (u8*) ent + offset, ent->stat.offsetData - ent->stat.iconAddr);
            DCFlushRange(ent, ent->stat.offsetData - ent->stat.iconAddr);

            j = 0;
            ent->cFrame = 0;
            for (cIcon = 0; cIcon < CARD_ICON_MAX; ++cIcon, ++j)
            {
                speed = CARDGetIconSpeed(&ent->stat, cIcon);
                if (speed == CARD_STAT_SPEED_END)
                {
                    break;
                }
                ent->nFrame[j] = ent->cFrame;
                ent->cFrame += 4 * speed;
                if (CARDGetIconFormat(&ent->stat, cIcon) != CARD_STAT_ICON_NONE)
                {
                    ent->iIcon[j] = cIcon;
                }
                else
                {
                    // Pick up next one
                    ent->iIcon[j] = 0;
                    for (i = cIcon; i < CARD_ICON_MAX; ++i)
                    {
                        if (CARDGetIconFormat(&ent->stat, i) != CARD_STAT_ICON_NONE)
                        {
                            ent->iIcon[j] = i;
                            break;
                        }
                    }
                }
            }
            if (CARDGetIconAnim(&ent->stat) == CARD_STAT_ANIM_BOUNCE && 2 < cIcon)
            {
                for (i = cIcon - 2; 0 < i; --i, ++j)
                {
                    speed = CARDGetIconSpeed(&ent->stat, i);
                    ASSERT(speed != CARD_STAT_SPEED_END);
                    ent->nFrame[j] = ent->cFrame;
                    ent->iIcon[j] = ent->iIcon[i];
                    ent->cFrame += 4 * speed;
                }
            }
        }

        ent->fileNo = fileNo;

        CardUtilLockDirectory();
            ++CardControl.numFiles;
        CardUtilUnlockDirectory();
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         DoOpen

  Description:  Reads the specified file.
 *---------------------------------------------------------------------------*/
static s32 DoOpen(s32 slot, s32 fileNo, void* buffer)
{
    s32          result;
    CARDStat     stat;
    CARDFileInfo fileInfo;

    result = CARDGetStatus(slot, fileNo, &stat);
    if (result < 0)
    {
        return result;
    }

    result = CARDFastOpen(slot, fileNo, &fileInfo);
    if (result < 0)
    {
        return result;
    }

    CardControl.length = (s32) stat.length;
    result = CARDRead(&fileInfo, buffer, (s32) stat.length, 0);
    CARDClose(&fileInfo);
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         DoSave

  Description:  Saves the specified file.

  Note:         This function performs reliable file save by saving data
                into temporary file first. Thus it requires the same number
                of temporary free blocks as the number of saved
                file consumes, and also one free file directory entry.
                Note temporary file name begins with '~', and thus the game
                program should choose filenames that not begin with '~'.
 *---------------------------------------------------------------------------*/
static s32 DoSave(s32 slot, CARDStat* stat, void* buffer)
{
    CARDFileInfo    fileInfo;
    char            fileName[CARD_FILENAME_MAX + 1];
    char            tempName[CARD_FILENAME_MAX + 1];
    s32             result;
    s32             fileNo;
    s32             oldNo;
    CardUtilDirent* ent;

    strncpy(fileName, stat->fileName, CARD_FILENAME_MAX);
    fileName[CARD_FILENAME_MAX] = '\0';
    if (CARD_FILENAME_MAX <= strlen(fileName))
    {
        return CARD_RESULT_NAMETOOLONG;
    }
    if (fileName[0] == '~')
    {
        return CARD_RESULT_FATAL_ERROR;
    }

    // Creates temorary file name from fileName by attaching '~' at
    // the beginning.
    tempName[0] = '~';
    strncpy(&tempName[1], stat->fileName, CARD_FILENAME_MAX - 1);
    tempName[CARD_FILENAME_MAX] = '\0';

    // To perform overwrite, verifies if the original file exists.
    oldNo = -1;
    result = CARDOpen(slot, fileName, &fileInfo);
    if (result == CARD_RESULT_READY)
    {
        // Gets original file number
        oldNo = CARDGetFileNo(&fileInfo);
        CARDClose(&fileInfo);
    }

    CardControl.length = (s32) stat->length + CARD_XFER_CREATE + CARD_XFER_WRITE + CARD_XFER_SETSTATUS + CARD_XFER_RENAME;
    if (0 <= oldNo && oldNo < CARD_MAX_FILE)
    {
        CardControl.length += CARD_XFER_DELETE;
    }

    // Tries to create temorary file
    result = CARDCreate(slot, tempName, stat->length, &fileInfo);
    if (result < 0)
    {
        return result;
    }

    // Saves the data to the temporary file.
    // Performs CARDSetStatus() at very last so the program can assume
    // the temporay file has valid save data if its comment and
    // other directory infomation is valid. Note CARD API guarantees the
    // system block integrity.
    fileNo = CARDGetFileNo(&fileInfo);
    result = CARDWrite(&fileInfo, buffer, (s32) stat->length, 0);
    CARDClose(&fileInfo);
    if (result < 0 || (result = CARDSetStatus(slot, fileNo, stat)) < 0)
    {
        return result;
    }

    // To perform overwrite, deletes the original file if exists.
    if (0 <= oldNo && oldNo < CARD_MAX_FILE)
    {
        // Delete original file
        result = CARDFastDelete(slot, oldNo);
        if (result < 0)
        {
            return result;
        }
    }

    // If the memory card becomes unavailable here due to power failure, etc.
    // DoList() renames the temp file to the original file name as it has the
    // valid data. If both temp file and original file exist in the same
    // memory card, DoList() removes the temp file.

    // Renames the temp file name to the final name
    result = CARDRename(slot, tempName, fileName);
    if (result < 0)
    {
        return result;
    }

    if (CardControl.directory == 0)
    {
        // FreeBlocks
        return CARDFreeBlocks(slot, &CardControl.byteNotUsed, &CardControl.filesNotUsed);
        // NOT REACHED HERE
    }

    // Locks and updates directory
    CardUtilLockDirectory();

        if (oldNo == -1)
        {
            ent = &CardControl.directory[CardControl.numFiles];
            ++CardControl.numFiles;
        }
        else
        {
            for (ent = CardControl.directory;
                 ent < &CardControl.directory[CardControl.numFiles];
                 ++ent)
            {
                if (ent->fileNo == oldNo)
                {
                    break;
                }
            }
            if (ent == &CardControl.directory[CardControl.numFiles])
            {
                ++CardControl.numFiles;
            }
        }
        ASSERT(CardControl.numFiles <= CARD_MAX_FILE);

        // Read comment
        memset(ent->comment, 0, CARD_COMMENT_SIZE);
        if (stat->commentAddr <= stat->length - CARD_COMMENT_SIZE)
        {
            memmove(ent->comment, (u8*) buffer + stat->commentAddr, CARD_COMMENT_SIZE);
        }

        // Read banner and icon data
        ent->cFrame = 0;
        if (stat->bannerFormat || stat->iconFormat)
        {
            int i;
            int j;
            int cIcon;
            int speed;

            ASSERT(stat->iconAddr < stat->length);
            ASSERT(stat->offsetData < stat->length);
            ASSERT(stat->iconAddr < stat->offsetData);
            memmove(ent, (u8*) buffer + stat->iconAddr, stat->offsetData - stat->iconAddr);
            DCFlushRange(ent, stat->offsetData - stat->iconAddr);

            j = 0;
            for (cIcon = 0, i = 0; cIcon < CARD_ICON_MAX; ++cIcon, ++j)
            {
                speed = CARDGetIconSpeed(stat, cIcon);
                if (speed == CARD_STAT_SPEED_END)
                {
                    break;
                }
                ent->nFrame[j] = ent->cFrame;
                ent->iIcon[j] = i;
                ent->cFrame += 4 * speed;
                if (CARDGetIconFormat(stat, i) != CARD_STAT_ICON_NONE)
                {
                    ++i;
                }
            }
            if (CARDGetIconAnim(stat) == CARD_STAT_ANIM_BOUNCE && 2 < cIcon)
            {
                for (i = cIcon - 2; 0 < i; --i, ++j)
                {
                    speed = CARDGetIconSpeed(stat, i);
                    ASSERT(speed != CARD_STAT_SPEED_END);
                    ent->nFrame[j] = ent->cFrame;
                    ent->iIcon[j] = ent->iIcon[i];
                    ent->cFrame += 4 * speed;
                }
            }
        }

        memcpy(&ent->stat, stat, sizeof(CARDStat));
        ent->fileNo = fileNo;

    CardUtilUnlockDirectory();

    // FreeBlocks
    return CARDFreeBlocks(slot, &CardControl.byteNotUsed, &CardControl.filesNotUsed);
}

/*---------------------------------------------------------------------------*
  Name:         CardUtilCommand

  Description:  Issues a command to the card utility thread.

                The game threads communicate to the card utility thread via
                the CardControl block, which is guarded by a mutex.
 *---------------------------------------------------------------------------*/
static s32 CardUtilCommand(s32 slot, s32 command, s32 fileNo, void* param)
{
    s32 result;

    OSLockMutex(&CardControl.mutex);

    // CardUtil accepts one command at a time
    ASSERT(CardControl.slot == -1);

    if (CardControl.slot != -1)
    {
        // Card utility thread is busy
        result = CardControl.result;
    }
    else
    {
        // Card utility thread is idle. Save command infomation and
        // wakes up card utility threads.
        CardControl.slot    = slot;
        CardControl.command = command;
        CardControl.fileNo  = fileNo;
        CardControl.param   = param;
        CardControl.result  = CARD_RESULT_BUSY;
        if (command != CARDUTIL_CMD_LIST)
        {
            CardControl.xferred = CARDGetXferredBytes(slot);
        }
        result = CARD_RESULT_READY;
        OSSignalCond(&CardControl.cond);
    }
    OSUnlockMutex(&CardControl.mutex);
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         CardUtilResultCode

  Description:  Gets the last result code

  Arguments:    None.

  Returns:      The last result code
 *---------------------------------------------------------------------------*/
s32 CardUtilResultCode(void)
{
    return CardControl.result;
}

/*---------------------------------------------------------------------------*
  Name:         CardUtilMount

  Description:  Requests the mount operation to the card utility thread.
                The implementation is in DoMount().

  Arguments:    slot        Memory card slot number
                workArea    Work area for CARDMount()

  Returns:      CARD_RESULT_READY if succeeded. Otherwise CARD_RESULT_BUSY.
 *---------------------------------------------------------------------------*/
s32 CardUtilMount(s32 slot, void* workArea)
{
    return CardUtilCommand(slot, CARDUTIL_CMD_MOUNT, 0, workArea);
}

/*---------------------------------------------------------------------------*
  Name:         CardUtilUnmount

  Description:  Requests the unmount operation to the card utility thread
                The implementation is in DoUnmount().

  Arguments:    slot        Memory card slot number

  Returns:      CARD_RESULT_READY if succeeded. Otherwise CARD_RESULT_BUSY.
 *---------------------------------------------------------------------------*/
s32 CardUtilUnmount(s32 slot)
{
    return CardUtilCommand(slot, CARDUTIL_CMD_UNMOUNT, 0, 0);
}

/*---------------------------------------------------------------------------*
  Name:         CardUtilList

  Description:  Requests the list operation to the card utility thread
                The implementation is in DoList().

  Arguments:    slot        Memory card slot number
                directory   Pointer to directory

  Returns:      CARD_RESULT_READY if succeeded. Otherwise CARD_RESULT_BUSY.
 *---------------------------------------------------------------------------*/
s32 CardUtilList(s32 slot, CardUtilDirent* directory)
{
    return CardUtilCommand(slot, CARDUTIL_CMD_LIST, 0, directory);
}

/*---------------------------------------------------------------------------*
  Name:         CardUtilFormat

  Description:  Requests the format operation to the card utility thread
                The implementation is in DoFormat().

  Arguments:    slot        Memory card slot number

  Returns:      CARD_RESULT_READY if succeeded. Otherwise CARD_RESULT_BUSY.
 *---------------------------------------------------------------------------*/
s32 CardUtilFormat(s32 slot)
{
    return CardUtilCommand(slot, CARDUTIL_CMD_FORMAT, 0, 0);
}

/*---------------------------------------------------------------------------*
  Name:         CardUtilErase

  Description:  Requests the erase operation to the card utility thread
                The implementation is in DoErase().

  Arguments:    slot        Memory card slot number

  Returns:      CARD_RESULT_READY if succeeded. Otherwise CARD_RESULT_BUSY.
 *---------------------------------------------------------------------------*/
s32 CardUtilErase(s32 slot, s32 fileNo)
{
    return CardUtilCommand(slot, CARDUTIL_CMD_ERASE, fileNo, 0);
}

/*---------------------------------------------------------------------------*
  Name:         CardUtilOpen

  Description:  Requests the open operation to the card utility thread
                The implementation is in DoOpen().

  Arguments:    slot        Memory card slot number
                fileNo      File number to open
                buffer      Pointer to read data buffer

  Returns:      CARD_RESULT_READY if succeeded. Otherwise CARD_RESULT_BUSY.
 *---------------------------------------------------------------------------*/
s32 CardUtilOpen(s32 slot, s32 fileNo, void* buffer)
{
    return CardUtilCommand(slot, CARDUTIL_CMD_OPEN, fileNo, buffer);
}

/*---------------------------------------------------------------------------*
  Name:         CardUtilSave

  Description:  Requests the save operation to the card utility thread
                The implementation is in DoOpen().

  Arguments:    slot        Memory card slot number
                stat        pointer to CARDStat. The following members must
                            be initialized properly.
                                fileName, length, iconAddr, commentAddr.
                buffer      Pointer to save data buffer. Banner, icon,
                            comment data must be properly set up in the
                            buffer.

  Returns:      CARD_RESULT_READY if succeeded. Otherwise CARD_RESULT_BUSY.
 *---------------------------------------------------------------------------*/
s32 CardUtilSave(s32 slot, CARDStat* stat, void* buffer)
{
    return CardUtilCommand(slot, CARDUTIL_CMD_SAVE, (s32) stat, buffer);
}

/*---------------------------------------------------------------------------*
  Name:         CardUtilMain

  Description:  The main program for the card utility thread. It takes the
                next command from CardControl and execute the specified
                operation.
 *---------------------------------------------------------------------------*/
static void* CardUtilMain(void* param)
{
    s32 slot;
    s32 command;
    s32 result;
    s32 fileNo;

    for (;;)
    {
        OSLockMutex(&CardControl.mutex);
        while (CardControl.slot == -1)
        {
            // No command is issued so far. Waits until any command
            // is issued.
            OSWaitCond(&CardControl.cond, &CardControl.mutex);
        }
        slot    = CardControl.slot;
        command = CardControl.command;
        fileNo  = CardControl.fileNo;
        param   = CardControl.param;
        OSUnlockMutex(&CardControl.mutex);

        switch (command)
        {
          case CARDUTIL_CMD_MOUNT:
            result = DoMount(slot, param);
            break;
          case CARDUTIL_CMD_UNMOUNT:
            result = DoUnmount(slot);
            break;
          case CARDUTIL_CMD_FORMAT:
            result = DoFormat(slot);
            break;
          case CARDUTIL_CMD_LIST:
            result = DoList(slot, param);
            break;
          case CARDUTIL_CMD_ERASE:
            result = DoErase(slot, fileNo);
            break;
          case CARDUTIL_CMD_OPEN:
            result = DoOpen(slot, fileNo, param);
            break;
          case CARDUTIL_CMD_SAVE:
            result = DoSave(slot, (CARDStat*) fileNo, param);
            break;
        }

        OSLockMutex(&CardControl.mutex);
        CardControl.result = result;
        CardControl.slot = -1;
        OSUnlockMutex(&CardControl.mutex);
    }
    return NULL;
}

/*---------------------------------------------------------------------------*
  Name:         CardUtilInit

  Description:  The card utility functions executes the requested command
                concurrently using thread. CardUtilInit() initializes the
                card utility thread.

  Arguments:    stackBase   Address of initial stack pointer. Note that
                            stacks grow DOWN, so this should be the highest
                            addressable location in the stack
                stackSize   Size of the stack
                priority    Base scheduling priority for card utility thread

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CardUtilInit(void* stackBase, u32 stackSize, OSPriority priority)
{
    ASSERT(sizeof(CardUtilDirent) % 32 == 0);
    CARDInit();
    OSInitMutex(&CardControl.mutex);
    OSInitMutex(&CardControl.mutexDirectory);
    OSInitCond(&CardControl.cond);
    OSCreateThread(
        &CardThread,
        CardUtilMain,
        0,
        stackBase,
        stackSize,
        priority,
        OS_THREAD_ATTR_DETACH);
    OSResumeThread(&CardThread);
}

/*---------------------------------------------------------------------------*
  Name:         CardUtilDrawIcon

  Description:  Simple utility function to draw banner and icon texture
                image to the screen.

  Arguments:    x           screen x pos
                y           screen y pos
                size        pixel width of image
                image       texture image
                tlut        tlut
                width       texel width of image
                height      texel height of image
                format      texture format

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CardUtilDrawIcon(int x, int y, int size, void* image, void* tlut, u16 width, u16 height, int format)
{
    GXTexObj  texObj;
    GXTlutObj tlutObj;
    Mtx       m;
    s16       posLeft   = (s16) x;
    s16       posRight  = (s16) (posLeft + size * width / width);
    s16       posTop    = (s16) y;
    s16       posBottom = (s16) (y + (height * size / width) * height / height);

    if (format == CARD_STAT_ICON_RGB5A3)
    {
        GXInitTexObj(&texObj,           // obj
                     image,             // image
                     width,             // wight
                     height,            // height
                     GX_TF_RGB5A3,      // format
                     GX_CLAMP,          // wrap_s      (don't care)
                     GX_CLAMP,          // wrap_t      (don't care)
                     GX_FALSE);         // mipmap      (don't care)
    }
    else if (format == CARD_STAT_ICON_C8)
    {
        ASSERT(format == CARD_STAT_ICON_C8);
        GXInitTlutObj(&tlutObj,
                      tlut,
                      GX_TL_RGB5A3,
                      256);

        GXInitTexObjCI(&texObj,         // obj
                       image,           // image
                       width,           // wight
                       height,          // height
                       GX_TF_C8,        // format
                       GX_CLAMP,        // wrap_s      (don't care)
                       GX_CLAMP,        // wrap_t      (don't care)
                       GX_FALSE,        // mipmap      (don't care)
                       GX_TLUT0);

        GXLoadTlut(&tlutObj, GX_TLUT0);
    }
    else
    {
        return;
    }

    GXLoadTexObj(&texObj, GX_TEXMAP0);
    MTXScale(m, 1.0f / width, 1.0f / height, 1.0f);
    GXLoadTexMtxImm(m, GX_TEXMTX0, GX_MTX2x4);
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);

    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);

    // Set up vertex descriptors
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS,  GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS,  GX_POS_XYZ, GX_S16, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST,  GX_S16, 0);

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);

    GXPosition3s16(posLeft , posTop   , 0);
    GXTexCoord2s16((s16) 0,           (s16) 0);

    GXPosition3s16(posRight, posTop   , 0);
    GXTexCoord2s16((s16) width,       (s16) 0);

    GXPosition3s16(posRight, posBottom, 0);
    GXTexCoord2s16((s16) width,       (s16) height);

    GXPosition3s16(posLeft , posBottom, 0);
    GXTexCoord2s16((s16) 0,           (s16) height);

    GXEnd();
}

/*---------------------------------------------------------------------------*
  Name:         CardUtilDrawIcon

  Description:  Simple utility function to draw animated icon texture.

  Arguments:    ent         pointer to directory entry to draw icon
                x           screen x pos
                y           screen y pos
                size        pixel width of image

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CardUtilDrawAnimatedIcon(CardUtilDirent* ent, int x, int y, int size)
{
    int i;
    u32 curr;

    if (ent->cFrame)
    {
        curr = VIGetRetraceCount() % ent->cFrame;
        for (i = 0; i < 2 * CARD_ICON_MAX - 2; ++i)
        {
            if (curr < ent->nFrame[i])
            {
                break;
            }
        }
        i = ent->iIcon[i];
        if (CARDGetIconFormat(&ent->stat, i) != CARD_STAT_ICON_NONE)
        {
            CardUtilDrawIcon(x, y, size,
                     (u8*) ent + ent->stat.offsetIcon[i]  - ent->stat.iconAddr,
                     (u8*) ent + ent->stat.offsetIconTlut - ent->stat.iconAddr,
                     CARD_ICON_WIDTH, CARD_ICON_HEIGHT,
                     CARDGetIconFormat(&ent->stat, i));
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CardUtilGetProgress

  Description:  Returns current command progress in percent (only for
                mount, format, save and erase commands).

  Arguments:    slot        Memory card slot number

  Returns:      current command progress in percent (0 to 100)
 *---------------------------------------------------------------------------*/
s32 CardUtilGetProgress(s32 slot)
{
    s32 percent;

    percent = CARDGetXferredBytes(slot) - CardControl.xferred;
    if (CardControl.length)
    {
        percent = (100 * percent) / CardControl.length;
    }
    if (100 < percent)
    {
        percent = 100;
    }
    return percent;
}
