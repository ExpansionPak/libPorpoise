/*---------------------------------------------------------------------------*
  CARDStat.c - File Statistics Operations
 *---------------------------------------------------------------------------*/

#include <dolphin/card.h>
#include <dolphin/card_internal.h>
#include <dolphin/porpoise/Guard.h>
#include <string.h>

/*---------------------------------------------------------------------------*
  Name:         CARDGetStatus

  Description:  Get file statistics.

  Arguments:    chan    Card channel
                fileNo  File number
                stat    Stat structure to fill

  Returns:      CARD_RESULT_READY on success
 *---------------------------------------------------------------------------*/
s32 CARDGetStatus(s32 chan, s32 fileNo, CARDStat* stat) {
    PP_GUARD_RET(chan >= 0 && chan < CARD_MAX_CHAN, CARD_RESULT_FATAL_ERROR, "invalid channel");
    PP_GUARD_RET(fileNo >= 0 && fileNo < 127, CARD_RESULT_FATAL_ERROR, "invalid file number");
    PP_GUARD_PTR_RET(stat, CARD_RESULT_FATAL_ERROR);
    memset(stat, 0, sizeof(CARDStat));
    
    return CARD_RESULT_READY;
}

/*---------------------------------------------------------------------------*
  Name:         CARDSetStatus

  Description:  Set file statistics.

  Arguments:    chan    Card channel
                fileNo  File number
                stat    Stat structure

  Returns:      CARD_RESULT_READY on success
 *---------------------------------------------------------------------------*/
s32 CARDSetStatus(s32 chan, s32 fileNo, CARDStat* stat) {
    PP_GUARD_RET(chan >= 0 && chan < CARD_MAX_CHAN, CARD_RESULT_FATAL_ERROR, "invalid channel");
    PP_GUARD_RET(fileNo >= 0 && fileNo < 127, CARD_RESULT_FATAL_ERROR, "invalid file number");
    PP_GUARD_PTR_RET(stat, CARD_RESULT_FATAL_ERROR);
    
    return CARD_RESULT_READY;
}

/*---------------------------------------------------------------------------*
  Name:         CARDGetStatusEx

  Description:  Get file statistics (from file info).

  Arguments:    chan      Card channel
                fileInfo  File info
                stat      Stat structure

  Returns:      CARD_RESULT_READY on success
 *---------------------------------------------------------------------------*/
s32 CARDGetStatusEx(s32 chan, const CARDFileInfo* fileInfo, CARDStat* stat) {
    PP_GUARD_RET(chan >= 0 && chan < CARD_MAX_CHAN, CARD_RESULT_FATAL_ERROR, "invalid channel");
    PP_GUARD_PTR_RET(fileInfo, CARD_RESULT_FATAL_ERROR);
    PP_GUARD_PTR_RET(stat, CARD_RESULT_FATAL_ERROR);
    memset(stat, 0, sizeof(CARDStat));
    
    return CARD_RESULT_READY;
}

/*---------------------------------------------------------------------------*
  Name:         CARDSetStatusEx

  Description:  Set file statistics (from file info).

  Arguments:    chan      Card channel
                fileInfo  File info
                stat      Stat structure

  Returns:      CARD_RESULT_READY on success
 *---------------------------------------------------------------------------*/
s32 CARDSetStatusEx(s32 chan, CARDFileInfo* fileInfo, CARDStat* stat) {
    PP_GUARD_RET(chan >= 0 && chan < CARD_MAX_CHAN, CARD_RESULT_FATAL_ERROR, "invalid channel");
    PP_GUARD_PTR_RET(fileInfo, CARD_RESULT_FATAL_ERROR);
    PP_GUARD_PTR_RET(stat, CARD_RESULT_FATAL_ERROR);
    
    return CARD_RESULT_READY;
}

