
/*
	File:		Server_ReceiveGameID.c

	Contains:	Server get gameID function

	Written by:	Dave Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<22>	 9/16/94	ATM		Made net errors log to LOG_NETERR.
		<21>	  9/3/94	ATM		Spiffed up the game results a bit.
		<20>	  9/1/94	ATM		Send game result stuff to LOG_GAMERESULT.
		<19>	 8/25/94	BET		Add support for multiple NetErrorRecord types for both X25 and
									800 services.
		<18>	 8/24/94	BET		Add packetRetransError field to log.
		<17>	 8/24/94	ATM		Moved stuff into DumpGameResults(), and now dump game error
									results as well.
		<16>	 8/23/94	ATM		Splat.
		<15>	 8/23/94	ATM		Added debugging stuff for receiving game results.
		<14>	 8/19/94	BET		Fix Server_ReceiveGameResults to recieve the gameResult based on
									the size field.
		<13>	 8/18/94	BET		Add BoxNet dump.
		<12>	 8/17/94	ATM		Added NBA Jam rev 2 to internal list.
		<11>	 8/13/94	BET		Add Server_RecieveNetErrors
		<10>	 8/12/94	ATM		Converted to Logmsg.
		 <9>	  8/4/94	DJ		Server_ReceiveGameErrorResults
		 <8>	 7/20/94	DJ		added Server_Comm stuff
		 <7>	 7/17/94	BET		Make printf's go to gLogFile instead of stdout, which is the
									modem.
		 <6>	 7/12/94	DJ		using Server_TCheckError instead of TCheckError
		 <5>	  7/1/94	DJ		making server handle errors from the comm layer
		 <4>	 6/11/94	DJ		printf of which game is being played
		 <3>	  6/9/94	BET		Fix hairy compiles based on TransportSlayer.h
		 <2>	  6/4/94	DJ		making everything take a ServerState instead of SessionRec
	To Do:
*/

#include "ServerCore.h"
#include "Server.h"
#include "Messages.h"
#include "Server_Comm.h"
#include <stdio.h>
#include "GameDB.h"


void DumpGameResults(ServerState *state, GameResult *gameResults, int isErr);


int Server_ReceiveGameID(ServerState *state)
{
unsigned char opCode;

	Logmsg("Server_ReceiveGameID\n");

	Server_TReadDataSync( state->session, 1, (Ptr)&opCode );
	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	if(opCode != msGameIDAndPatchVersion){
		// fucked
		Logmsg("Wrong opCode.  Expected %d, got %d\n", msGameIDAndPatchVersion, opCode);
		return(kServerFuncAbort);
	}

	Server_TReadDataSync( state->session, sizeof(long), (Ptr)&state->gameIDData.gameID );
	Server_TReadDataSync( state->session, sizeof(long), (Ptr)&state->gameIDData.version );

	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	state->validFlags |= kServerValidFlag_GameID;

	Logmsg("gameID = %ld, version = %ld\n", state->gameIDData.gameID, state->gameIDData.version);
	switch(state->gameIDData.gameID)
	{
		case kMortalKombatGameID:	Logmsg("** Game is Mortal Kombat\n");
			break;
		case kNBAJamGameID:			Logmsg("** Game is Old NBA Jam\n");
			break;
		case 0x39677bdb:			Logmsg("** Game is NBA Jam Rev2\n");
			break;
		default:					Logmsg("** Unknown gameID\n");
			break;
	}
	
	Logmsg("Server_ReceiveGameID done\n");
	return(kServerFuncOK);
}



