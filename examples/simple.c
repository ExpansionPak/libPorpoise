#include <dolphin/os.h>

int main(void) {
    OSReport("Initializing libPorpoise...\n");
    
    // Initialize the OS
    OSInit();
    
    // Get current time
    OSTime time = OSGetTime();
    OSReport("Current time: %lld ticks\n", (long long)time);
    
    // Get console type
    u32 consoleType = OSGetConsoleType();
    OSReport("Console type: 0x%08X\n", consoleType);
    
    // Convert time to calendar
    OSCalendarTime cal = {0};
    OSTicksToCalendarTime(time, &cal);
    OSReport("Calendar time: %04d-%02d-%02d %02d:%02d:%02d\n",
             cal.year, cal.mon + 1, cal.mday,
             cal.hour, cal.min, cal.sec);
    
    OSReport("Simple example completed successfully!\n");
    
    return 0;
}

