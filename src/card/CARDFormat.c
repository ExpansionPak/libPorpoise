/*---------------------------------------------------------------------------*
  CARDFormat.c - Card Format Operations
 *---------------------------------------------------------------------------*/

#include <dolphin/card.h>
#include <dolphin/card_internal.h>
#include <dolphin/os.h>
#include <dolphin/porpoise/Guard.h>

/*---------------------------------------------------------------------------*
  Name:         CARDFormatAsync

  Description:  Format memory card (asynchronous).

  Arguments:    chan      Card channel
                callback  Completion callback

  Returns:      CARD_RESULT_READY on success
 *---------------------------------------------------------------------------*/
s32 CARDFormatAsync(s32 chan, CARDCallback callback) {
    PP_GUARD_RET(chan >= 0 && chan < CARD_MAX_CHAN, CARD_RESULT_FATAL_ERROR, "invalid channel");
    
    if (!CARDProbe(chan)) {
        return CARD_RESULT_NOCARD;
    }
    
    OSReport("CARD: Formatting slot %c...\n", 'A' + chan);
    
    __CARDCards[chan].formatted = TRUE;
    
    if (callback) {
        callback(chan, CARD_RESULT_READY);
    }
    
    return CARD_RESULT_READY;
}

/*---------------------------------------------------------------------------*
  Name:         CARDFormat

  Description:  Format memory card (synchronous).

  Arguments:    chan  Card channel

  Returns:      CARD_RESULT_READY on success
 *---------------------------------------------------------------------------*/
s32 CARDFormat(s32 chan) {
    return CARDFormatAsync(chan, NULL);
}

