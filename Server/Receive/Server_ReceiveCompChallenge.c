
/*
	File:		Server_ReceiveCompetitiveChallenge.c

	Contains:	Server receive competitive or challenge request

	Written by:	Dave Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<11>	 8/12/94	ATM		Convert to Logmsg.
		<10>	  8/3/94	DJ		changed challenge transmit to send much less data
		 <9>	 7/20/94	DJ		added Server_Comm stuff
		 <8>	 7/17/94	BET		Make printf's go to gLogFile instead of stdout, which is the
									modem.
		 <7>	 7/12/94	DJ		supporting new challenge/mail-only connection format
		 <6>	  7/1/94	DJ		making server handle errors from the comm layer
		 <5>	  6/9/94	BET		Fix hairy compiles based on TransportSlayer.h
		 <4>	  6/4/94	DJ		making everything take a ServerState instead of SessionRec
		 <3>	  6/1/94	DJ		printf tweaks
		 <2>	 5/27/94	DJ		updated to userIdentifier struct
		 <1>	 5/25/94	DJ		first checked in
	To Do:
*/

#include "ServerCore.h"
#include "Server.h"
#include "Messages.h"
#include "UsrConfg.h"
#include "Server_Comm.h"
#include <stdio.h>

#include "Challnge.h"


//
// Receives either a challenge to a specific player, or a request to play a competitive game
// (which selects players by rank)
//
int Server_ReceiveCompetitiveChallenge(ServerState *state)
{
unsigned char 	opCode, length;

	Logmsg("Server_ReceiveCompetitiveChallenge\n");

	if(Server_TReadDataSync( state->session, 1, (Ptr)&opCode ) != noErr)
		return(kServerFuncAbort);

	if(opCode != msChallengeRequest)
	{
		Logmsg("Wrong opCode.  Expected %d, got %d\n", msChallengeRequest, opCode);
		return(kServerFuncAbort);
	}

	Server_TReadDataSync( state->session, sizeof(unsigned char), (Ptr)&state->challengeData.challengeType);

	switch(state->challengeData.challengeType)
	{
		case kChallengeTypeChallengeByHandle:			// specific challenge & -1 boxsernum, so receive the handle
				Server_TReadDataSync( state->session,  sizeof(unsigned char), (Ptr)&length );
				Server_TReadDataSync( state->session,  (long)length, (Ptr)state->challengeData.userID.userName);
				state->challengeData.userID.box.box = -1;
				state->challengeData.userID.box.region = -1;
				Logmsg("Play against challenge user.  Name = %s\n", state->challengeData.userID.userName);
			break;

		case kChallengeTypeAutomatch:					// automatch
				state->challengeData.userID.box.box = kFindNetworkOpponentSerialNumber;
				state->challengeData.userID.box.region = -1;
				Logmsg("Automatch connection\n");
			break;

		case kChallengeTypeMailOnly:					// mail only
				state->challengeData.userID.box.box = kDownloadOnlyMailSerialNumber;
				state->challengeData.userID.box.region = -1;
				Logmsg("Mail only connection\n");
			break;

		case kChallengeTypeChallengeByBoxSerialNumber:	// specific challenge by box serial number
				Server_TReadDataSync( state->session,  sizeof(BoxSerialNumber), (Ptr)&state->challengeData.userID.box );
				Server_TReadDataSync( state->session,  sizeof(unsigned char), (Ptr)&state->challengeData.userID.userID );
				state->challengeData.userID.userName[0] = 0;
				Logmsg("Play against challenge user.  BoxSerNum = %ld  Region = %ld\n", state->challengeData.userID.box.box, state->challengeData.userID.box.region);
			break;

		default:
				Logmsg("Unknown challengeType %ld\n", (long)state->challengeData.challengeType);
				return(kServerFuncAbort);
			break;
	}

	state->validFlags |= kServerValidFlag_ChallengeOrCompete;

	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	Logmsg("Challenge request done\n");

	return(kServerFuncOK);
}

