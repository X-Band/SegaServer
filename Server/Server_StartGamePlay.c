/*
	File:		Server_StartGamePlay.c

	Contains:	Asks the Matcher to find an opponent for the box.

	Written by:	David Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<49>	 9/18/94	ATM		Server_GameName becomes Common_GameName.
		<48>	 9/16/94	ATM		Added "9," hack based on debug1.
		<47>	 9/16/94	ATM		Send the "long distance okay" flag to the matcher.
		<46>	 9/12/94	ATM		Tweaked the log messages.
		<45>	 9/10/94	ATM		Tweaked a dialog to handle an unusual condition.
		<44>	  9/7/94	ATM		Moved the "Mail-only connect" statusmsg in here.
		<43>	  9/7/94	ATM		Add dialogs for some unusual cases.
		<42>	  9/6/94	ATM		Complete overhaul of matching system.
		<41>	 8/29/94	ATM		Don't send blank string as opponent phone number.  Nuked
									extractedPhoneNumber use.
		<40>	 8/25/94	ATM		Don't allow player to challenge someone on his own box.
									(Prohibited by address book, but let's be thorough.)
		<39>	 8/25/94	ATM		Added "your ROMs don't match" message.
		<38>	 8/25/94	ATM		Added StatusPrintGameResults.
		<37>	 8/23/94	DJ		added state->disallowGameConnect
		<36>	 8/22/94	DJ		groomed dialog text
		<35>	 8/22/94	ATM		De-CHEEZEd.
		<34>	 8/21/94	BET		Add dummy opponent name string to send to box until we get the
									database hacked up to send it back.
		<33>	 8/20/94	BET		Added calls to Server_SendOpponentNameString.
		<32>	 8/19/94	BET		serverState->gameResult is now a pointer, fix up
									Server_StartGamePlay to do the right thing.
		<31>	 8/19/94	ATM		Fixed game result printing.
		<30>	 8/18/94	DJ		smarter messages for challenges
		<29>	 8/18/94	ATM		Show challenge connects.
		<28>	 8/17/94	ATM		Added printing of game results per request.
		<27>	 8/13/94	ATM		Added separate Statusmsg for challenges.
		<26>	 8/13/94	ATM		Fixed it.
		<25>	 8/13/94	ATM		Added a sanity check on the phone number length.
		<24>	 8/13/94	ATM		Changed extractedPhoneNumber to boxPhoneNumber; trying to avoid
									"blank phone number" syndrome.
		<23>	 8/12/94	ATM		Updated Statusmsg calls to use Server_GameName.
		<22>	 8/12/94	ATM		Added Statusmsg calls.
		<21>	 8/11/94	ATM		Converted to Logmsg.
		<20>	 8/10/94	DJ		mpw sucks
		<18>	 8/10/94	ATM		Changed "find opponent" calls to include boxPhoneNumber as well
									(for area code munging).
		<17>	  8/9/94	ATM		Cope with area codes.
		<16>	  8/8/94	DJ		SendDialog takes boolean whether to stick or disappear in 3 sec.
		<15>	 7/26/94	DJ		nuthin honey
		<14>	 7/25/94	DJ		5 days of hacking, including: added support for challenges
		<13>	 7/20/94	DJ		no ServerState passed to dbase routines
		<12>	 7/19/94	DJ		gameName is now just a char*
		<11>	 7/18/94	DJ		added opponentVerificationTag (magicCookie)
		<10>	 7/17/94	BET		Make printf's go to gLogFile instead of stdout, which is the
									modem.
		 <9>	 7/15/94	DJ		made mail-only connects work
		 <8>	 7/14/94	DJ		added 1min timeout for kon
		 <7>	 7/13/94	DJ		added Server_SendRegisterPlayer
		 <6>	 7/12/94	DJ		getting ready for mail-only connections
		 <5>	  7/8/94	DJ		printfs for testing
		 <4>	  7/1/94	DJ		making server handle errors in Comm layer
		 <3>	 6/12/94	DJ		seems to work well.
		 <2>	 6/11/94	DJ		made this real at the same time as making the dbase for games
									real
		 <1>	 5/27/94	DJ		first checked in

	To Do:
*/

