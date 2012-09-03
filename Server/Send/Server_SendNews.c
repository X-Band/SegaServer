/*
	File:		Server_SendNews.c

	Contains:	xxx put contents here xxx

	Written by:	Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<11>	 8/12/94	ATM		Converted to Logmsg.
		<10>	 7/20/94	DJ		added Server_Comm stuff
		 <9>	 7/18/94	DJ		doesn't resend dailynews
		 <8>	 7/17/94	BET		Make printf's go to gLogFile instead of stdout, which is the
									modem.
		 <7>	 7/12/94	DJ		using Server_TCheckError instead of TCheckError
		 <6>	  7/1/94	DJ		making server handle errors from the comm layer
		 <5>	 6/30/94	DJ		new news
		 <4>	 6/28/94	DJ		added newsfalgs byte
		 <3>	  6/9/94	BET		Fix hairy compiles based on TransportSlayer.h
		 <2>	  6/5/94	DJ		making everything take a ServerState instead of SessionRec and
									added Server_DebugService
		 <1>	  6/2/94	DJ		first checked in

	To Do:
*/


#include "ServerCore.h"
#include "Server.h"
#include "ServerDataBase.h"
#include "Server_Comm.h"

#include "Messages.h"
#include "News.h"

#include <stdio.h>


//
// Tells the box to empty out its daily news and then sends the first page of the new news.
//
// Someday this shouldn't resend old news, etc.
//
int Server_SendFirstNewsPage(ServerState *state, ServerNewsPage *page)
{
messOut			opCode;
unsigned char 	newsFlags;
Err				err;

	Logmsg("Server_SendFirstNewsPage\n");
	if(Server_DebugService(state) != kServerFuncOK)
		return(kServerFuncAbort);

	ASSERT(state);
	ASSERT(page);
	if(!page)
		return(kServerFuncOK);

	opCode = msNewsHeader;
	Server_TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);
	Server_TWriteDataSync(state->session, sizeof(DBType), (Ptr)&page->type);

	newsFlags = kNewDaysNewsFlag;
	Server_TWriteDataSync(state->session, sizeof(char), (Ptr)&newsFlags);

	if((err = Server_TCheckError()) != noErr)
	{
		Logmsg("Server_TCheckError == %ld\n", err);
		return(kServerFuncAbort);
	}

	return(Server_SendNewsPage(state, page));
}

int Server_SendNoNewsPage(ServerState *state, DBType pagetype)
{
messOut			opCode;
unsigned char 	newsFlags;
Err				err;

	Logmsg("Server_SendNoNewsPage\n");
	if(Server_DebugService(state) != kServerFuncOK)
		return(kServerFuncAbort);

	ASSERT(state);

	opCode = msNewsHeader;
	Server_TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);
	Server_TWriteDataSync(state->session, sizeof(DBType), (Ptr)&pagetype);

	newsFlags = 0;	// no new news
	Server_TWriteDataSync(state->session, sizeof(char), (Ptr)&newsFlags);

	if((err = Server_TCheckError()) != noErr)
	{
		Logmsg("Server_TCheckError == %ld\n", err);
		return(kServerFuncAbort);
	}

	return(kServerFuncOK);
}

int Server_SendNewsPage(ServerState *state, ServerNewsPage *page)
{
messOut	opCode;
Err		err;

	Logmsg("Server_SendNewsPage\n");
	if(Server_DebugService(state) != kServerFuncOK)
		return(kServerFuncAbort);

	ASSERT(state);
	ASSERT(page);
	if(!page)
		return(kServerFuncOK);

	opCode = msNewsPage;
	Server_TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);
	Server_TWriteDataSync(state->session, sizeof(DBType), (Ptr)&page->type);
	Server_TWriteDataSync(state->session, sizeof(long), (Ptr)&page->length);
	Server_TWriteDataSync(state->session, page->length, (Ptr)page->page);

	if((err = Server_TCheckError()) != noErr)
	{
		Logmsg("Server_TCheckError == %ld\n", err);
		return(kServerFuncAbort);
	}

	return(kServerFuncOK);
}
