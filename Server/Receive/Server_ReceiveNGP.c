
/*
	File:		Server_ReceiveNGP.c

	Contains:	Server receive NGP function

	Written by:	Dave Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <8>	 8/12/94	ATM		Converted to Logmsg.
		 <7>	 7/20/94	DJ		added Server_Comm stuff
		 <6>	 7/17/94	BET		Make printf's go to gLogFile instead of stdout, which is the
									modem.
		 <5>	  7/1/94	DJ		making server handle errors from the comm layer
		 <4>	  6/9/94	BET		Fix hairy compiles based on TransportSlayer.h
		 <3>	  6/4/94	DJ		making everything take a ServerState instead of SessionRec
		 <2>	  6/1/94	DJ		removed byte gobbler
		 <1>	 5/25/94	DJ		first checked in
	To Do:
*/

#include "ServerCore.h"
#include "Server.h"
#include "Messages.h"
#include "UsrConfg.h"
#include "Server_Comm.h"
#include <stdio.h>


int Server_ReceiveNGP(ServerState *state)
{
unsigned char 	opCode;

	Logmsg("Server_ReceiveNGP\n");

	if(Server_TReadDataSync( state->session, 1, (Ptr)&opCode ) != noErr)
		return(kServerFuncAbort);

	if(opCode != msSendNGPVersion){
		// fucked
		Logmsg("Wrong opCode.  Expected %d, got %d\n", msSendNGPVersion, opCode);
		return(kServerFuncAbort);
	}

	if(Server_TReadDataSync( state->session, 4, (Ptr)&state->NGPVersion ) != noErr)
		return(kServerFuncAbort);

	state->validFlags |= kServerValidFlag_NGPVersion;

	Logmsg("NGP Version = %ld\n", state->NGPVersion);
	Logmsg("Server_ReceiveNGP done\n");

	return(kServerFuncOK);
}

/*
void DoSendNGPVersionOpCode( void * notUsed )
{
messOut	sendDummy;
long	NGPVersion;

	NGPVersion = GetNGPVersion();
	sendDummy = msSendNGPVersion;
	SendNetDataASync( msOutgoingOpCodeSize, (Ptr)&sendDummy );
	SendNetDataASync( 4, (Ptr)&NGPVersion );
}
*/
