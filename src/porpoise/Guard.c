#include <dolphin/os.h>
#include <dolphin/porpoise/Guard.h>

void PPGuardFailure(const char* file, int line, const char* expr, const char* detail) {
    const char* safeExpr = expr ? expr : "<unknown>";
    const char* safeDetail = detail ? detail : "";

#if PORPOISE_GUARD_MODE == PORPOISE_GUARD_MODE_PANIC
    OSPanic(file, line, "Guard failure: %s (%s)", safeExpr, safeDetail);
#elif PORPOISE_GUARD_MODE == PORPOISE_GUARD_MODE_LOG_RETURN
    if (safeDetail[0] != '\0') {
        OSReport("[Guard] %s:%d: %s (%s)\n", file, line, safeExpr, safeDetail);
    } else {
        OSReport("[Guard] %s:%d: %s\n", file, line, safeExpr);
    }
#else
    (void)file;
    (void)line;
    (void)safeExpr;
    (void)safeDetail;
#endif
}
