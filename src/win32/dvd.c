//============================================================================
// PC Dolphin Emulator Version 2.6
// Nintendo of America, Inc.
// Product Support Group
// Dante Treglia II
// danttr01@noa.nintendo.com
// April 12, 2000
// Copyright (c) 2000 Nintendo of America Inc.
//============================================================================

#include <dolphin.h>
#include <win32/win32.h>
#include <errno.h>
#include <sys/stat.h>

BOOL DVDOpen (char* fileName, DVDFileInfo* fileInfo) {
		struct _stat fileStat;

	_stat(fileName, &fileStat);
	fileInfo->length = fileStat.st_size;
	fileInfo->file = fopen(fileName, "rbw");
	assert(fileInfo->file != NULL && "DVDOpen: Couldn't open file");
	return (fileInfo->file != NULL);
}


s32 DVDReadPrio (DVDFileInfo* fileInfo, 
			 void* addr, s32 length, 
			 s32 offset, s32 prio) {
	u32 bytesRead;

	assert(fileInfo->file != NULL && "DVDRead: File not open");
	fseek(fileInfo->file, offset, SEEK_SET);
	bytesRead = fread(addr, sizeof(char), length, fileInfo->file);
	return bytesRead;
}

BOOL DVDClose (DVDFileInfo* fileInfo) {
	fclose(fileInfo->file);
	return 1;
}

void DVDSetRoot(const char *rootPath) {
	SetCurrentDirectory(rootPath);
}

BOOL DVDChangeDir(char* dirName) {
	SetCurrentDirectory(dirName);
	return TRUE;
}

void DVDInit(void) {
}

