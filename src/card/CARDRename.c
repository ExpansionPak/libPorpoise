/*---------------------------------------------------------------------------*
  CARDRename.c - File Rename Operations
 *---------------------------------------------------------------------------*/

#include <dolphin/card.h>
#include <dolphin/card_internal.h>
#include <dolphin/os.h>
#include <dolphin/porpoise/Guard.h>
#include <stdio.h>

/*---------------------------------------------------------------------------*
  Name:         CARDRename

  Description:  Rename file.

  Arguments:    chan     Card channel
                oldName  Current filename
                newName  New filename

  Returns:      CARD_RESULT_READY on success
 *---------------------------------------------------------------------------*/
s32 CARDRename(s32 chan, const char* oldName, const char* newName) {
    PP_GUARD_RET(chan >= 0 && chan < CARD_MAX_CHAN, CARD_RESULT_FATAL_ERROR, "invalid channel");
    PP_GUARD_PTR_RET(oldName, CARD_RESULT_FATAL_ERROR);
    PP_GUARD_PTR_RET(newName, CARD_RESULT_FATAL_ERROR);
    if (!__CARDCards[chan].mounted) {
        return CARD_RESULT_NOCARD;
    }
    
    char oldPath[512], newPath[512];
    __CARDBuildFilePath(chan, oldName, oldPath, sizeof(oldPath));
    __CARDBuildFilePath(chan, newName, newPath, sizeof(newPath));
    
    if (rename(oldPath, newPath) != 0) {
        return CARD_RESULT_NOFILE;
    }
    
    OSReport("CARD: Renamed '%s' → '%s'\n", oldName, newName);
    return CARD_RESULT_READY;
}

