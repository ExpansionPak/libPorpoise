/*---------------------------------------------------------------------------*
  Project:  Dolphin AX stream demo
  File:     axstream.c

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/axdemo/src/axstream.c $
    
    6     12/11/01 7:02p Billyjack
    - keep interrupts disabled during audio frame callback
    
    5     8/03/01 4:32p Billyjack
    added OSEnableInterrupts() and OSRestoreInterrupts() to AX aufdio frame
    callback per change in AX lib.
    
    4     7/06/01 11:50a Billyjack
    commented DCInvalidateRange for DVD to RAM transfers
    
    3     5/14/01 1:39p Billyjack
    - reworked for DVDDATA file location and names
    - uses ARGetBaseAddress where applicable
    
    2     5/09/01 6:09p Billyjack
    now uses the mix lib
    
    1     3/26/01 2:32p Billyjack
    created
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <string.h>
#include <demo.h>
#include <dolphin.h>
#include <dolphin/mix.h>

#define STREAM_BLOCK_LEN        0x1000

#define STREAM_BUFFERL_START    (4096)                                          // in bytes
#define STREAM_BUFFERL_HALF     (STREAM_BUFFERL_START + STREAM_BLOCK_LEN)       // in bytes
#define STREAM_BUFFERL_END      (STREAM_BUFFERL_START + (STREAM_BLOCK_LEN * 2)) // in bytes

#define STREAM_BUFFERR_START    (STREAM_BUFFERL_START + (STREAM_BLOCK_LEN * 2)) // in bytes
#define STREAM_BUFFERR_HALF     (STREAM_BUFFERR_START + STREAM_BLOCK_LEN)       // in bytes
#define STREAM_BUFFERR_END      (STREAM_BUFFERR_START + (STREAM_BLOCK_LEN * 2)) // in bytes

static u32          frames;
static u32          flag;

#define STREAM_NONE         0
#define STREAM_INITIALIZING 1
#define STREAM_STARTED      2
#define STREAM_STOPPING     3
#define STREAM_STOPPED      4

static AXVPB        *streamL;
static AXVPB        *streamR;

static u32          aramBufferL0;
static u32          aramBufferL1;
static u32          aramBufferLEnd;
static u32          aramBufferR0;
static u32          aramBufferR1;
static u32          aramBufferREnd;
static u32          streamLastPositionR;

static u32          ramBufferL;         // base address for left buffer 
static u32          ramBufferR;         // base address for right buffer
static u32          ramBufferLen;       // length in bytes
static u32          ramBufferLPcm16;    // base address for left buffer 
static u32          ramBufferRPcm16;    // base address for right buffer
static u32          ramBufferLenPcm16;  // length in bytes
static u32          ramBufferLPcm8;     // base address for left buffer 
static u32          ramBufferRPcm8;     // base address for right buffer
static u32          ramBufferLenPcm8;   // length in bytes
static u32          ramBufferLAdpcm;    // base address for left buffer 
static u32          ramBufferRAdpcm;    // base address for right buffer
static u32          ramBufferLenAdpcm;  // length in bytes

static u32          ramBufferPosition;  // read position in byte offset

static ARQRequest   taskL;
static ARQRequest   taskR;

static u8           streamBufferL[STREAM_BLOCK_LEN]; 
static u8           streamBufferR[STREAM_BLOCK_LEN];

/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void fillAram(int buffer)
{
    u32 aramByteAddressL;
    u32 aramByteAddressR;
   
    // set ARAM destination
    if (buffer)
    {
        aramByteAddressL = STREAM_BUFFERL_HALF;
        aramByteAddressR = STREAM_BUFFERR_HALF;
    }
    else
    {
        aramByteAddressL = STREAM_BUFFERL_START;
        aramByteAddressR = STREAM_BUFFERR_START;
    }

    // check to see if we are passed the end of the input buffer
    if (ramBufferPosition == ramBufferLen) // passed end, fill with 0s
    {
        memset(streamBufferL, 0, STREAM_BLOCK_LEN);
        memset(streamBufferR, 0, STREAM_BLOCK_LEN);
    }
    else
    {
        u32 bytesLeft = ramBufferLen - ramBufferPosition; 

        // first see if we have enough buffers left for the entire 4K
        if (bytesLeft >= STREAM_BLOCK_LEN)
        {
            memcpy(streamBufferL, (void*)((u8*)ramBufferL + ramBufferPosition), STREAM_BLOCK_LEN); 
            memcpy(streamBufferR, (void*)((u8*)ramBufferR + ramBufferPosition), STREAM_BLOCK_LEN); 

            ramBufferPosition += STREAM_BLOCK_LEN;
        }
        else
        {
            memset(streamBufferL, 0, STREAM_BLOCK_LEN);
            memset(streamBufferR, 0, STREAM_BLOCK_LEN);
            
            if (bytesLeft)
            {
                memcpy(streamBufferL, (void*)(ramBufferL + ramBufferPosition), bytesLeft); 
                memcpy(streamBufferR, (void*)(ramBufferR + ramBufferPosition), bytesLeft);
            }
            
            ramBufferPosition = ramBufferLen;
        }
    }

    if (buffer == 0)    // if we just updated the first buffer program loop context for ADPCM
    {
        AXPBADPCMLOOP   loop;        

        loop.loop_pred_scale    = (u16)(*((u8*)(streamBufferL)));
        loop.loop_yn1           = 0;
        loop.loop_yn2           = 0;

        AXSetVoiceAdpcmLoop(streamL, &loop);

        loop.loop_pred_scale    = (u16)(*((u8*)(streamBufferR)));
        loop.loop_yn1           = 0;
        loop.loop_yn2           = 0;

        AXSetVoiceAdpcmLoop(streamR, &loop);
    }

    DCFlushRange(streamBufferL, STREAM_BLOCK_LEN);
    DCFlushRange(streamBufferR, STREAM_BLOCK_LEN);
    
    ARQPostRequest(
        &taskL,
        0,
        ARQ_TYPE_MRAM_TO_ARAM,
        ARQ_PRIORITY_HIGH,
        (u32)streamBufferL,
        aramByteAddressL,
        STREAM_BLOCK_LEN,
        NULL
        );

    ARQPostRequest(
        &taskR,
        0,
        ARQ_TYPE_MRAM_TO_ARAM,
        ARQ_PRIORITY_HIGH,
        (u32)streamBufferR,
        aramByteAddressR,
        STREAM_BLOCK_LEN,
        NULL
        );
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void callbackAudioFrame(void)
{
    // tell the mixer to update settings
    MIXUpdateSettings();

    switch (flag)
    {
    case STREAM_NONE:   
    case STREAM_INITIALIZING:
        
        break;

    case STREAM_STARTED:
        
        if (streamR)
        {
            u32 currentPosition;

            // get current position of stream
            currentPosition = *((u32*)&streamR->pb.addr.currentAddressHi);

            // see if we need to fill any buffers
            if (currentPosition < streamLastPositionR)  // fill buffer 1
                fillAram(1);
            
            if ((currentPosition >= aramBufferR1) &&
                (streamLastPositionR < aramBufferR1))    // fill buffer 0
                fillAram(0);

            streamLastPositionR = currentPosition;
        }

        break;

    case STREAM_STOPPING:
        
        if(streamL)
        {
            MIXReleaseChannel(streamL);
            AXFreeVoice(streamL);
            streamL = NULL;
        }
    
        if(streamR)
        {
            MIXReleaseChannel(streamR);
            AXFreeVoice(streamR);
            streamR = NULL;
        }

        flag = STREAM_STOPPED;

        break;

    case STREAM_STOPPED:

        flag = STREAM_NONE;
        
        break;

    }
    
    frames++;
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void startVoices(u32 task)
{
    ARQRequest *p = (ARQRequest*)task;

    if (streamL)    AXSetVoiceState(streamL, AX_PB_STATE_RUN);
    if (streamR)    AXSetVoiceState(streamR, AX_PB_STATE_RUN);

    flag = STREAM_STARTED;
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void startStreamPcm16(void)
{
    if (flag != STREAM_NONE)
        return;

    // allocate voices for use
    streamL = AXAcquireVoice(15, NULL, 0);
    streamR = AXAcquireVoice(15, NULL, 0);

    // set source buffer to PCM16 data
    ramBufferL      = ramBufferLPcm16;
    ramBufferR      = ramBufferRPcm16;
    ramBufferLen    = ramBufferLenPcm16;

    // initialize the ARAM buffer addresses in word mode
    aramBufferL0    = STREAM_BUFFERL_START  / 2;
    aramBufferL1    = STREAM_BUFFERL_HALF   / 2;
    aramBufferLEnd  = (STREAM_BUFFERL_END    / 2) - 1;
    aramBufferR0    = STREAM_BUFFERR_START  / 2;
    aramBufferR1    = STREAM_BUFFERR_HALF   / 2;
    aramBufferREnd  = (STREAM_BUFFERR_END    / 2) - 1;

    // setup the voice, kick off a ARQ task to fill the buffer and use the
    // callback to start the voice
    if (streamL)
    {
        AXPBADDR        addr;

        MIXInitChannel(streamL, 0, 0, -904, -904, 0, 127, 0);

        addr.loopFlag           = AXPBADDR_LOOP_ON;
        addr.format             = AX_PB_FORMAT_PCM16;   
        addr.loopAddressHi      = (u16)(aramBufferL0 >> 16);       
        addr.loopAddressLo      = (u16)(aramBufferL0 & 0xFFFF);       
        addr.endAddressHi       = (u16)(aramBufferLEnd >> 16);       
        addr.endAddressLo       = (u16)(aramBufferLEnd & 0xFFFF);       
        addr.currentAddressHi   = (u16)(aramBufferL0 >> 16);       
        addr.currentAddressLo   = (u16)(aramBufferL0 & 0xFFFF);

        AXSetVoiceAddr(streamL, &addr);                     // input addressing
        AXSetVoiceSrcType(streamL, AX_SRC_TYPE_NONE);       // no SRC
    }

    if (streamR)
    {
        AXPBADDR        addr;

        MIXInitChannel(streamR, 0, 0, -904, -904, 127, 127, 0);

        addr.loopFlag           = AXPBADDR_LOOP_ON;
        addr.format             = AX_PB_FORMAT_PCM16;   
        addr.loopAddressHi      = (u16)(aramBufferR0 >> 16);       
        addr.loopAddressLo      = (u16)(aramBufferR0 & 0xFFFF);       
        addr.endAddressHi       = (u16)(aramBufferREnd >> 16);       
        addr.endAddressLo       = (u16)(aramBufferREnd & 0xFFFF);       
        addr.currentAddressHi   = (u16)(aramBufferR0 >> 16);       
        addr.currentAddressLo   = (u16)(aramBufferR0 & 0xFFFF);

        AXSetVoiceAddr(streamR, &addr);                   // input addressing
        AXSetVoiceSrcType(streamR, AX_SRC_TYPE_NONE);     // no SRC
    }
    
    // use the right side for buffer position
    streamLastPositionR = 0;
    
    // fill the ARAM buffers then use the ARQ callback to start
    // the voices
    ARQPostRequest(
        &taskL,
        0,
        ARQ_TYPE_MRAM_TO_ARAM,
        ARQ_PRIORITY_HIGH,
        (u32)ramBufferL,
        STREAM_BUFFERL_START,
        STREAM_BLOCK_LEN * 2,
        NULL
        );

    ARQPostRequest(
        &taskR,
        0,
        ARQ_TYPE_MRAM_TO_ARAM,
        ARQ_PRIORITY_HIGH,
        (u32)ramBufferR,
        STREAM_BUFFERR_START,
        STREAM_BLOCK_LEN * 2,
        &startVoices
        );

    ramBufferPosition   = STREAM_BLOCK_LEN * 2;
    streamLastPositionR = aramBufferR0;

    flag = STREAM_INITIALIZING;
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void startStreamPcm8(void)
{
    if (flag != STREAM_NONE)
        return;

    // allocate a voice for use
    streamL = AXAcquireVoice(15, NULL, 0);
    streamR = AXAcquireVoice(15, NULL, 0);

    // set source buffer to PCM16 data
    ramBufferL      = ramBufferLPcm8;
    ramBufferR      = ramBufferRPcm8;
    ramBufferLen    = ramBufferLenPcm8;

    // initialize the ARAM buffer addresses in word mode
    aramBufferL0    = STREAM_BUFFERL_START;
    aramBufferL1    = STREAM_BUFFERL_HALF;
    aramBufferLEnd  = STREAM_BUFFERL_END - 1;
    aramBufferR0    = STREAM_BUFFERR_START;
    aramBufferR1    = STREAM_BUFFERR_HALF;
    aramBufferREnd  = STREAM_BUFFERR_END - 1;

    // setup the voice, kick off a ARQ task to fill the buffer and use the
    // callback to start the voice
    if (streamL)
    {
        AXPBADDR        addr;

        MIXInitChannel(streamL, 0, 0, -904, -904, 0, 127, 0);

        addr.loopFlag           = AXPBADDR_LOOP_ON;
        addr.format             = AX_PB_FORMAT_PCM8;   
        addr.loopAddressHi      = (u16)(aramBufferL0 >> 16);       
        addr.loopAddressLo      = (u16)(aramBufferL0 & 0xFFFF);       
        addr.endAddressHi       = (u16)(aramBufferLEnd >> 16);       
        addr.endAddressLo       = (u16)(aramBufferLEnd & 0xFFFF);       
        addr.currentAddressHi   = (u16)(aramBufferL0 >> 16);       
        addr.currentAddressLo   = (u16)(aramBufferL0 & 0xFFFF);

        AXSetVoiceAddr(streamL, &addr);                     // input addressing
        AXSetVoiceSrcType(streamL, AX_SRC_TYPE_NONE);       // no SRC
    }

    if (streamR)
    {
        AXPBADDR        addr;

        MIXInitChannel(streamR, 0, 0, -904, -904, 127, 127, 0);

        addr.loopFlag           = AXPBADDR_LOOP_ON;
        addr.format             = AX_PB_FORMAT_PCM8;   
        addr.loopAddressHi      = (u16)(aramBufferR0 >> 16);       
        addr.loopAddressLo      = (u16)(aramBufferR0 & 0xFFFF);       
        addr.endAddressHi       = (u16)(aramBufferREnd >> 16);       
        addr.endAddressLo       = (u16)(aramBufferREnd & 0xFFFF);       
        addr.currentAddressHi   = (u16)(aramBufferR0 >> 16);       
        addr.currentAddressLo   = (u16)(aramBufferR0 & 0xFFFF);

        AXSetVoiceAddr(streamR, &addr);                     // input addressing
        AXSetVoiceSrcType(streamR, AX_SRC_TYPE_NONE);       // no SRC
    }
    
    // use the right side for buffer position
    streamLastPositionR = 0;
    
    // fill the ARAM buffers then use the ARQ callback to start
    // the voices
    ARQPostRequest(
        &taskL,
        0,
        ARQ_TYPE_MRAM_TO_ARAM,
        ARQ_PRIORITY_HIGH,
        (u32)ramBufferL,
        STREAM_BUFFERL_START,
        STREAM_BLOCK_LEN * 2,
        NULL
        );

    ARQPostRequest(
        &taskR,
        0,
        ARQ_TYPE_MRAM_TO_ARAM,
        ARQ_PRIORITY_HIGH,
        (u32)ramBufferR,
        STREAM_BUFFERR_START,
        STREAM_BLOCK_LEN * 2,
        &startVoices
        );

    ramBufferPosition   = STREAM_BLOCK_LEN * 2;
    streamLastPositionR = aramBufferR0;

    flag = STREAM_INITIALIZING;
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void startStreamAdpcm(void)
{
    if (flag != STREAM_NONE)
        return;

    // allocate a voice for use
    streamL = AXAcquireVoice(15, NULL, 0);
    streamR = AXAcquireVoice(15, NULL, 0);

    // set source buffer to ADPCM data
    ramBufferL      = ramBufferLAdpcm;
    ramBufferR      = ramBufferRAdpcm;
    ramBufferLen    = ramBufferLenAdpcm;

    // initialize the ARAM buffer addresses in nibble mode, keep in mind that
    // it is illegal to put start, loop, or end address on the ADPCM frame 
    // header
    aramBufferL0    = (STREAM_BUFFERL_START  * 2) + 2;  // +2 to skip header
    aramBufferL1    = STREAM_BUFFERL_HALF   * 2;
    aramBufferLEnd  = (STREAM_BUFFERL_END    * 2) -1;    
    aramBufferR0    = (STREAM_BUFFERR_START  * 2) + 2;  // +2 to skip header
    aramBufferR1    = STREAM_BUFFERR_HALF   * 2;
    aramBufferREnd  = (STREAM_BUFFERR_END    * 2) -1;   

    // setup the voice, kick off a ARQ task to fill the buffer and use the
    // callback to start the voice
    if (streamL)
    {
        AXPBADDR        addr;
        AXPBADPCM       adpcm;

        MIXInitChannel(streamL, 0, 0, -904, -904, 0, 127, 0);

        addr.loopFlag           = AXPBADDR_LOOP_ON;
        addr.format             = AX_PB_FORMAT_ADPCM;   
        addr.loopAddressHi      = (u16)(aramBufferL0 >> 16);              
        addr.loopAddressLo      = (u16)(aramBufferL0 & 0xFFFF);       
        addr.endAddressHi       = (u16)(aramBufferLEnd >> 16);       
        addr.endAddressLo       = (u16)(aramBufferLEnd & 0xFFFF);       
        addr.currentAddressHi   = (u16)(aramBufferL0 >> 16);       
        addr.currentAddressLo   = (u16)(aramBufferL0 & 0xFFFF);

        adpcm.a[0][0]           = 0x0136;
        adpcm.a[1][0]           = 0x06b2;
        adpcm.a[2][0]           = 0x029f;
        adpcm.a[3][0]           = 0x0a05;
        adpcm.a[4][0]           = 0x047d;
        adpcm.a[5][0]           = 0x0742;
        adpcm.a[6][0]           = 0x04ce;
        adpcm.a[7][0]           = 0x0c87;
        adpcm.a[0][1]           = 0xfe78;
        adpcm.a[1][1]           = 0xff09;
        adpcm.a[2][1]           = 0x038d;
        adpcm.a[3][1]           = 0xfd47;
        adpcm.a[4][1]           = 0xff11;
        adpcm.a[5][1]           = 0x0005;
        adpcm.a[6][1]           = 0x02b7;
        adpcm.a[7][1]           = 0xfb49;
        adpcm.gain              = 0;
        adpcm.pred_scale        = (u16)*((u8*)ramBufferL);
        adpcm.yn1               = 0;
        adpcm.yn2               = 0;

        AXSetVoiceType(streamL, AX_PB_TYPE_STREAM);         // no loop context
        AXSetVoiceAddr(streamL, &addr);                     // input addressing
        AXSetVoiceSrcType(streamL, AX_SRC_TYPE_NONE);       // no SRC
        AXSetVoiceAdpcm(streamL, &adpcm);                   // ADPCM coefficients
    }

    if (streamR)
    {
        AXPBADDR        addr;
        AXPBADPCM       adpcm;

        MIXInitChannel(streamR, 0, 0, -904, -904, 127, 127, 0);

        addr.loopFlag           = AXPBADDR_LOOP_ON;
        addr.format             = AX_PB_FORMAT_ADPCM;   
        addr.loopAddressHi      = (u16)(aramBufferR0 >> 16);       
        addr.loopAddressLo      = (u16)(aramBufferR0 & 0xFFFF);       
        addr.endAddressHi       = (u16)(aramBufferREnd >> 16);       
        addr.endAddressLo       = (u16)(aramBufferREnd & 0xFFFF);       
        addr.currentAddressHi   = (u16)(aramBufferR0 >> 16);       
        addr.currentAddressLo   = (u16)(aramBufferR0 & 0xFFFF);

        adpcm.a[0][0]           = 0x00bf;
        adpcm.a[1][0]           = 0x0661;
        adpcm.a[2][0]           = 0x02b9;
        adpcm.a[3][0]           = 0x0a35;
        adpcm.a[4][0]           = 0x03ca;
        adpcm.a[5][0]           = 0x0732;
        adpcm.a[6][0]           = 0x0499;
        adpcm.a[7][0]           = 0x0cf9;
        adpcm.a[0][1]           = 0xffc8;
        adpcm.a[1][1]           = 0xff43;
        adpcm.a[2][1]           = 0x037d;
        adpcm.a[3][1]           = 0xfd14;
        adpcm.a[4][1]           = 0x000e;
        adpcm.a[5][1]           = 0x0018;
        adpcm.a[6][1]           = 0x02ee;
        adpcm.a[7][1]           = 0xfad5;
        adpcm.gain              = 0;
        adpcm.pred_scale        = (u16)*((u8*)ramBufferR);
        adpcm.yn1               = 0;
        adpcm.yn2               = 0;

        AXSetVoiceType(streamR, AX_PB_TYPE_STREAM);       // no loop context
        AXSetVoiceAddr(streamR, &addr);                   // input addressing
        AXSetVoiceSrcType(streamR, AX_SRC_TYPE_NONE);     // no SRC
        AXSetVoiceAdpcm(streamR, &adpcm);                 // ADPCM coefficients
    }
    
    // use the right side for buffer position
    streamLastPositionR = 0;
    
    // fill the ARAM buffers then use the ARQ callback to start
    // the voices
    ARQPostRequest(
        &taskL,
        0,
        ARQ_TYPE_MRAM_TO_ARAM,
        ARQ_PRIORITY_HIGH,
        (u32)ramBufferL,
        STREAM_BUFFERL_START,
        STREAM_BLOCK_LEN * 2,
        NULL
        );

    ARQPostRequest(
        &taskR,
        0,
        ARQ_TYPE_MRAM_TO_ARAM,
        ARQ_PRIORITY_HIGH,
        (u32)ramBufferR,
        STREAM_BUFFERR_START,
        STREAM_BLOCK_LEN * 2,
        &startVoices
        );

    ramBufferPosition   = STREAM_BLOCK_LEN * 2;
    streamLastPositionR = aramBufferR0;

    flag = STREAM_INITIALIZING;
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void stopStream(void)
{
    if (flag != STREAM_STARTED)
        return;

    if (streamR)
    {
        MIXSetFader(streamL, -960);
        MIXSetFader(streamR, -960);
    }

    flag = STREAM_STOPPING;
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
#define mROUNDUP32(x)      (((u32)(x) + 32 - 1) & ~(32 - 1))
static u32 putSamplesIntoRam(char *path)
{
    DVDFileInfo handle;
    u32         round_length;
    s32         read_length;
    void        *buffer;

    // sine64.raw
    if (!DVDOpen(path, &handle))
    {
        char ch[1024];

        sprintf(ch, "Cannot open %s", path);

        ASSERTMSG(0, ch);
    }

    // make sure file length s not 0
    ASSERTMSG(DVDGetLength(&handle), "File length is 0\n");

    round_length    = mROUNDUP32(DVDGetLength(&handle));
    buffer          = OSAlloc(round_length);

    // make sure we got a buffer
    ASSERTMSG(buffer, "Unable to allocate buffer\n");

    read_length     = DVDRead(&handle, buffer, (s32)(round_length), 0);

//    DCInvalidateRange(buffer, (u32)read_length);

    // make sure we read the file correctly
    ASSERTMSG(read_length > 0, "Read length <= 0\n");

    return (u32)buffer;
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
#define MAX_ARAM_BLOCKS     2
static u32 aramMemArray[MAX_ARAM_BLOCKS];

void main(void)
{
    PADStatus    pads[PAD_MAX_CONTROLLERS];

    DEMOInit(NULL);

    ARInit(aramMemArray, MAX_ARAM_BLOCKS);
    ARQInit();
    AIInit(NULL);

    AXInit();
    MIXInit();

    frames  = 0;

    // put samples in to RAM
    ramBufferLPcm16     = putSamplesIntoRam("/axdemo/stream/left.pcm16");
    ramBufferRPcm16     = putSamplesIntoRam("/axdemo/stream/right.pcm16");
    ramBufferLenPcm16   = 827260;

    ramBufferLPcm8      = putSamplesIntoRam("/axdemo/stream/left.pcm8");
    ramBufferRPcm8      = putSamplesIntoRam("/axdemo/stream/right.pcm8");
    ramBufferLenPcm8    = 413635;

    ramBufferLAdpcm     = putSamplesIntoRam("/axdemo/stream/left.adpcm");
    ramBufferRAdpcm     = putSamplesIntoRam("/axdemo/stream/right.adpcm");
    ramBufferLenAdpcm   = 236368;

    // register user callback for audio frames notification
    AXRegisterCallback(&callbackAudioFrame);

    OSReport("Press the A button to play PCM16 stream.\n");
    OSReport("Press the B button to play PCM8 stream.\n");
    OSReport("Press the X button to play DSP ADPCM stream.\n");
    OSReport("Press the Y button to stop the stream.\n");

    OSReport("Press Menu button to exit program\n");

    while (1)
    {

if (streamR)
    OSReport("L:%x R:%x\n",
            *(u32*)&streamL->pb.addr.currentAddressHi,
            *(u32*)&streamR->pb.addr.currentAddressHi
            );
        
        // wait for retrace
        VIWaitForRetrace();

        // check pad
        PADRead(pads);

        // see if we should quit
        if (pads[0].button & PAD_BUTTON_MENU)
            break;

        // run PCM16 stream
        if (pads[0].button & PAD_BUTTON_A)
            startStreamPcm16();

        // run PCM8 stream
        if (pads[0].button & PAD_BUTTON_B)
            startStreamPcm8();

        // run ADPCM stream
        if (pads[0].button & PAD_BUTTON_X)
            startStreamAdpcm();

        // stop stream
        if (pads[0].button & PAD_BUTTON_Y)
            stopStream();
    }

    MIXQuit();
    AXQuit();

    OSReport("AX ran %x audio frames\n", frames);

    OSHalt("End of program\n");    
}