int Server_ReceiveGameResults(ServerState *state)
{
unsigned char	opCode;
unsigned long	size;
GameResult		*gameResults;

	Logmsg("Server_ReceiveGameResults\n");
	state->gameResult = nil;
	
	Server_TReadDataSync( state->session, 1, (Ptr)&opCode );
	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	if(opCode == msSendNoGameResults)
	{
		Logmsg("No Game Results to receive\n");
		return(kServerFuncOK);
	}

	if(opCode != msSendGameResults){
		// fucked
		Logmsg("Wrong opCode.  Expected %ld or %ld, got %ld\n", 
							(long) msSendGameResults, 
							(long) msSendNoGameResults, 
							(long) opCode);
		return(kServerFuncAbort);
	}
	
	// how big is it?
	Server_TReadDataSync( state->session, sizeof(size), (Ptr)&size );
	gameResults = (GameResult *)malloc(size);
	
	if (!gameResults)
		return(kServerFuncAbort);
	
	gameResults->size = size;
	Server_TReadDataSync( state->session, size-sizeof(gameResults->size), (Ptr)&gameResults->gameID );
	
	state->gameResult = gameResults;
	state->validFlags |= kServerValidFlag_GameResults;

	DumpGameResults(state, gameResults, 0);

	Logmsg("Server_ReceiveGameResults done\n");
	return(kServerFuncOK);
}

int Server_ReceiveGameErrorResults(ServerState *state)
{
unsigned char opCode;

	Logmsg("Server_ReceiveGameErrorResults\n");

	Server_TReadDataSync( state->session, 1, (Ptr)&opCode );
	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	if(opCode == msSendNoGameErrorResults )
	{
		Logmsg("No Game Error Results to receive\n");
		return(kServerFuncOK);
	}

	if(opCode != msSendGameErrorResults){
		// fucked
		Logmsg("Wrong opCode.  Expected %ld or %ld, got %ld\n", 
							(long) msSendGameErrorResults, 
							(long) msSendNoGameErrorResults, 
							(long) opCode);
		return(kServerFuncAbort);
	}
	
	Server_TReadDataSync( state->session, sizeof(GameResult), (Ptr)&state->gameErrorResult );

	state->validFlags |= kServerValidFlag_GameErrorResults;

	DumpGameResults(state, &state->gameErrorResult, 1);

	Logmsg("Server_ReceiveGameErrorResults done\n");
	return(kServerFuncOK);
}

int Server_ReceiveNetErrorsOld(ServerState *state)
{
unsigned char opCode;

	Logmsg("Server_ReceiveNetErrors\n");
	Logmsg("pre-Version 7 box, stuffing net errors into boxNetErrors800\n");

	Server_TReadDataSync( state->session, 1, (Ptr)&opCode );
	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	if(opCode == msNoNetErrors )
	{
		Logmsg("No Net Errors to receive\n");
		return(kServerFuncOK);
	}

	if(opCode != msSendNetErrors){
		// fucked
		Logmsg("Wrong opCode.  Expected %ld or %ld, got %ld\n", 
							(long) msSendNetErrors, 
							(long) msNoNetErrors, 
							(long) opCode);
		return(kServerFuncAbort);
	}
	
	Server_TReadDataSync( state->session, sizeof(NetErrorRecord) - 4, (Ptr)&state->boxNetErrors800 );
	FLogmsg(LOG_NETERR, "	Box net: serverConnects = %d\n", state->boxNetErrors800.serverConnects);
	FLogmsg(LOG_NETERR, "	Box net: peerConnects = %d\n", state->boxNetErrors800.peerConnects);
	FLogmsg(LOG_NETERR, "	Box net: framingError = %d\n", state->boxNetErrors800.framingError);
	FLogmsg(LOG_NETERR, "	Box net: overrunError = %d\n", state->boxNetErrors800.overrunError);
	FLogmsg(LOG_NETERR, "	Box net: packetError = %d\n", state->boxNetErrors800.packetError);
	FLogmsg(LOG_NETERR, "	Box net: callWaitingInterrupt = %d\n", state->boxNetErrors800.callWaitingInterrupt);
	FLogmsg(LOG_NETERR, "	Box net: noDialtoneError = %d\n", state->boxNetErrors800.noDialtoneError);
	FLogmsg(LOG_NETERR, "	Box net: serverBusyError = %d\n", state->boxNetErrors800.serverBusyError);
	FLogmsg(LOG_NETERR, "	Box net: peerBusyError = %d\n", state->boxNetErrors800.peerBusyError);
	FLogmsg(LOG_NETERR, "	Box net: serverDisconnectError = %d\n", state->boxNetErrors800.serverDisconnectError);
	FLogmsg(LOG_NETERR, "	Box net: peerDisconnectError = %d\n", state->boxNetErrors800.peerDisconnectError);
	FLogmsg(LOG_NETERR, "	Box net: serverAbortError = %d\n", state->boxNetErrors800.serverAbortError);
	FLogmsg(LOG_NETERR, "	Box net: peerAbortError = %d\n", state->boxNetErrors800.peerAbortError);
	FLogmsg(LOG_NETERR, "	Box net: serverNoAnswerError = %d\n", state->boxNetErrors800.serverNoAnswerError);
	FLogmsg(LOG_NETERR, "	Box net: peerNoAnswerError = %d\n", state->boxNetErrors800.peerNoAnswerError);
	FLogmsg(LOG_NETERR, "	Box net: serverHandshakeError = %d\n", state->boxNetErrors800.serverHandshakeError);
	FLogmsg(LOG_NETERR, "	Box net: peerHandshakeError = %d\n", state->boxNetErrors800.peerHandshakeError);
	FLogmsg(LOG_NETERR, "	Box net: serverX25NoServiceError = %d\n", state->boxNetErrors800.serverX25NoServiceError);
	FLogmsg(LOG_NETERR, "	Box net: callWaitingError = %d\n", state->boxNetErrors800.callWaitingError);
	FLogmsg(LOG_NETERR, "	Box net: remoteCallWaitingError = %d\n", state->boxNetErrors800.remoteCallWaitingError);
	FLogmsg(LOG_NETERR, "	Box net: scriptLoginError = %d\n", state->boxNetErrors800.scriptLoginError);
	FLogmsg(LOG_NETERR, "	Box net: < V7 box has no retrans info\n");

	state->validFlags |= kServerValidFlag_NetErrors800;

	Logmsg("Server_ReceiveNetErrors done\n");
	return(kServerFuncOK);
}

