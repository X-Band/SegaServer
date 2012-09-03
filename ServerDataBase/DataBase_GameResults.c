/*
	File:		DataBase_GameResults.c

	Contains:	xxx put contents here xxx

	Written by:	David Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<12>	 9/16/94	ATM		boxFlags->boxModified, playerFlags->playerModified,
									acceptChallenges->playerFlags.
		<11>	  9/3/94	ATM		Handle "xpoints" field.
		<10>	  9/2/94	ATM		Neglected to set kPA_ranking.
		 <9>	  9/2/94	ATM		Fix aforementioned tie crediting.
		 <8>	 8/31/94	ATM		Credit 1 win and 1 loss for a tie.
		 <7>	 8/28/94	ATM		Don't update game rankings if there's an error result.  (This
									may change...)
		 <6>	 8/19/94	ATM		Removed a FreeAccount call.
		 <5>	 8/19/94	ATM		De-RPCed UpdateRanking.
		 <4>	 8/19/94	ATM		Implemented DataBase_UpdateRanking.
		 <3>	  7/2/94	DJ		GameResult struct
		 <2>	 5/29/94	DJ		tweaked api
		 <1>	 5/27/94	DJ		first checked in

	To Do:
*/

#include "Server.h"	// this is only for PreformedMessage type
#include "ServerDataBase.h"
#include "BoxSer.h"

#include <stdio.h>


//
// Add Game Results to the database
//
int DataBase_AddGameResult(userIdentification *userID, const GameResult *result)
{
	return(0);
}


//
// Update PlayerAccount->rankingInfo; DO NOT RPC this call.
//
Err DataBase_UpdateRanking(Account *account, const GameResult *gameResult)
{
	RankingInfo *rankInfo;
	long pointsFor, pointsAgainst;
	int idx;

	pointsFor = gameResult->localPlayer1Result + gameResult->localPlayer2Result;
	pointsAgainst = gameResult->remotePlayer1Result + gameResult->remotePlayer2Result;
	Logmsg("UpdateRanking (%ld %ld %ld)\n",		// remove me
		pointsFor, pointsAgainst, gameResult->playTime);

	if (gameResult->gameError) {
		Logmsg("Game ended in error, rankings not updated\n");
		return (kNoError);		// not a DB error
	}

	//if ((account = DataBase_GetAccount(userID)) == NULL)	// shouldn't happen
	//	return (kFucked);

	// temporary hack to see if it works
	switch (gameResult->gameID) {
	case kMortalKombatGameID:
		idx = 0;
		break;
	case kNBAJamGameID:
	case 0x39677bdb:
		idx = 1;
		break;
	default:
		Logmsg("Not storing game results for 0x%.8lx\n", gameResult->gameID);
		//DataBase_FreeAccount(account);
		return (kNoError);
	}

	// so 0 is MK, 1 is NBA (old & new)
	rankInfo = &(account->playerAccount.rankingInfo[idx]);

	rankInfo->gameID = gameResult->gameID;		// just set this
	rankInfo->pointsFor += pointsFor;
	rankInfo->pointsAgainst += pointsAgainst;
	// assign 1 win and 1 loss for a tie
	if (pointsFor >= pointsAgainst) {
		rankInfo->wins++;
		rankInfo->xpoints++;		// XBAND points; need a multiplier too
	}
	if (pointsFor <= pointsAgainst)
		rankInfo->losses++;
	account->playerModified |= kPA_ranking;

	//DataBase_UpdateAccount(account, kPA_ranking);

	Logmsg("Added new game results for 0x%.8lx\n", gameResult->gameID);
	Logmsg("  Now wins: %ld-%ld, points: %ld-%ld\n",
		rankInfo->wins, rankInfo->losses,
		rankInfo->pointsFor, rankInfo->pointsAgainst);

	//DataBase_FreeAccount(account);
	return(0);
}

