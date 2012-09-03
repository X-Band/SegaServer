
/*
	File:		Server_SendOpponent.c

	Contains:	Server send opponent functions

	Written by:	Dave Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<22>	 9/16/94	ATM		Make the thank-you strings slightly less silly.
		<21>	  9/2/94	DJ		changed a dialog
		<20>	 8/29/94	ATM		Send a variety of thank-you strings.
		<19>	 8/22/94	DJ		groomed dialog text
		<18>	 8/22/94	DJ		random worth strings
		<17>	 8/22/94	DJ		got burned by Octal when setting the first char of the worth
									strings to 0x3.
		<16>	 8/21/94	DJ		fontid is 3 for worthstrings
		<15>	 8/20/94	BET		Add Server_SendOpponentNameString
		<14>	 8/19/94	DJ		added Server_SendWorthString
		<13>	 8/17/94	DJ		moved routines between files
		<12>	 8/12/94	ATM		Converted to Logmsg.
		<11>	 7/20/94	DJ		added Server_Comm stuff
		<10>	 7/18/94	DJ		magicCookie (ie. opponentVerificationTag)
		 <9>	 7/17/94	BET		Make printf's go to gLogFile instead of stdout, which is the
									modem.
		 <8>	 7/12/94	DJ		using Server_TCheckError instead of TCheckError
		 <7>	  7/8/94	DJ		send sizeof(phoneNumber)
		 <6>	  7/1/94	DJ		making server handle errors from the comm layer
		 <5>	  6/9/94	BET		Fix hairy compiles based on TransportSlayer.h
		 <4>	  6/5/94	DJ		making everything take a ServerState instead of SessionRec and
									added Server_DebugService
		 <3>	 5/29/94	DJ		sync writing instead of async
		 <2>	 5/27/94	DJ		made it send messages to box
		 <1>	 5/25/94	DJ		first checked in
	To Do:
*/

#include "ServerCore.h"
#include "Server.h"
#include "Messages.h"
#include "Server_Comm.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>

#include "StringDB.h"


static char 	*worthStrings[] = {	"\0031\nPoint",
								"\00310\nPoints", 
								"\003100\nPoints", 
								"\003Quarter\nFinals", 
								"\003Semi\nFinals", 
								"\003Tourney",
								"\003Flame\nFest",
								0};

static char		*endGameStrings[] = {
					"Thank you for playing XBAND!",
					"Y'all come back now, hear?",
					"Having fun yet?  Play another round!",
				};
#define NELEM(x)	(sizeof(x) / sizeof(x[0]))


//
// This gets called for either master or slave connects.  It sends the "worth" string, which is
// how many points this game is worth to your opponent.  These are exchanged between the peers
// once they connect peer-to-peer.  The worth string is based on your rank for this game.
// It can also be
//
int Server_SendWorthString(ServerState *state)
{
unsigned char 	opCode;
long			length;
DBID			theID;
char			*worth, *endstr;
long			worthIndex;
long			randNum;

	Logmsg("Server_SendWorthString\n");
	if(Server_DebugService(state) != kServerFuncOK)
		return(kServerFuncAbort);


	//
	// Your worth is based on your ranking for the current game.
	//
	for(worthIndex = 0; worthStrings[worthIndex]; worthIndex++)	// compute number of worthStrings.
		;

	srandom(time(NULL));
	randNum = random();
	randNum %= worthIndex;
	worth = worthStrings[randNum];

	opCode = msReceiveWriteableString;
	if(Server_TWriteDataSync(state->session, sizeof(messOut), (Ptr) &opCode) != noErr)
		return(kServerFuncAbort);

	theID = kLocalGameValueString;
	if(Server_TWriteDataSync(state->session, sizeof(DBID), (Ptr) &theID) != noErr)
		return(kServerFuncAbort);

	length = strlen( worth ) + 1;
	if(Server_TWriteDataSync(state->session, sizeof(long), (Ptr) &length) != noErr)
		return(kServerFuncAbort);

	if(Server_TWriteDataSync(state->session, length, (Ptr) worth) != noErr)
		return(kServerFuncAbort);

	Logmsg("Server_SendWorthString done\n");


	//
	// For fun, change the end-ranking string.
	//
	randNum = random() % NELEM(endGameStrings);
	endstr = endGameStrings[randNum];
	Logmsg("Sending end game string '%s'\n", endstr);
	opCode = msReceiveWriteableString;

	if(Server_TWriteDataSync(state->session, sizeof(messOut), (Ptr) &opCode) != noErr)
		return(kServerFuncAbort);

	theID = kThankYouStringID;
	if(Server_TWriteDataSync(state->session, sizeof(DBID), (Ptr) &theID) != noErr)
		return(kServerFuncAbort);

	length = strlen( endstr ) + 1;
	if(Server_TWriteDataSync(state->session, sizeof(long), (Ptr) &length) != noErr)
		return(kServerFuncAbort);

	if(Server_TWriteDataSync(state->session, length, (Ptr) endstr) != noErr)
		return(kServerFuncAbort);

	return(kServerFuncOK);
}