int Server_ReceiveNetErrorsCombined(ServerState *state)
{
unsigned char opCode;

	Logmsg("Server_ReceiveNetErrorsCombined\n");
	Logmsg("Version 7 box, stuffing net errors into boxNetErrors800\n");

	Server_TReadDataSync( state->session, 1, (Ptr)&opCode );
	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	if(opCode == msNoNetErrors )
	{
		Logmsg("No Net Errors to receive\n");
		return(kServerFuncOK);
	}

	if(opCode != msSendNetErrors){
		// fucked
		Logmsg("Wrong opCode.  Expected %ld or %ld, got %ld\n", 
							(long) msSendNetErrors, 
							(long) msNoNetErrors, 
							(long) opCode);
		return(kServerFuncAbort);
	}
	
	Server_TReadDataSync( state->session, sizeof(NetErrorRecord), (Ptr)&state->boxNetErrors800 );
	FLogmsg(LOG_NETERR, "	Box net (combined): serverConnects = %d\n", state->boxNetErrors800.serverConnects);
	FLogmsg(LOG_NETERR, "	Box net (combined): peerConnects = %d\n", state->boxNetErrors800.peerConnects);
	FLogmsg(LOG_NETERR, "	Box net (combined): framingError = %d\n", state->boxNetErrors800.framingError);
	FLogmsg(LOG_NETERR, "	Box net (combined): overrunError = %d\n", state->boxNetErrors800.overrunError);
	FLogmsg(LOG_NETERR, "	Box net (combined): packetError = %d\n", state->boxNetErrors800.packetError);
	FLogmsg(LOG_NETERR, "	Box net (combined): callWaitingInterrupt = %d\n", state->boxNetErrors800.callWaitingInterrupt);
	FLogmsg(LOG_NETERR, "	Box net (combined): noDialtoneError = %d\n", state->boxNetErrors800.noDialtoneError);
	FLogmsg(LOG_NETERR, "	Box net (combined): serverBusyError = %d\n", state->boxNetErrors800.serverBusyError);
	FLogmsg(LOG_NETERR, "	Box net (combined): peerBusyError = %d\n", state->boxNetErrors800.peerBusyError);
	FLogmsg(LOG_NETERR, "	Box net (combined): serverDisconnectError = %d\n", state->boxNetErrors800.serverDisconnectError);
	FLogmsg(LOG_NETERR, "	Box net (combined): peerDisconnectError = %d\n", state->boxNetErrors800.peerDisconnectError);
	FLogmsg(LOG_NETERR, "	Box net (combined): serverAbortError = %d\n", state->boxNetErrors800.serverAbortError);
	FLogmsg(LOG_NETERR, "	Box net (combined): peerAbortError = %d\n", state->boxNetErrors800.peerAbortError);
	FLogmsg(LOG_NETERR, "	Box net (combined): serverNoAnswerError = %d\n", state->boxNetErrors800.serverNoAnswerError);
	FLogmsg(LOG_NETERR, "	Box net (combined): peerNoAnswerError = %d\n", state->boxNetErrors800.peerNoAnswerError);
	FLogmsg(LOG_NETERR, "	Box net (combined): serverHandshakeError = %d\n", state->boxNetErrors800.serverHandshakeError);
	FLogmsg(LOG_NETERR, "	Box net (combined): peerHandshakeError = %d\n", state->boxNetErrors800.peerHandshakeError);
	FLogmsg(LOG_NETERR, "	Box net (combined): serverX25NoServiceError = %d\n", state->boxNetErrors800.serverX25NoServiceError);
	FLogmsg(LOG_NETERR, "	Box net (combined): callWaitingError = %d\n", state->boxNetErrors800.callWaitingError);
	FLogmsg(LOG_NETERR, "	Box net (combined): remoteCallWaitingError = %d\n", state->boxNetErrors800.remoteCallWaitingError);
	FLogmsg(LOG_NETERR, "	Box net (combined): scriptLoginError = %d\n", state->boxNetErrors800.scriptLoginError);
	FLogmsg(LOG_NETERR, "	Box net (combined): packetRetransError = %d\n", state->boxNetErrors800.packetRetransError);

	state->validFlags |= kServerValidFlag_NetErrors800;

	Logmsg("Server_ReceiveNetErrorsCombined done\n");
	return(kServerFuncOK);
}


