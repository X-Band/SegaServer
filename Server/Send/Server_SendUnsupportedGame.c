/*
	File:		Server_SendUnsupportedGame.c

	Contains:	xxx put contents here xxx

	Written by:	Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<14>	 8/22/94	DJ		groomed dialog text
		<13>	 8/17/94	DJ		moved routines between files
		<12>	 8/16/94	DJ		dialogs are sticky
		<11>	 8/12/94	ATM		Converted to Logmsg.
		<10>	  8/8/94	DJ		SendDialog takes boolean whether to stick or disappear in 3 sec.
		 <9>	 7/25/94	DJ		5 days of hacking, including: sending competitive challenge info
		 <8>	 7/20/94	DJ		added Server_Comm stuff
		 <7>	 7/19/94	DJ		gameName is now just a char*
		 <6>	 7/18/94	DJ		gamename is a single string, not 2
		 <5>	 7/17/94	BET		Make printf's go to gLogFile instead of stdout, which is the
									modem.
		 <4>	 7/13/94	DJ		added Server_SendRegisterPlayer
		 <3>	  7/2/94	DJ		made it send a dialog instead of msUnsupportedGame message
		 <2>	  7/1/94	DJ		making server handle errors from the comm layer
		 <1>	 6/11/94	DJ		first checked in

	To Do:
*/



#include "ServerCore.h"
#include "Server.h"
#include "Messages.h"
#include "Server_Comm.h"
#include <stdio.h>


int Server_SendUnsupportedGame(ServerState *state)
{
	Logmsg("Server_SendUnsupportedGame\n");
	
	Server_SendDialog(state, "This game is not yet available on XBAND", true);

/*
unsigned char 	opCode;

	if(Server_DebugService(state) != kServerFuncOK)
		return(kServerFuncAbort);

	opCode = msUnsupportedGame;
	if(Server_TWriteDataSync(state->session, 1, (Ptr)&opCode) != noErr)
		return(kServerFuncAbort);

	Logmsg("Server_SendUnsupportedGame done\n");
*/

	return(kServerFuncOK);
}

