
/*
	File:		Server_ReceiveSystemVersion.c

	Contains:	Server get box's system version function

	Written by:	Dave Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <8>	 8/12/94	ATM		Converted to Logmsg.
		 <7>	 7/20/94	DJ		added Server_Comm stuff
		 <6>	 7/17/94	BET		Make printf's go to gLogFile instead of stdout, which is the
									modem.
		 <5>	 7/12/94	DJ		using Server_TCheckError instead of TCheckError
		 <4>	  7/1/94	DJ		making server handle errors from the comm layer
		 <3>	  6/9/94	BET		Fix hairy compiles based on TransportSlayer.h
		 <2>	  6/4/94	DJ		making everything take a ServerState instead of SessionRec
	To Do:
*/

#include "ServerCore.h"
#include "Server.h"
#include "Messages.h"
#include "UsrConfg.h"
#include "Server_Comm.h"
#include <stdio.h>



int Server_ReceiveSystemVersion(ServerState *state)
{
unsigned char	opCode;

	Logmsg("Server_ReceiveSystemVersion\n");

	if(Server_TReadDataSync(state->session, 1, (Ptr)&opCode ) != noErr)
		return(kServerFuncAbort);

	if(opCode != msSystemVersion){
		// fucked
		Logmsg("Wrong opCode.  Expected %d, got %d\n", msSystemVersion, opCode);
		return(kServerFuncAbort);
	}
	Server_TReadDataSync( state->session, 2, (Ptr)&state->systemVersionData.length );
	Server_TReadDataSync( state->session, 4, (Ptr)&state->systemVersionData.version );

	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	state->validFlags |= kServerValidFlag_SystemVersion;
	
	Logmsg("length = %d, version = %ld\n", state->systemVersionData.length, state->systemVersionData.version);
	Logmsg("Server_ReceiveSystemVersion done\n");

	return(kServerFuncOK);
}

/*
void DoSendSystemVersionOpCode( void * notUsed )
{
messOut		sendDummy;
short		length;
long		versionVersion;

	sendDummy = msSystemVersion;
	SendNetDataASync( msOutgoingOpCodeSize, (Ptr)&sendDummy );
	length = sizeof( versionVersion );
	SendNetDataASync( 2, (Ptr)&length );
	versionVersion = 0;
	SendNetDataASync( 4, (Ptr)&versionVersion );
}
*/