int Server_ReceiveNetErrorsNew(ServerState *state)
{
unsigned char opCode;

	Logmsg("Server_ReceiveNetErrorsNew\n");

	Server_TReadDataSync( state->session, 1, (Ptr)&opCode );
	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	if(opCode == msNoNetErrors )
	{
		Logmsg("No Net Errors to receive\n");
	}
	else if(opCode != msSendNetErrors){
		// fucked
		Logmsg("Wrong opCode.  Expected %ld or %ld, got %ld\n", 
							(long) msSendNetErrors, 
							(long) msNoNetErrors, 
							(long) opCode);
		return(kServerFuncAbort);
	}
	else {
		Server_TReadDataSync( state->session, sizeof(NetErrorRecord), (Ptr)&state->boxNetErrors800 );
		FLogmsg(LOG_NETERR, "	Box net 800: serverConnects = %d\n", state->boxNetErrors800.serverConnects);
		FLogmsg(LOG_NETERR, "	Box net 800: peerConnects = %d\n", state->boxNetErrors800.peerConnects);
		FLogmsg(LOG_NETERR, "	Box net 800: framingError = %d\n", state->boxNetErrors800.framingError);
		FLogmsg(LOG_NETERR, "	Box net 800: overrunError = %d\n", state->boxNetErrors800.overrunError);
		FLogmsg(LOG_NETERR, "	Box net 800: packetError = %d\n", state->boxNetErrors800.packetError);
		FLogmsg(LOG_NETERR, "	Box net 800: callWaitingInterrupt = %d\n", state->boxNetErrors800.callWaitingInterrupt);
		FLogmsg(LOG_NETERR, "	Box net 800: noDialtoneError = %d\n", state->boxNetErrors800.noDialtoneError);
		FLogmsg(LOG_NETERR, "	Box net 800: serverBusyError = %d\n", state->boxNetErrors800.serverBusyError);
		FLogmsg(LOG_NETERR, "	Box net 800: peerBusyError = %d\n", state->boxNetErrors800.peerBusyError);
		FLogmsg(LOG_NETERR, "	Box net 800: serverDisconnectError = %d\n", state->boxNetErrors800.serverDisconnectError);
		FLogmsg(LOG_NETERR, "	Box net 800: peerDisconnectError = %d\n", state->boxNetErrors800.peerDisconnectError);
		FLogmsg(LOG_NETERR, "	Box net 800: serverAbortError = %d\n", state->boxNetErrors800.serverAbortError);
		FLogmsg(LOG_NETERR, "	Box net 800: peerAbortError = %d\n", state->boxNetErrors800.peerAbortError);
		FLogmsg(LOG_NETERR, "	Box net 800: serverNoAnswerError = %d\n", state->boxNetErrors800.serverNoAnswerError);
		FLogmsg(LOG_NETERR, "	Box net 800: peerNoAnswerError = %d\n", state->boxNetErrors800.peerNoAnswerError);
		FLogmsg(LOG_NETERR, "	Box net 800: serverHandshakeError = %d\n", state->boxNetErrors800.serverHandshakeError);
		FLogmsg(LOG_NETERR, "	Box net 800: peerHandshakeError = %d\n", state->boxNetErrors800.peerHandshakeError);
		FLogmsg(LOG_NETERR, "	Box net 800: serverX25NoServiceError = %d\n", state->boxNetErrors800.serverX25NoServiceError);
		FLogmsg(LOG_NETERR, "	Box net 800: callWaitingError = %d\n", state->boxNetErrors800.callWaitingError);
		FLogmsg(LOG_NETERR, "	Box net 800: remoteCallWaitingError = %d\n", state->boxNetErrors800.remoteCallWaitingError);
		FLogmsg(LOG_NETERR, "	Box net 800: scriptLoginError = %d\n", state->boxNetErrors800.scriptLoginError);
		FLogmsg(LOG_NETERR, "	Box net 800: packetRetransError = %d\n", state->boxNetErrors800.packetRetransError);
	
		state->validFlags |= kServerValidFlag_NetErrors800;
	}
	Server_TReadDataSync( state->session, 1, (Ptr)&opCode );
	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	if(opCode == msNoNetErrors )
	{
		Logmsg("No X25 Net Errors to receive\n");
	}
	else if (opCode != msSendNetErrors){
		// fucked
		Logmsg("Wrong opCode.  Expected %ld or %ld, got %ld\n", 
							(long) msSendNetErrors, 
							(long) msNoNetErrors, 
							(long) opCode);
		return(kServerFuncAbort);
	}
	else {
		Server_TReadDataSync( state->session, sizeof(NetErrorRecord), (Ptr)&state->boxNetErrorsX25 );
		FLogmsg(LOG_NETERR, "	Box net X25: serverConnects = %d\n", state->boxNetErrorsX25.serverConnects);
		FLogmsg(LOG_NETERR, "	Box net X25: peerConnects = %d\n", state->boxNetErrorsX25.peerConnects);
		FLogmsg(LOG_NETERR, "	Box net X25: framingError = %d\n", state->boxNetErrorsX25.framingError);
		FLogmsg(LOG_NETERR, "	Box net X25: overrunError = %d\n", state->boxNetErrorsX25.overrunError);
		FLogmsg(LOG_NETERR, "	Box net X25: packetError = %d\n", state->boxNetErrorsX25.packetError);
		FLogmsg(LOG_NETERR, "	Box net X25: callWaitingInterrupt = %d\n", state->boxNetErrorsX25.callWaitingInterrupt);
		FLogmsg(LOG_NETERR, "	Box net X25: noDialtoneError = %d\n", state->boxNetErrorsX25.noDialtoneError);
		FLogmsg(LOG_NETERR, "	Box net X25: serverBusyError = %d\n", state->boxNetErrorsX25.serverBusyError);
		FLogmsg(LOG_NETERR, "	Box net X25: peerBusyError = %d\n", state->boxNetErrorsX25.peerBusyError);
		FLogmsg(LOG_NETERR, "	Box net X25: serverDisconnectError = %d\n", state->boxNetErrorsX25.serverDisconnectError);
		FLogmsg(LOG_NETERR, "	Box net X25: peerDisconnectError = %d\n", state->boxNetErrorsX25.peerDisconnectError);
		FLogmsg(LOG_NETERR, "	Box net X25: serverAbortError = %d\n", state->boxNetErrorsX25.serverAbortError);
		FLogmsg(LOG_NETERR, "	Box net X25: peerAbortError = %d\n", state->boxNetErrorsX25.peerAbortError);
		FLogmsg(LOG_NETERR, "	Box net X25: serverNoAnswerError = %d\n", state->boxNetErrorsX25.serverNoAnswerError);
		FLogmsg(LOG_NETERR, "	Box net X25: peerNoAnswerError = %d\n", state->boxNetErrorsX25.peerNoAnswerError);
		FLogmsg(LOG_NETERR, "	Box net X25: serverHandshakeError = %d\n", state->boxNetErrorsX25.serverHandshakeError);
		FLogmsg(LOG_NETERR, "	Box net X25: peerHandshakeError = %d\n", state->boxNetErrorsX25.peerHandshakeError);
		FLogmsg(LOG_NETERR, "	Box net X25: serverX25NoServiceError = %d\n", state->boxNetErrorsX25.serverX25NoServiceError);
		FLogmsg(LOG_NETERR, "	Box net X25: callWaitingError = %d\n", state->boxNetErrorsX25.callWaitingError);
		FLogmsg(LOG_NETERR, "	Box net X25: remoteCallWaitingError = %d\n", state->boxNetErrorsX25.remoteCallWaitingError);
		FLogmsg(LOG_NETERR, "	Box net X25: scriptLoginError = %d\n", state->boxNetErrorsX25.scriptLoginError);
		FLogmsg(LOG_NETERR, "	Box net X25: packetRetransError = %d\n", state->boxNetErrorsX25.packetRetransError);
	
		state->validFlags |= kServerValidFlag_NetErrorsX25;
	}
	
	Logmsg("Server_ReceiveNetErrorsNew done\n");
	return(kServerFuncOK);
}


