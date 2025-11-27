//============================================================================
// PC Dolphin Emulator Version 2.6
// Nintendo of America, Inc.
// Product Support Group
// Dante Treglia II
// danttr01@noa.nintendo.com
// April 12, 2000
// Copyright (c) 2000 Nintendo of America Inc.
//============================================================================


#include <win32/win32.h>
#include <dolphin.h>

FILE * outfile;


// volatile because some functions use this as hidden macro parameter
volatile OSHeapHandle   __OSCurrHeap = -1;

void* OSAllocFromHeap(OSHeapHandle heap, u32 size) {
	//return (void *)OSRoundUp32B(malloc(OSRoundUp32B(size)));
	return malloc(size);
}

void OSFreeToHeap(OSHeapHandle heap, void* ptr) {
	free(ptr);
}

void OSReport(char* msg, ...) {
	DWORD cCharsWritten;
	char s[256];
	va_list argptr;
	int cnt;

	va_start(argptr, msg);
	cnt = vsprintf(s, msg, argptr);

	// Outfile
	if (outfile) vfprintf(outfile, msg, argptr);

	va_end(argptr);

	if (outputWinHandle)
		WriteConsole(outputWinHandle, s, strlen(s), &cCharsWritten, NULL);
}

void OSPanic(char* file, int line, char* msg, ...) {
	DWORD cCharsWritten;
	char s[256];
	char s1[256];
	va_list argptr;
	int cnt;

	va_start(argptr, msg);
	cnt = vsprintf(s, msg, argptr);

	// OutFile
	if (outfile) vfprintf(outfile, msg, argptr);
	if (outfile) fprintf(outfile, " in \"%s\" on line %d.\n", file, line);

	va_end(argptr);

	sprintf(s1, " in \"%s\" on line %d.\n", file, line);
	strcat(s, s1);

	if (outputWinHandle) {
		WriteConsole(outputWinHandle, s, strlen(s), &cCharsWritten, NULL);
	}

	DebugBreak();
	//WinDestroy();
}


OSTime OSGetTime ( void ) {
	return GetTickCount();
}


OSTick OSGetTick ( void ) {
	return GetTickCount();
}

void OSInit(void) {
	outfile = fopen("output.txt", "w");
}

void DCInvalidateRange(void* start, u32 nBytes) {
}

void DCFlushRange(void* start, u32 nBytes) {
}

void DCStoreRange(void* start, u32 nBytes) {
}

void ICInvalidateRange(void* start, u32 nBytes) {
}

OSHeapHandle OSSetCurrentHeap(OSHeapHandle heap) {
	return 0;
}

void* OSInitAlloc(void* arenaStart, void* arenaEnd, int maxHeaps) {
	return 0;
}

OSHeapHandle OSCreateHeap(void* start, void* end) {	
	return 0;
}

void* OSGetArenaHi(void) {
	return 0;
}

void* OSGetArenaLo(void) {
	return 0;
}

void OSSetArenaHi(void* newHi) {
}

void OSSetArenaLo(void* newLo) {
}

BOOL OSDisableInterrupts(void) {
	return 0;
}

BOOL OSEnableInterrupts(void) {
	return 1;
}

BOOL OSRestoreInterrupts(BOOL level) {
	return 1;
}

void        OSInitStopwatch     ( OSStopwatch* sw, char* name )
{
    sw->name    = name;
    sw->total   = 0;
    sw->hits    = 0;

    // XXX should use limits
    sw->min     = 0xFFFFFFFF;
    sw->max     = 0;
}

void        OSStartStopwatch    ( OSStopwatch* sw )
{
    sw->running = TRUE;
    sw->last    = OSGetTime();
}

void        OSStopStopwatch     ( OSStopwatch* sw )
{
    OSTime  interval;

    if (sw->running)
    {
        interval    = OSGetTime() - sw->last;
        sw->total   += interval;
        sw->running = FALSE;
        sw->hits++;

        if (interval > sw->max)
        {
            sw->max = interval;
        }

        if (interval < sw->min)
        {
            sw->min = interval;
        }
    }
}

OSTime      OSCheckStopwatch    ( OSStopwatch* sw )
{
    OSTime      currTotal = sw->total;

    if (sw->running)
    {
        currTotal += OSGetTime() - sw->last;
    }
    return  currTotal;
}

void        OSResetStopwatch    ( OSStopwatch* sw )
{
    OSInitStopwatch(sw, sw->name);
}

void        OSDumpStopwatch     ( OSStopwatch* sw )
{
    OSReport("Stopwatch [%s]\t:\n", sw->name);
    OSReport("\tTotal= %lld us\n", OSTicksToMicroseconds(sw->total));
    OSReport("\tHits = %d \n", sw->hits);
    OSReport("\tMin  = %lld us\n", OSTicksToMicroseconds(sw->min));
    OSReport("\tMax  = %lld us\n", OSTicksToMicroseconds(sw->max));
    OSReport("\tMean = %lld us\n", OSTicksToMicroseconds(sw->total/sw->hits));
}

/*
void    OSInitStopwatch     ( OSStopwatch* sw, char* name ) {};
void    OSStartStopwatch    ( OSStopwatch* sw ) {};
void    OSStopStopwatch     ( OSStopwatch* sw ) {};
OSTime  OSCheckStopwatch    ( OSStopwatch* sw ) {return 0;};
void    OSResetStopwatch    ( OSStopwatch* sw ) {};
void    OSDumpStopwatch     ( OSStopwatch* sw ) {};
*/
