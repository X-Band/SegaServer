/*
	File:		Server_SendBoxSerialNumber.c

	Contains:	xxx put contents here xxx

	Written by:	David Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <8>	 8/12/94	ATM		Converted to Logmsg.
		 <7>	 7/20/94	DJ		added Server_Comm stuff
		 <6>	 7/17/94	BET		Make printf's go to gLogFile instead of stdout, which is the
									modem.
		 <5>	 7/12/94	DJ		sending new boxserialnumber format
		 <4>	  7/1/94	DJ		making server handle errors from the comm layer
		 <3>	  6/9/94	BET		Fix hairy compiles based on TransportSlayer.h
		 <2>	  6/5/94	DJ		making everything take a ServerState instead of SessionRec and
									added Server_DebugService
		 <1>	 5/31/94	DJ		first checked in

	To Do:
*/


#include "ServerCore.h"
#include "Server.h"
#include "ServerState.h"
#include "Messages.h"
#include "Server_Comm.h"
#include <stdio.h>


//
// sends a new serial number to the box.  this is used only once when the box initially registers
// with Catapult.  We could add all sorts of wiggy hardcore security by disabling the
// handler for this message.  We could only run it if we downloaded it first.  Very nice!
//
int Server_SendNewBoxSerialNumber(ServerState *state, BoxSerialNumber *boxSerialNumber)
{
messOut	opCode;

	Logmsg("Server_SendNewBoxSerialNumber\n");
	if(Server_DebugService(state) != kServerFuncOK)
		return(kServerFuncAbort);

	opCode = msSetBoxSerialNumber;
	Server_TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);
	Server_TWriteDataSync(state->session, sizeof(BoxSerialNumber), (Ptr)boxSerialNumber);

	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	Logmsg("Server_SendNewBoxSerialNumber done\n");

	return(kServerFuncOK);
}

int Server_SendNewBoxHometown(ServerState *state, Hometown town)
{
messOut	opCode;

	Logmsg("Server_SendNewBoxHometown\n");
	if(Server_DebugService(state) != kServerFuncOK)
		return(kServerFuncAbort);

	opCode = msSetBoxHometown;
	Server_TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);
	Server_TWriteDataSync(state->session, sizeof(Hometown), (Ptr)town);

	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	Logmsg("Server_SendNewBoxHometown done\n");

	return(kServerFuncOK);
}

int Server_SendNewCurrentUserName(ServerState *state, UserName name)
{
messOut	opCode;

	Logmsg("Server_SendNewCurrentUserName\n");
	if(Server_DebugService(state) != kServerFuncOK)
		return(kServerFuncAbort);

	opCode = msSetCurrentUserName;
	Server_TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);
	Server_TWriteDataSync(state->session, sizeof(UserName), (Ptr)name);

	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	Logmsg("Server_SendNewCurrentUserName done\n");

	return(kServerFuncOK);
}