// Dump the game results into the log so we can debug them.
//
void DumpGameResults(ServerState *state, GameResult *gameResults, int isErr)
{
	long structSize, extraSize;

	structSize = ((unsigned char *)&gameResults->pad) -
		((unsigned char *)gameResults);

	if (state->validFlags & kServerValidFlag_Login &&
		state->validFlags & kServerValidFlag_BoxPhoneNumber)
	{
		FLogmsg(LOG_GAMERESULT, "%s Results for %s (%s):\n",
			isErr ? "Game Error" : "Game",
			state->loginData.userID.userName,
			state->boxPhoneNumber.phoneNumber);
	} else {
		FLogmsg(LOG_GAMERESULT, "%s Results:\n",
			isErr ? "Game Error" : "Game");
	}

	FLogmsg(LOG_GAMERESULT, " size=%ld  gameID=0x%.8lx  gameError=%ld  playTime=%ld\n",
		gameResults->size, gameResults->gameID, gameResults->gameError,
		gameResults->playTime);
	FLogmsg(LOG_GAMERESULT, " local1=%ld  local2=%ld  remote1=%ld  remote2=%ld\n",
		gameResults->localPlayer1Result, gameResults->localPlayer2Result,
		gameResults->remotePlayer1Result, gameResults->remotePlayer2Result);
	FLogmsg(LOG_GAMERESULT, " dbIDDataPtr=%ld  dbIDDataSize=%ld  numPlayers=%ld gameReserved:\n",
		gameResults->dbIDDataPtr, gameResults->dbIDDataSize,
		(long)gameResults->numLocalPlayers);
	FLoghexdump(LOG_GAMERESULT, (unsigned char *) &gameResults->gameReserved,
		sizeof(gameResults->gameReserved));
	extraSize = gameResults->size + sizeof(gameResults->size) - structSize;

	// do a sanity check so we don't go nuts
	if (extraSize < 0 || extraSize > 1024) {
		FLogmsg(LOG_GAMERESULT, " extraSize = %ld, wrong!\n");
		return;
	}
	if (extraSize) {
		FLogmsg(LOG_GAMERESULT, " Extra stuff (%ld bytes):\n", extraSize);
		FLoghexdump(LOG_GAMERESULT, (unsigned char *)&gameResults->pad, extraSize);
	} else {
		FLogmsg(LOG_GAMERESULT, " No extra stuff\n");
	}
}

