
/*
	File:		Server_EndCommunication.c

	Contains:	Server end communication function

	Written by:	Dave Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<12>	 8/13/94	DJ		added forceendcomm (but it isn't used)
		<11>	 8/12/94	ATM		Converted to Logmsg.
		<10>	 7/29/94	DJ		added validation token and problem token
		 <9>	 7/20/94	DJ		added Server_Comm stuff
		 <8>	 7/17/94	BET		Make printf's go to gLogFile instead of stdout, which is the
									modem.
		 <7>	 7/12/94	DJ		using Server_TCheckError instead of TCheckError
		 <6>	  7/2/94	DJ		removed unused variable
		 <5>	  7/1/94	DJ		making server handle errors from the comm layer
		 <4>	  6/9/94	BET		Fix hairy compiles based on TransportSlayer.h
		 <3>	  6/5/94	DJ		making everything take a ServerState instead of SessionRec and
									added Server_DebugService
		 <2>	 5/29/94	DJ		sync writing instead of async
		 <1>	 5/25/94	DJ		first checked in
	To Do:
*/

#include "ServerCore.h"
#include "Server.h"
#include "Messages.h"
#include "Server_Comm.h"
#include <stdio.h>


int Server_EndCommunication(ServerState *state)
{
unsigned char opCode;

	Logmsg("Server_EndCommunication\n");

	if(Server_DebugService(state) != kServerFuncOK)
		return(kServerFuncAbort);


	opCode = msEndOfStream;
	Server_TWriteDataSync(state->session, 1, (Ptr)&opCode );
	Server_TReadDataSync( state->session, 1, (Ptr)&opCode );

	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	if(opCode != msEndOfStream)
	{
		//
		// Even though this is end of service, I'll return an abort just for fun.
		//
		Logmsg("Error: Box didn't respond to msEndOfStream with its own msEndOfStream\n");
		return(kServerFuncAbort);
	}

	Logmsg("Server_EndCommunication done\n");

	return(kServerFuncOK);
}

//
// Same as Server_EndCommunication, except doesn't wait for the box to respond with own
// msEndOfStream.
//
// Unused 8/13/94
//
int Server_ForceEndCommunication(ServerState *state)
{
unsigned char opCode;

	Logmsg("Server_ForceEndCommunication\n");

	if(Server_DebugService(state) != kServerFuncOK)
		return(kServerFuncAbort);


	opCode = msEndOfStream;
	Server_TWriteDataSync(state->session, 1, (Ptr)&opCode );

	Logmsg("Server_ForceEndCommunication done\n");

	return(kServerFuncOK);
}

//
// Send a new problem token
//
int Server_SendProblemToken(ServerState *state)
{
unsigned long token;
unsigned char opCode;

	Logmsg("Server_SendProblemToken\n");

	token = 23;
	opCode = msReceiveProblemToken;
	Server_TWriteDataSync(state->session, 1, (Ptr)&opCode );
	Server_TWriteDataSync(state->session, sizeof(long), (Ptr)&token );

	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	Logmsg("Server_SendProblemToken done\n");

	return(kServerFuncOK);
}

//
// Send a new validation token
//
int Server_SendValidationToken(ServerState *state)
{
unsigned long token;
unsigned char opCode;

	Logmsg("Server_SendValidationToken\n");

	token = 987654321;
	opCode = msReceiveValidationToken;
	Server_TWriteDataSync(state->session, 1, (Ptr)&opCode );
	Server_TWriteDataSync(state->session, sizeof(long), (Ptr)&token );

	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	Logmsg("Server_SendValidationToken done\n");

	return(kServerFuncOK);
}

