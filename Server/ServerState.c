/*
	File:		ServerState.c

	Contains:	ServerState

	Written by:	Dave Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<30>	  9/7/94	ATM		Moved the Statusmsg off to StartGamePlay.
		<29>	 8/30/94	ATM		Converted accountChangedMask references to Account flags.
									Changed userID printing.
		<28>	 8/26/94	ATM		Added a Logmsg to avoid confusion.
		<27>	 8/25/94	ATM		(doh!)
		<26>	 8/25/94	ATM		Changed some log messages.
		<25>	 8/23/94	DJ		state->disallowGameConnect
		<24>	 8/21/94	DJ		added state->personificationFlagsEditedByServer
		<23>	 8/19/94	BET		serverState->gameResult is a pointer now, fix up
									ServerState_Print to do the right thing.
		<22>	 8/19/94	ATM		Fixed game result printing.
		<21>	 8/17/94	ATM		Added logging of game results.
		<20>	 8/13/94	ATM		Bug.
		<19>	 8/13/94	ATM		Added Statusmsg call for mail-only connects.
		<18>	 8/11/94	DJ		latest personifuckation
		<17>	 8/10/94	DJ		personifuckation
		<16>	  8/5/94	DJ		playeraccount stuff
		<15>	  8/4/94	DJ		no more SNDQElement
		<14>	 7/20/94	DJ		removed configstr from serverstate
		<13>	 7/18/94	DJ		added opponentMagicCookie
		<12>	 7/17/94	BET		Make printf's go to gLogFile instead of stdout, which is the
									modem.
		<11>	 7/14/94	DJ		new mail
		<10>	 7/12/94	DJ		supports mail-only connections
		 <9>	  7/6/94	DJ		address book validation
		 <8>	 6/30/94	DJ		new sendQ format
		 <7>	 6/29/94	BET		(Really DJ) Clean up after KON.
		 <6>	 6/15/94	DJ		boxSerialNumber tweaks
		 <5>	 6/10/94	DJ		supporting configString better
		 <4>	  6/9/94	BET		Fix hairy compiles based on TransportSlayer.h
		 <3>	  6/4/94	DJ		setting the sessionrec in ServerState_Init
		 <2>	 5/27/94	DJ		updated to userIdentity struct
	To Do:
*/

#include "ServerCore.h"
#include "Server.h"
#include "ServerState.h"
#include "Messages.h"
#include "UsrConfg.h"
#include <stdio.h>
#include <stdlib.h>
#include "Utils.h"
#include "ServerDataBase.h"
#include "Server_Personification.h"

extern	FILE				*gLogFile;

void ServerState_Init(ServerState *state, SessionRec *session, char *config)
{
register long a;
register char *p;

	// clear every field of ServerState
	//
	for(p = (char *)state, a = 0; a < sizeof(ServerState); a++)
		*p++ = 0;


	state->validFlags = kServerValidFlag_None;
	
	state->session = session;
	if(session)
		state->validFlags |= kServerValidFlag_SessionRec;
	
	if(config)
	{
		state->validFlags |= kServerValidFlag_ConfigString;
		ByteCopy((Ptr)state->configBuffer, (Ptr)config, kConfigStrLength);
	} else
		state->configBuffer[0] = 0;
	
	state->account = NULL;
	
	state->personificationFlags = 0;
	state->personificationFlagsEditedByServer = 0;
	
	state->disallowGameConnect = false;

}

