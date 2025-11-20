/*---------------------------------------------------------------------*

Project:  mcc bitmap viewer - core loop manager module
File:     core.h

(C) 2000 Nintendo

-----------------------------------------------------------------------*/

#ifndef __CORE_LOOP
#define __CORE_LOOP

//	CALLBACK PROTOTYPES
// ================
typedef void* CORE_PARAM;
typedef void(*CORE_TASK)(CORE_PARAM param);
typedef void(*CORE_CALLBACK_ENUM)( u8 taskID );

//	FUNCTION PROTOTYPES
// ================
void	CoreBegin( CORE_TASK beginTask );
void	CoreBreak();

u8		CoreTaskAdd( CORE_TASK task, CORE_PARAM param, u8 priority, char* name );
void	CoreTaskDelete( u8 taskID );

u8		CoreTaskGetNumberOf();
u8		CoreTaskGetPriority( u8 taskID );
void	CoreTaskGetName( u8 taskID, char* name );
void	CoreTaskGetParam( u8 taskID, CORE_PARAM* patam );
//void	CoreTaskEnum( CORE_CALLBACK_ENUM callback );

#endif __CORE_LOOP