#include "ServerCore.h"
#include "Server.h"
#include "ServerState.h"
#include "ServerDataBase.h"
#include "Challnge.h"

#include "Matching.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>


int Server_StartGamePlay(ServerState *state)
{
	Contestant contestant;
	userIdentification challengeUserID;
	Matchup *matchup;
	RankingInfo *rankingInfo;
	Err err;
	char msg[256];
	int i;

	Logmsg("Server_StartGamePlay\n");

	ASSERT((state->validFlags & kServerValidFlag_Account));
	ASSERT((state->validFlags & kServerValidFlag_ChallengeOrCompete));

	if(state->disallowGameConnect)
	{
		Logmsg("REJECT: Game connect is disallowed.\n");
		return(kServerFuncOK);
	}

	if(state->challengeData.userID.box.box == kDownloadOnlyMailSerialNumber)
	{
		Statusmsg("SunSega: Mail-only connect from '%s' (%s)\n",
			state->loginData.userID.userName,
			state->account->boxAccount.gamePhone.phoneNumber);
		StatusPrintGameResults(state);
		return(kServerFuncOK);
	}


	// Set up the Contestant struct.
	//
	memset(&contestant, 0, sizeof(Contestant));
	contestant.gameID = state->gameIDData.gameID;
	contestant.boxSerialNumber = state->loginData.userID.box;
	contestant.player = state->loginData.userID.userID;
	strcpy(contestant.userName, state->loginData.userID.userName);
	contestant.boxPhoneNumber = state->account->boxAccount.gamePhone;
	if (state->challengeData.userID.box.box !=
		kFindNetworkOpponentSerialNumber)
	{
		contestant.challengeFlags = kSpecificChallenge;
	} else if (state->challengeData.userID.box.box ==
			   kFindNetworkOpponentSerialNumber)
	{
		if (state->account->playerAccount.playerFlags & kPlayerFlags_acceptChallenges)
			contestant.challengeFlags = kAcceptChallenges;
		else
			contestant.challengeFlags = kIgnoreChallenges;
	} else {
		Logmsg("Invalid state->challengeData.userID.box.box\n");
		return (kServerFuncAbort);
	}
	contestant.callLongDistance =
		((state->account->boxAccount.boxFlags & kBoxFlags_dialLongDistance)!=0);
	contestant.romID = state->boxOSState.boxType;	// not used anymore

	// The "opp" fields get the desired opponent's identification if it's
	// a specific match request, or the previous opponent's identification
	// if it's an auto-match request.
	//
	if (contestant.challengeFlags == kSpecificChallenge) {
		// specific request
		//
		contestant.oppBoxSerialNumber = state->challengeData.userID.box;
		contestant.oppPlayer = state->challengeData.userID.userID;

		// he wants to challenge a specific player.
		// check if that is a valid playerID.
		
		if (DataBase_FindUserIdentification(&state->challengeData.userID,
			&challengeUserID) != kNoError)
		{		
			// Barf back to the dude... no such user.
			sprintf(msg, "There is no player named %s on XBAND.  You have not been registered to play.", state->challengeData.userID.userName);
			Server_SendDialog(state, msg, true);
			return(kServerFuncOK);
		}
		
		// Do UserIDs match exactly? (box + player#)
		if (Database_CompareUserIdentifications(&challengeUserID,
			&state->loginData.userID))
		{
			Server_SendDialog(state, "Come on - you can't play yourself!  You have not been registered to play.", true);
			return(kServerFuncOK);
		}
		// Do the BoxSerialNumbers match? (just box)
		if (Database_CompareBoxSerialNumbers(&challengeUserID.box,
			&state->loginData.userID.box))
		{
			Server_SendDialog(state, "You can't play against someone on your own box!  You have not been registered to play.", true);
			return(kServerFuncOK);
		}

		contestant.oppBoxSerialNumber = challengeUserID.box;
		contestant.oppPlayer = challengeUserID.userID;

	} else {
		// auto-match requests
		//
		// BRAIN DAMAGE: fill this in correctly
		contestant.oppBoxSerialNumber.box = -1;
		contestant.oppBoxSerialNumber.region = -1;
		contestant.oppPlayer = -1;
	}

	// The "rankingInfo" field needs the ranking data for this game, if
	// any.  If the user has never played before, we need to pass in a
	// zeroed-out rankingInfo struct with a correct gameID.
	//
	// BRAIN DAMAGE: use the fake rankingInfo stuff for now
	//
	for (i = 0; i < 2; i++) {
		rankingInfo = &(state->account->playerAccount.rankingInfo[i]);

		if (rankingInfo->gameID == contestant.gameID) {
			contestant.rankingInfo = *rankingInfo;
			break;
		}
	}
	if (i == 2) {
		Logmsg("No rankingInfo for gameID 0x%.8lx, passing in empty one\n",
			contestant.gameID);
		memset(&contestant.rankingInfo, 0, sizeof(RankingInfo));
		contestant.rankingInfo.gameID = contestant.gameID;
	}


	// One-stop shopping.
	//
	matchup = RPC_FindMatch(&contestant);

	if (matchup == NULL) {
		// wow
		sprintf(msg, "Player matching appears to be down.  Try again later.");
		Server_SendDialog(state, msg, true);
		return(kServerFuncOK);
	}

	// If we got a phone number back (or even if we didn't), and the
	// "dial 9 to get out" hack is in effect, prepend "9,".
	if (state->account->boxAccount.debug1 & 1) {
		phoneNumber tmpPhoneNumber;
		strcpy(tmpPhoneNumber.phoneNumber, matchup->oppPhoneNumber.phoneNumber);
		strcpy(matchup->oppPhoneNumber.phoneNumber, "9,");
		strcat(matchup->oppPhoneNumber.phoneNumber, tmpPhoneNumber.phoneNumber);
	}


	// Generic errors.
	//
	if (matchup->result == kInvalidArguments) {
		sprintf(msg, "Looks like something is messed up in the server.  Unable to match you with anyone.");
		Server_SendDialog(state, msg, true);
		return(kServerFuncOK);
	}
	if (matchup->result == kInvalidPhoneNumber) {
		sprintf(msg, "Looks like something is messed up in the server.  The matcher can't understand your phone number.");
		Server_SendDialog(state, msg, true);
		return(kServerFuncOK);
	}

	// Handle errors based on what we tried to do.
	//
	if (contestant.challengeFlags == kSpecificChallenge) {
		// Was a specific match request.
		//
		if (matchup->result == kMatchDial)
		{
			Logmsg("*** Sending Opponent Phone Number : %s (cookie=%ld)\n",
				matchup->oppPhoneNumber.phoneNumber, matchup->oppMagicCookie);
			Statusmsg("SunSega: '%s' (%s) told to challenge '%s' (%s) for %s\n",
				contestant.userName,
				contestant.boxPhoneNumber.phoneNumber,
				matchup->oppUserName,
				matchup->oppPhoneNumber.phoneNumber,
				Common_GameName(contestant.gameID));
			StatusPrintGameResults(state);
			Server_SendOpponentNameString(state, matchup->oppUserName);
			return (Server_SendOpponentPhoneNumber(state,
				&matchup->oppPhoneNumber, matchup->oppMagicCookie));
		}
		
		if (matchup->result == kChallengeeWaitingForDifferentGame)
		{		
			sprintf(msg, "You can't play %s, because they have a different game cartridge installed.  You have not been registered to play.", challengeUserID.userName);
			Server_SendDialog(state, msg, true);
			return(kServerFuncOK);
		}
		
		if (matchup->result == kChallengeeWaitingForDifferentUser)
		{
			sprintf(msg, "You can't play %s, because they have registered to play someone else.  You have not been registered to play.", challengeUserID.userName);
			Server_SendDialog(state, msg, true);
			return(kServerFuncOK);
		}
	
		if (matchup->result == kChallengeeWaitingForAutoMatch)
		{
			// BRAIN DAMAGE.  If the other player accepts challenges, we should match you together.
			//
			sprintf(msg, "You can't play %s, because they are waiting to play a challenge game.  You have not been registered to play.", challengeUserID.userName);
			Server_SendLargeDialog(state, msg, true);
			return(kServerFuncOK);
		}

		if (matchup->result == kDifferentRomVersion)
		{
			sprintf(msg, "You can't play %s, because they have a different XBAND ROM revision.", challengeUserID.userName);
			Server_SendDialog(state, msg, true);
			return(kServerFuncOK);
		}

		if (matchup->result != kMatchWait) {
			Logmsg("specific-Matching_FindMatch returned error %ld\n",
				matchup->result);
			sprintf(msg, "Your challenge request could not be completed\n");
			Server_SendDialog(state, msg, true);
			return(kServerFuncOK);
		}
		
		// bummer.  that dude isn't online, fall out into wait handler

	} else {
		// Was an auto-match request.
		//
		if (matchup->result == kMatchDial)
		{
			Logmsg("*** Sending Opponent Phone Number : %s (cookie=%ld)\n",
				matchup->oppPhoneNumber.phoneNumber, matchup->oppMagicCookie);
			Statusmsg("SunSega: '%s' (%s) told to call '%s' (%s) for %s\n",
				contestant.userName,
				contestant.boxPhoneNumber.phoneNumber,
				matchup->oppUserName,
				matchup->oppPhoneNumber.phoneNumber,
				Common_GameName(contestant.gameID));
			StatusPrintGameResults(state);
			Server_SendOpponentNameString(state, matchup->oppUserName);
			return(Server_SendOpponentPhoneNumber(state,
				&matchup->oppPhoneNumber, matchup->oppMagicCookie));
		}

		if (matchup->result != kMatchWait) {
			Logmsg("auto-Matching_FindMatch returned error %ld\n",
				matchup->result);
			sprintf(msg, "Your challenge request could not be completed\n");
			Server_SendDialog(state, msg, true);
			return(kServerFuncOK);
		}
	}

	// Used to be sendwaitforopponent
	//
	if (matchup->result == kMatchWait)
	{		
		// Tell the box to wait for a call.
		//
		Logmsg("@@@ Wait For Opponent (my cookie=%ld)\n", matchup->magicCookie);
		if (contestant.challengeFlags == kSpecificChallenge) {
			Statusmsg("SunSega: '%s' (%s) waiting for %s challenge with %s\n",
				contestant.userName,
				contestant.boxPhoneNumber.phoneNumber,
				Common_GameName(contestant.gameID),
				challengeUserID.userName);
			StatusPrintGameResults(state);
		} else {
			Statusmsg("SunSega: '%s' (%s) waiting for %s\n",
				contestant.userName,
				contestant.boxPhoneNumber.phoneNumber,
				Common_GameName(contestant.gameID));
			StatusPrintGameResults(state);
		}
		err = Server_SendWaitForOpponent(state, matchup->magicCookie);
		if (err == kServerFuncOK)
		{
			char gameName[kGameNameSize];
		
			if (!DataBase_GetGameName(contestant.gameID, gameName))
			{
				Logmsg("Server_StartGamePlay Impl Error: game 0x%.8lx has no name in the DB??\n", contestant.gameID);
				gameName[0] = 0;
			}

			if (contestant.challengeFlags != kSpecificChallenge)
			{
				err = Server_SendRegisterPlayer(state,
					kBoxWaitForOpponentTimeout, gameName);
			}
			else
			{
				err = Server_SendRegisterChallengePlayer(state,
					kBoxWaitForOpponentTimeout, gameName,
					challengeUserID.userName);
			}
			
			return(err);
		}
		else
		{
			Logmsg("Server_SendWaitForOpponent returned error %ld\n", err);
			return(kServerFuncAbort);
		}
	}
}


// Print a game result status message.
//
void
StatusPrintGameResults(ServerState *state)
{
	char numbuf[16];

	if (state->validFlags & kServerValidFlag_GameResults) {
		sprintf(numbuf, "  ERR %ld", state->gameResult->gameError);

		Statusmsg("         localScore: %ld  remoteScore: %ld  playTime: %ld%s\n",
			state->gameResult->localPlayer1Result + state->gameResult->localPlayer2Result,
			state->gameResult->remotePlayer1Result + state->gameResult->remotePlayer2Result,
			state->gameResult->playTime,
			state->gameResult->gameError ? numbuf : "");
	} else {
		// say nothing; will make the log smaller
		//Statusmsg("         (no game results)\n");
	}
}

