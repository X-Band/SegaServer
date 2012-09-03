
/*
	File:		Server_SendNGP.c

	Contains:	Server send NGP function

	Written by:	Dave Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <6>	 8/12/94	ATM		Converted to Logmsg.
		 <5>	 7/20/94	DJ		added Server_Comm stuff
		 <4>	 7/17/94	BET		Make printf's go to gLogFile instead of stdout, which is the
									modem.
		 <3>	  6/5/94	DJ		nothing
		 <2>	 5/27/94	DJ		Server_SendNGP is rubbish. delete it from project!
		 <1>	 5/25/94	DJ		first checked in
	To Do:
*/

#include "ServerCore.h"
#include "Server.h"
#include "Messages.h"
#include "UsrConfg.h"
#include <stdio.h>


// BRAIN DAMAGE  delete this file and function.

int Server_SendNGP()
{
/*
unsigned char 	opCode;
long			NGPVersion;

	Server_TReadDataSync( &aGame, 1, (Ptr)&opCode );
	if(opCode != msSendNGPVersion){
		// fucked
		return(kServerFuncAbort);
	}

	Server_TReadDataSync( &aGame, 4, (Ptr)&NGPVersion );


	Logmsg("NGP version = %ld\n", NGPVersion);
	Logmsg("Server_SendNGP done\n");
*/

	Logmsg("Server_SendNGP is totally fucked.  kill it!\n");

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
