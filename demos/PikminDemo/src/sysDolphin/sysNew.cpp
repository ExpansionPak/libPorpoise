#include "sysNew.h"

#include "DebugLog.h"
#include "MemStat.h"

/*
 * --INFO--
 * Address:	........
 * Size:	00009C
 */
DEFINE_ERROR(6)

/*
 * --INFO--
 * Address:	........
 * Size:	0000F0
 */
DEFINE_PRINT("sysNew");

/*
 * --INFO--
 * Address:	80047004
 * Size:	000164
 */
void* System::alloc(size_t size)
{
	void* result = nullptr;
	if (size & 0x3) {
		size = (size + 3) & ~0x3;
	}

	if (gsys->mActiveHeapIdx < 0) {
#if 0
		// The DLL uses `GlobalAlloc` here and has an ERROR if that fails.  This branch of code is probably DLL exclusive,
		// since the GCN can't just ask WinAPI for unlimited memory.  We'll see once JPN Demo version matching begins.
		if (!result) {
			ERROR("new[] %d failed", size);
		}
#endif
	} else {
		AyuHeap* heap = &gsys->mHeaps[gsys->mActiveHeapIdx];
		if (size == 0) {
			PRINT("trying to allocate %d bytes on heap\n", 0);
		}
		result = heap->push(static_cast<int>(size));
		if (!result) {
			ERROR("new[] %d failed in heap '%s'", gsys->mActiveHeapIdx, size);
		}

		if (size == 0 || gsys->mForcePrint) {
			bool print         = gsys->mTogglePrint;
			gsys->mTogglePrint = TRUE;
			gsys->mTogglePrint = print;
		}

		MemInfo* info = gsys->mCurrMemInfo;
		while (info) {
			info->mMemorySize += size;
			info = static_cast<MemInfo*>(info->mParent);
		}

		if (reinterpret_cast<uintptr_t>(result) & 0x3) {
			((uintptr_t)result);
		}

		int length = static_cast<int>(size / 4);
		for (int i = 0; i < length; i++) {
			((u32*)result)[i] = 0;
		}
	}

	return result;
}

/*
 * --INFO--
 * Address:	........
 * Size:	000044
 */
void* operator new(size_t size, int alignment)
{
	if (alignment <= 0) {
		alignment = alignof(void*);
	}

	uintptr_t alloc = reinterpret_cast<uintptr_t>(System::alloc(size + alignment));
	if (!alloc) {
		return nullptr;
	}
	uintptr_t result = (alloc + (alignment - 1)) & ~(static_cast<uintptr_t>(alignment) - 1);
	return reinterpret_cast<void*>(result);
}

/*
 * --INFO--
 * Address:	80047168
 * Size:	000044
 */
void* operator new[](size_t size, int alignment)
{
	if (alignment <= 0) {
		alignment = alignof(void*);
	}

	uintptr_t alloc  = reinterpret_cast<uintptr_t>(System::alloc(size + alignment));
	uintptr_t result = (alloc + (alignment - 1)) & ~(alignment - 1);
	return reinterpret_cast<void*>(result);
}

/*
 * --INFO--
 * Address:	800471AC
 * Size:	000004
 */
void operator delete(void*)
{
}

/*
 * --INFO--
 * Address:	800471B0
 * Size:	000004
 */
void operator delete[](void*)
{
}
