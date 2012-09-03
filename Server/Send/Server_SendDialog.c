/*
	File:		Server_SendDialog.c

	Contains:	xxx put contents here xxx

	Written by:	Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<10>	 8/22/94	DJ		kDDASAFP works now
		 <9>	 8/18/94	DJ		don't send sticky dialogs when in redial test mode
		 <8>	 8/17/94	DJ		sticky and kDDASAFP
		 <8>	 8/17/94	DJ		sticky always, and use kDDASAFP
		 <7>	 8/12/94	ATM		Converted to Logmsg.
		 <6>	  8/8/94	DJ		SendDialog takes boolean whether to stick or disappear in 3 sec.
		 <5>	 7/25/94	BET		Add gLogFile changes to printf strings.
		 <4>	 7/20/94	DJ		added Server_Comm stuff
		 <3>	 7/15/94	DJ		added large dialog
		 <2>	 7/12/94	DJ		using Server_TCheckError instead of TCheckError
		 <1>	  7/4/94	DJ		first checked in

	To Do:
*/


#include "ServerCore.h"
#include "Server.h"
#include "ServerDataBase.h"

#include "Messages.h"
#include "DialogMgr.h"
#include "DeferredDialogMgr.h"
#include "Server_Comm.h"

#include <string.h>
#include <stdio.h>

//
// OLDDIALOGS won't work for pre 'seg4' boxes because the kDDServerConnectDoneImmediate constant and its peers changed.
// 8/16/94   -dj
//
#define OLDkDDServerConnectDoneImmediate	5		/* for older ROM versions */
#define	STICKYDIALOGS						true	/* try sticky dialogs all the time */

#define kServerDialogMinTime	120	/* 2 seconds */
#define kServerDialogMaxTime	300	/* 5 seconds */


int Server_SendDialog(ServerState *state, char *str, Boolean sticky)
{
short	minTime, maxTime, dialogStick;

#ifdef STICKYDIALOGS
	sticky = true;		// Try the box out with sticky dialogs all the time
#endif

	if(state->boxOSState.boxType == kBoxType0 || 
		state->boxOSState.boxType == kBoxType1 || 
		state->boxOSState.boxType == kBoxType2 || 
		state->boxOSState.boxType == kBoxType3)
			dialogStick = OLDkDDServerConnectDoneImmediate;
	else
			dialogStick = kDDASAFP;



	if((state->validFlags & kServerValidFlag_Account)
		&& state->account->playerAccount.debug0 > 0)
	{
	//
	// The redial hack.
	//
//		dialogStick = kDDServerConnectDoneImmediate;
		sticky = false;
	}



	if(sticky)
		minTime = maxTime = 0;
	else
	{
		minTime = kServerDialogMinTime;
		maxTime = kServerDialogMaxTime;
	}

	return(Server_SendQDefDialog(state, dialogStick, str, (DBID)kMediumDialog, minTime, maxTime));
}


int Server_SendLargeDialog(ServerState *state, char *str, Boolean sticky)
{
short	minTime, maxTime, dialogStick;

#ifdef STICKYDIALOGS
	sticky = true;		// Try the box out with sticky dialogs all the time
#endif

	if(state->boxOSState.boxType == kBoxType0 || 
		state->boxOSState.boxType == kBoxType1 || 
		state->boxOSState.boxType == kBoxType2 || 
		state->boxOSState.boxType == kBoxType3)
			dialogStick = OLDkDDServerConnectDoneImmediate;
	else
			dialogStick = kDDASAFP;



	if((state->validFlags & kServerValidFlag_Account)
		&& state->account->playerAccount.debug0 > 0)
	{
	//
	// The redial hack.
	//
//		dialogStick = kDDServerConnectDoneImmediate;
		sticky = false;
	}



	if(sticky)
		minTime = maxTime = 0;
	else
	{
		minTime = kServerDialogMinTime;
		maxTime = kServerDialogMaxTime;
	}



	return(Server_SendQDefDialog(state, dialogStick, str, (DBID)kLargeDialog, minTime, maxTime));
}

int Server_SendQDefDialog(ServerState *state, short when, char *cString, DBID templat, short minTime, short maxTime)
{
long	length;
messOut	opCode;

Logmsg("Server_SendQDefDialog\n");

	opCode = msQDefDialog;
	Server_TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);

	length = strlen(cString) + 1;
	Server_TWriteDataSync(state->session, sizeof(short), (Ptr)&when);
	Server_TWriteDataSync(state->session, sizeof(DBID), (Ptr)&templat);
	Server_TWriteDataSync(state->session, sizeof(short), (Ptr)&minTime);
	Server_TWriteDataSync(state->session, sizeof(short), (Ptr)&maxTime);
	Server_TWriteDataSync(state->session, sizeof(long), (Ptr)&length);
	Server_TWriteDataSync(state->session, length, (Ptr)cString);
	
	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

Logmsg("Server_SendQDefDialog done\n");

	return(kServerFuncOK);
}