void ServerState_Empty(ServerState *state)
{
	Logmsg("ServerState_Empty\n");

	if(state->validFlags & kServerValidFlag_Account)
	{
		ASSERT(state->account);
		
		DataBase_FreeAccount(state->account);
		state->account = NULL;
	}
	
	if(	state->validFlags & kServerValidFlag_SendQ)
	{
		long i;
		
		for(i = 0; i < state->sendQData.count; i++){
			Logmsg("	Item %ld:\n", i);
			Logmsg("		DBID = %ld, Size = %ld\n",
							(long)state->sendQData.items[i].theID,
							state->sendQData.items[i].size);
			free(state->sendQData.items[i].data);
		}
		free(state->sendQData.items);
		state->sendQData.items = NULL;
	}
	
	if(state->validFlags & kServerValidFlag_AddrValidation)
	{
		// BRAIN DAMAGE????
	}
	
	if(state->validFlags & kServerValidFlag_IncomingMail)
	{
		short i;
		
		for(i = 0; i < state->incomingMail.count; i++)
			if(state->incomingMail.mailItems[i].mail)
				free(state->incomingMail.mailItems[i].mail);
		
		if(state->incomingMail.mailItems)
			free(state->incomingMail.mailItems);
		state->incomingMail.mailItems = NULL;
		state->incomingMail.count = 0;
	}
	
	if(state->validFlags & kServerValidFlag_GameResults)
	{
		free((Ptr)state->gameResult);
	}
		
	state->validFlags = kServerValidFlag_None;
}


void ServerState_Print(ServerState *state)
{
	if(state->validFlags & kServerValidFlag_Login)
	{
		Logmsg("Login Data:\n");
		ServerState_PrintUserIdentification(&state->loginData.userID);
		Logmsg("	Phone number = %s\n", state->boxPhoneNumber.phoneNumber);
	} else
		Logmsg("Login Data is invalid\n");
	
	if(state->validFlags & kServerValidFlag_ChallengeOrCompete)
	{
		if(state->challengeData.userID.box.box == -3) {
			Logmsg("Mail only connection\n");
			//Statusmsg("SunSega: Mail-only connect from '%s' (%s)\n",
			//	state->loginData.userID.userName,
			//	state->boxPhoneNumber.phoneNumber);
			//StatusPrintGameResults(state);
		} else if(state->challengeData.userID.box.box == -2)
			Logmsg("Normal competitive game request\n");
		else
		{
			Logmsg("Challenge Request:\n");
			ServerState_PrintUserIdentification(&state->challengeData.userID);
		}
	} else
		Logmsg("ChallengeOrCompete is invalid\n");
	
	if(state->validFlags & kServerValidFlag_SystemVersion)
	{
		Logmsg("System Version:\n");
		Logmsg("	length = %d, version = %ld\n", state->systemVersionData.length, state->systemVersionData.version);
	} else
		Logmsg("System Version is invalid\n");

	if(state->validFlags & kServerValidFlag_NGPVersion)
	{
		Logmsg("NGP Version = %ld\n", state->NGPVersion);
	} else
		Logmsg("NGP Version is invalid\n");
	
	if(state->validFlags & kServerValidFlag_GameID)
	{
		Logmsg("Game ID:\n");
		Logmsg("	gameID = 0x%.8lx, version = %ld\n", state->gameIDData.gameID, state->gameIDData.version);
	} else
		Logmsg("Game ID is invalid\n");

	if(	state->validFlags & kServerValidFlag_SendQ)
	{
		long i;
		
		Logmsg("SendQ Count = %d:\n", state->sendQData.count);
		for(i = 0; i < state->sendQData.count; i++){
			Logmsg("	Item %ld:\n", i);
			Logmsg("		Type = %ld, Size = %ld\n",
							(long)state->sendQData.items[i].theID,
							state->sendQData.items[i].size);
		}
	} else 
		Logmsg("SendQ is invalid\n");

	if(state->validFlags & kServerValidFlag_IncomingMail)
	{		
		Logmsg("Incoming Mail count = %ld\n", (long)state->incomingMail.count);
	}

}


void ServerState_PrintUserIdentification(const userIdentification *userID)
{
	Logmsg("  User identification:\n");
		Logmsg("	Box serial number = (%ld,%ld) [%ld]\n",
			userID->box.box, userID->box.region, (long)userID->userID);
		Logmsg("	User name = '%s'\n", userID->userName);
		Logmsg("    User town = '%s'\n", userID->userTown);
		Logmsg("    iconID = %ld, colorTableID = %ld\n",
			(long) userID->ROMIconID, (long)userID->colorTableID);
}

