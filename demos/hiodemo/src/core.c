/*---------------------------------------------------------------------*

Project:  mcc bitmap viewer - core loop manager module
File:     core.c

(C) 2000 Nintendo

-----------------------------------------------------------------------*/

// HEADER INCLUDE
// ================
#include <dolphin.h>
#include <string.h>
#include <ctype.h>
#include "core.h"

// INTERNAL DEFINES
// ================
#define	CORE_TASK_NUM	16
typedef struct{
	CORE_TASK	task;
	u8			priority;
	void*		param;
	char		name[16];
} CORE_TASK_INFO;

// INTERNAL VARIABLES
// ================
static CORE_TASK_INFO gTaskInfo[CORE_TASK_NUM];
static BOOL bIsBreak;

// FUNCTION PROTOTYPES
// ================
static void CoreTaskInit(void);
static void CoreTaskWork(void);


// ============================================================
//	API
// ============================================================

// ========================================
//	CORE LOOP
// ========================================
//	Core loop manager
// ==============================
void CoreBegin( CORE_TASK beginTask )
{
	//	Initializes internal information.
	CoreTaskInit();
	//	Starts beginTask.
	if(beginTask)
		beginTask(NULL);
	//	Main loop
	bIsBreak=FALSE;
	while(!bIsBreak){
		CoreTaskWork();
	}
}

//	Terminates core loop.
// ==============================
void CoreBreak()
{	bIsBreak=TRUE;
}

// ========================================
//	ADD DELETE TASK
// ========================================
//	Adds to task list
// ==============================
u8 CoreTaskAdd( CORE_TASK task, void *param, u8 priority, char* name )
{	u8 taskID;
	//	Checks free task.
	for(taskID=0;taskID<CORE_TASK_NUM;taskID++){
		if(!gTaskInfo[taskID].task){
			//	Free task is found.
			gTaskInfo[taskID].task=task;
			gTaskInfo[taskID].param=param;
			gTaskInfo[taskID].priority=priority;
			if(name)
				memcpy(gTaskInfo[taskID].name,name,15),
				gTaskInfo[taskID].name[15]='\0';
			return taskID;
		}
	}
	return (u8)-1;
}

//	Deletes from task list.
// ==============================
void CoreTaskDelete( u8 taskID )
{
	gTaskInfo[taskID].task		= NULL;
	gTaskInfo[taskID].param		= NULL;
	gTaskInfo[taskID].priority	= 0;
	gTaskInfo[taskID].name[0]	='\0';
}

// ========================================
//	TASK INFO
// ========================================
//	Obtains the number of task registered in task information.
// ==============================
u8 CoreTaskGetNumberOf()
{	u8 taskID,count=0;
	for(taskID=0;taskID<CORE_TASK_NUM;taskID++)
		if(gTaskInfo[taskID].task)
			count++;
	return count;
}

//	Obtains task priority.
// ==============================
u8 CoreTaskGetPriority( u8 taskID )
{	return gTaskInfo[taskID].priority;
}

//	Obtains task name.
// ==============================
void CoreTaskGetName( u8 taskID, char* name )
{	if(name) strcpy( name,gTaskInfo[taskID].name );
}

//	Obtains task parameter.
// ==============================
void CoreTaskGetParam( u8 taskID, CORE_PARAM* patam)
{	if(patam)
		*patam = gTaskInfo[taskID].param;
}
/*
void CoreTaskEnum( CORE_CALLBACK_ENUM callback );
*/


// ============================================================
//	FUNCTIONS
// ============================================================

//	Initializes task information.
// ==============================
static void CoreTaskInit(void)
{	memset( (void*)gTaskInfo,0,sizeof(CORE_TASK_INFO)*CORE_TASK_NUM );
}

//	Starts task registerd in task information.
// ==============================
static void CoreTaskWork(void)
{	int taskID;
	for(taskID=0;taskID<CORE_TASK_NUM;taskID++){
		if(gTaskInfo[taskID].task)
			gTaskInfo[taskID].task( gTaskInfo[taskID].param );
	
	}
}