int Server_SendOpponentNameString(ServerState *state, char *opponentName)
{
unsigned char 	opCode;
long			length;
DBID			theID;
char			*worth;

	Logmsg("Server_SendOpponentNameString\n");
	if(Server_DebugService(state) != kServerFuncOK)
		return(kServerFuncAbort);


	//
	// Your worth is based on your ranking for the current game.
	//
	opCode = msReceiveWriteableString;
	if(Server_TWriteDataSync(state->session, sizeof(messOut), (Ptr) &opCode) != noErr)
		return(kServerFuncAbort);

	theID = kOpponentName;
	if(Server_TWriteDataSync(state->session, sizeof(DBID), (Ptr) &theID) != noErr)
		return(kServerFuncAbort);

	Logmsg("Server_SendOpponentNameString: name is %s\n", opponentName);
	length = strlen( opponentName ) + 1;
	if(Server_TWriteDataSync(state->session, sizeof(long), (Ptr) &length) != noErr)
		return(kServerFuncAbort);

	if(Server_TWriteDataSync(state->session, length, (Ptr) opponentName) != noErr)
		return(kServerFuncAbort);

	Logmsg("Server_SendOpponentNameString done\n");

	return(kServerFuncOK);
}



//
// This is called before Server_SendRegisterPlayer or Server_SendRegisterChallengePlayer.
// This message sets the opponent verification cookie.
//
int Server_SendWaitForOpponent(ServerState *state, long magicCookie)
{
unsigned char 	opCode;
int				err;

	Logmsg("Server_SendWaitForOpponent\n");
	if(Server_DebugService(state) != kServerFuncOK)
		return(kServerFuncAbort);

	opCode = msWaitForOpponent;
	if(Server_TWriteDataSync(state->session, 1, (Ptr)&opCode) != noErr)
		return(kServerFuncAbort);
	if(Server_TWriteDataSync(state->session, sizeof(long), (Ptr)&magicCookie) != noErr)
		return(kServerFuncAbort);


	//
	// Send what this game is worth.
	//
	err = Server_SendWorthString(state);


	Logmsg("Server_SendWaitForOpponent done\n");

	return(err);
}


//
// This routine makes the box do a NetRegister with the timeout value.
//
int Server_SendRegisterPlayer(ServerState *state, long timeoutValue, char *gameName)
{
unsigned char 	opCode;
int				err;
char			boggle[1000];

	Logmsg("Server_SendRegisterPlayer\n");

	if(Server_DebugService(state) != kServerFuncOK)
		return(kServerFuncAbort);

	opCode = msRegisterPlayer;
	if(Server_TWriteDataSync(state->session, 1, (Ptr)&opCode) != noErr)
		return(kServerFuncAbort);

	if(Server_TWriteDataSync(state->session, sizeof(long), (Ptr)&timeoutValue) != noErr)
		return(kServerFuncAbort);

	// BRAIN DAMAGE.  this should tell you how long the wait is.  Is set to 10 mins now.
	//
	sprintf(boggle, "We are searching for a worthy %s opponent for you.  We should find one within 10 minutes... please twiddle your thumbs while you wait.",
		gameName[0] ? gameName : "this game");

	err = Server_SendDialog(state, boggle, true);

	Logmsg("Server_SendRegisterPlayer done\n");

	return(err);
}

//
// This routine makes the box do a NetRegister with the timeout value.
//
int Server_SendRegisterChallengePlayer(ServerState *state, long timeoutValue, char *gameName, char *challengeName)
{
unsigned char 	opCode;
int				err;
char			boggle[1000];

	Logmsg("Server_SendRegisterChallengePlayer\n");

	if(Server_DebugService(state) != kServerFuncOK)
		return(kServerFuncAbort);

	opCode = msRegisterPlayer;
	if(Server_TWriteDataSync(state->session, 1, (Ptr)&opCode) != noErr)
		return(kServerFuncAbort);

	if(Server_TWriteDataSync(state->session, sizeof(long), (Ptr)&timeoutValue) != noErr)
		return(kServerFuncAbort);

	sprintf(boggle, "You are registered to play %s in a game of %s.  Hang out until %s shows up.",
		challengeName,
		gameName[0] ? gameName : "this game",
		challengeName);

	err = Server_SendDialog(state, boggle, true);

	Logmsg("Server_SendRegisterChallengePlayer done\n");

	return(err);
}


//
// On the box, in messages.c, the receiving routine calls NetRegister.
//
int Server_SendOpponentPhoneNumber(ServerState *state, const phoneNumber *opponentPhoneNumber, long magicCookie)
{
unsigned char 	opCode;
int				err;

	Logmsg("Server_SendOpponentPhoneNumber\n");
	if(Server_DebugService(state) != kServerFuncOK)
		return(kServerFuncAbort);

	opCode = msOpponentPhoneNumber;
	Server_TWriteDataSync(state->session, 1, (Ptr)&opCode);
	Server_TWriteDataSync( state->session, sizeof(phoneNumber), (Ptr)opponentPhoneNumber );
	Server_TWriteDataSync( state->session, sizeof(long), (Ptr)&magicCookie );

	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	//
	// Send what this game is worth.
	//
	err = Server_SendWorthString(state);

	Logmsg("Server_SendOpponentPhoneNumber done\n");

	return(err);

}

