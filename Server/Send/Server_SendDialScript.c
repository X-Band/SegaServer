/*
	File:		Server_SendDialScript.c

	Contains:	Dial script update message routines

	Written by:	Brian Topping

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <2>	 8/12/94	BET		Update dial script message
		 <1>	 8/12/94	BET		first checked in

	To Do:
*/

#include "ServerCore.h"
#include "Server.h"
#include "Messages.h"
#include "Server_Comm.h"
#include <stdio.h>

extern	FILE				*gLogFile;

int Server_SendDialScript(ServerState *state, ServiceScript *script, DBID id)
{
unsigned char 	opCode;
unsigned long	size;

	fprintf(gLogFile, "Server_SendDialScript\n");
	if(Server_DebugService(state) != kServerFuncOK)
		return(kServerFuncAbort);

	opCode = msSendDialScript;
	// have to subtract one from pairCount because  
	// the ServiceScript type already has one
	size = (sizeof(ServiceScript) + (sizeof(ServicePair) * (script->pairCount-1));
	
	if(Server_TWriteDataSync(state->session, 1, (Ptr)&id) != noErr)
		return(kServerFuncAbort);
	if(Server_TWriteDataSync(state->session, 1, (Ptr)&opCode) != noErr)
		return(kServerFuncAbort);
	if(Server_TWriteDataSync(state->session, size, (Ptr)script) != noErr)
		return(kServerFuncAbort);
	
	fprintf(gLogFile, "Server_SendDialScript done\n");

	return(kServerFuncOK);
}

