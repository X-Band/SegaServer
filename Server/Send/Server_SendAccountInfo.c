/*
	File:		Server_SendAccountInfo.c

	Contains:	xxx put contents here xxx

	Written by:	Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<10>	 9/16/94	DJ		xband free credits now == 32 not 35
		 <9>	 9/16/94	ATM		Send an appropriate string based on value of long distance flag.
		 <8>	 8/22/94	DJ		groomed dialog text
		 <7>	 8/20/94	DJ		added a logmsg
		 <6>	 8/18/94	DJ		sending real account info
		 <5>	 8/12/94	ATM		Converted to Logmsg.
		 <4>	 8/11/94	DJ		cleaned up restrictions for user test
		 <3>	 7/25/94	DJ		boggles
		 <2>	 7/20/94	DJ		added Server_Comm stuff
		 <1>	 7/17/94	DJ		first checked in

	To Do:
*/


#include "ServerCore.h"
#include "Server.h"
#include "ServerDataBase.h"

#include "Messages.h"
#include "Server_Comm.h"
#include "Options.h"

#include <string.h>
#include <stdio.h>


int Server_SendAccountInfo(ServerState *state)
{
int			result;

	result = Server_SendRestrictions(state);
	if(result != kServerFuncOK)
		return(result);
		
	result = Server_SendCreditInfo(state);
	return(result);
}



int Server_SendRestrictions(ServerState *state)
{
messOut				opCode;
RestrictionsRecord	restr;

	Logmsg("Server_SendRestrictions\n");
	ASSERT(state->validFlags & kServerValidFlag_Account);

	opCode = msReceiveRestrictions;
	Server_TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);

	if (state->account->boxAccount.boxFlags & kBoxFlags_dialLongDistance)
		strcpy(restr.playField, "Long Distance");
	else
		strcpy(restr.playField, "Local Only");
	strcpy(restr.hours1, "Su - Th anytime");
	strcpy(restr.hours2, "Fr - Sa anytime");

	Server_TWriteDataSync(state->session, sizeof(RestrictionsRecord), (Ptr)&restr);

	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	Logmsg("Server_SendRestrictions done\n");

	return(kServerFuncOK);
}


int Server_SendCreditInfo(ServerState *state)
{
messOut				opCode;
CreditRecord		credits;

	Logmsg("Server_SendCreditInfo\n");
	ASSERT(state->validFlags & kServerValidFlag_Account);

	opCode = msReceiveCredit;
	Server_TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);

	credits.accountCredits = state->account->boxAccount.freeCredits;
	credits.usedCredits = kFreeXBandCredits - state->account->boxAccount.freeCredits;	// BRAIN DAMAGE.  This calc won't work for real accounting.

	Logmsg("Server_SendCreditInfo: accountCredits = %ld, usedCredits = %ld\n", credits.accountCredits, credits.usedCredits);

	Server_TWriteDataSync(state->session, sizeof(CreditRecord), (Ptr)&credits);

	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	Logmsg("Server_SendCreditInfo done\n");

	return(kServerFuncOK);
}
