/**
 * @file si.h
 * @brief Serial Interface (SI) API for libPorpoise
 * 
 * Serial Interface handles communication with controller ports.
 * On PC, PAD module uses SDL2 directly, but SI provides compatibility
 * for games that access SI registers or use SI functions.
 */

#ifndef DOLPHIN_SI_H
#define DOLPHIN_SI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <dolphin/types.h>

/*---------------------------------------------------------------------------*
    Constants
 *---------------------------------------------------------------------------*/

#define SI_MAX_CHAN    4  // Maximum number of SI channels (controllers)

/*---------------------------------------------------------------------------*
    Function Declarations
 *---------------------------------------------------------------------------*/

/**
 * @brief Initialize Serial Interface hardware
 * 
 * On GC/Wii: Initializes SI registers and interrupts
 * On PC: Initializes SI state tracking (PAD uses SDL2)
 */
void SIInit(void);

/**
 * @brief Get SI channel status register
 * 
 * @param chan  Channel number (0-3)
 * @return Status register value
 */
u32 SIGetStatus(s32 chan);

/**
 * @brief Get response data from SI channel
 * 
 * @param chan  Channel number
 * @param data  Pointer to receive response (2 u32s)
 * @return TRUE if response available
 */
BOOL SIGetResponse(s32 chan, u32* data);

/**
 * @brief Set SI command for channel
 * 
 * @param chan  Channel number
 * @param cmd   Command value
 */
void SISetCommand(s32 chan, u32 cmd);

/**
 * @brief Transfer data to/from SI device
 * 
 * @param chan      Channel number
 * @param output    Output buffer
 * @param outBytes  Output size
 * @param input     Input buffer
 * @param inBytes   Input size
 * @param callback  Callback when complete
 * @param delay     Delay (unused)
 * @return TRUE if transfer started
 */
BOOL SITransfer(s32 chan, void* output, u32 outBytes, void* input, u32 inBytes,
                void (*callback)(s32, u32, void*), u32 delay);

/**
 * @brief Get device type on SI channel
 * 
 * @param chan  Channel number
 * @return Device type (0x09000000 = standard controller)
 */
u32 SIGetType(s32 chan);

/**
 * @brief Get device type asynchronously
 * 
 * @param chan      Channel number
 * @param callback  Callback with type result
 * @return TRUE always
 */
BOOL SIGetTypeAsync(s32 chan, void (*callback)(s32, u32));

/**
 * @brief Enable automatic polling for channels
 * 
 * @param poll  Channel bits to enable
 */
void SIEnablePolling(u32 poll);

/**
 * @brief Disable automatic polling for channels
 * 
 * @param poll  Channel bits to disable
 */
void SIDisablePolling(u32 poll);

/**
 * @brief Check if SI is busy with transfer
 * 
 * @return FALSE always on PC (no hardware)
 */
BOOL SIBusy(void);

/**
 * @brief Check if specific channel is busy
 * 
 * @param chan  Channel number
 * @return FALSE always on PC
 */
BOOL SIIsChanBusy(s32 chan);

/**
 * @brief Set controller sampling rate
 * 
 * @param msec  Sampling rate in milliseconds
 */
void SISetSamplingRate(u32 msec);

/**
 * @brief Refresh sampling rate based on video mode
 */
void SIRefreshSamplingRate(void);

/**
 * @brief Register handler called during SI polling
 * 
 * @param handler  Handler function
 */
void SIRegisterPollingHandler(void (*handler)(void*, void*));

/**
 * @brief Unregister polling handler
 * 
 * @param handler  Handler function
 */
void SIUnregisterPollingHandler(void (*handler)(void*, void*));

/**
 * @brief Execute queued SI commands
 */
void SITransferCommands(void);

/**
 * @brief Set wireless controller ID
 * 
 * @param chan  Channel number
 * @param id    Wireless ID
 */
void OSSetWirelessID(s32 chan, u16 id);

#ifdef __cplusplus
}
#endif

#endif // DOLPHIN_SI_H

