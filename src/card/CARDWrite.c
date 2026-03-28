/*---------------------------------------------------------------------------*
  CARDWrite.c - File Write Operations
 *---------------------------------------------------------------------------*/

#include <dolphin/card.h>
#include <dolphin/card_internal.h>
#include <dolphin/os.h>
#include <dolphin/porpoise/Guard.h>
#include <stdio.h>

/*---------------------------------------------------------------------------*
  Name:         CARDWriteAsync

  Description:  Write to file (asynchronous).

  Arguments:    fileInfo  File info
                buf       Source buffer
                length    Bytes to write
                offset    File offset
                callback  Completion callback

  Returns:      Bytes written, or error
 *---------------------------------------------------------------------------*/
s32 CARDWriteAsync(CARDFileInfo* fileInfo, const void* buf, s32 length,
                   s32 offset, CARDCallback callback) {
    PP_GUARD_PTR_RET(fileInfo, CARD_RESULT_FATAL_ERROR);
    PP_GUARD_PTR_RET(buf, CARD_RESULT_FATAL_ERROR);
    PP_GUARD_NONNEG_RET(length, CARD_RESULT_FATAL_ERROR);
    PP_GUARD_NONNEG_RET(offset, CARD_RESULT_FATAL_ERROR);
    
    s32 chan = fileInfo->chan;
    PP_GUARD_RET(chan >= 0 && chan < CARD_MAX_CHAN, CARD_RESULT_FATAL_ERROR, "invalid channel");
    
    if (!__CARDCards[chan].mounted) {
        return CARD_RESULT_NOCARD;
    }
    
    // Get filename from file number
    s32 fileNo = fileInfo->fileNo;
    PP_GUARD_RET(fileNo >= 0 && fileNo < 127 && __CARDCards[chan].openFiles[fileNo][0] != '\0',
                 CARD_RESULT_FATAL_ERROR, "file is not open");
    
    const char* fileName = __CARDCards[chan].openFiles[fileNo];
    char path[512];
    __CARDBuildFilePath(chan, fileName, path, sizeof(path));
    
    // Write to actual file (read+write mode to preserve existing data)
    FILE* file = fopen(path, "r+b");
    if (!file) {
        // Try creating if doesn't exist
        file = fopen(path, "w+b");
        if (!file) {
            return CARD_RESULT_IOERROR;
        }
    }
    
    if (fseek(file, offset, SEEK_SET) != 0) {
        fclose(file);
        return CARD_RESULT_IOERROR;
    }
    
    size_t bytesWritten = fwrite(buf, 1, length, file);
    fclose(file);
    
    if (callback) {
        callback(chan, (s32)bytesWritten);
    }
    
    return (s32)bytesWritten;
}

/*---------------------------------------------------------------------------*
  Name:         CARDWrite

  Description:  Write to file (synchronous).

  Arguments:    fileInfo  File info
                buf       Source buffer
                length    Bytes to write
                offset    File offset

  Returns:      Bytes written, or error
 *---------------------------------------------------------------------------*/
s32 CARDWrite(CARDFileInfo* fileInfo, const void* buf, s32 length, s32 offset) {
    return CARDWriteAsync(fileInfo, buf, length, offset, NULL);
}

