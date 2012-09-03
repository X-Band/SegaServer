/*
	File:		Server_SendPreformedMesg.c

	Contains:	xxx put contents here xxx

	Written by:	David Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <9>	 8/12/94	ATM		Converted to Logmsg.
		 <8>	 7/20/94	DJ		added Server_Comm stuff
		 <7>	 7/17/94	BET		Make printf's go to gLogFile instead of stdout, which is the
									modem.
		 <6>	  7/1/94	DJ		making server handle errors from the comm layer
		 <6>	  7/1/94	DJ		making server handle errors from the comm layer
		 <5>	  6/9/94	BET		Fix hairy compiles based on TransportSlayer.h
		 <4>	  6/5/94	DJ		making everything take a ServerState instead of SessionRec and
									added Server_DebugService
		 <3>	  6/2/94	BET		tweak
		 <2>	 5/29/94	DJ		sync writing instead of async
		 <1>	 5/27/94	DJ		first checked in

	To Do:
*/

#include "ServerCore.h"
#include "Server.h"
#include "ServerState.h"
#include "Server_Comm.h"

#include <stdio.h>


//
// sends a preformed message (eg. mail, patch, Zeus-generated bitmap).
// message is a data chunk that contains opCode and the data.
//
int Server_SendPreformedMessage(ServerState *state, PreformedMessage *mesg)
{
	Logmsg("Server_SendPreformedMessage\n");
	if(Server_DebugService(state) != kServerFuncOK)
		return(kServerFuncAbort);

	if(Server_TWriteDataSync(state->session, mesg->length, mesg->message) != noErr)
		return(kServerFuncAbort);

	return(kServerFuncOK);
}

