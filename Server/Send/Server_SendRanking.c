/*
	File:		Server_SendRanking.c

	Contains:	Server send ranking function

	Written by:	Dave Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<32>	 9/12/94	ATM		Changed GameInfo stuff to Common_.  Added FreeGameInfo calls.
		<31>	  9/7/94	ATM		At highest level, was still sending previous "next rank" string.
		<30>	  9/4/94	ATM		Now use GameInfo stuff.  Removed cruft.
		<29>	  9/2/94	ATM		Minor fix.
		<28>	  9/1/94	ATM		Changed ranking strings; hope they fit on the screen now.
		<27>	 8/27/94	ATM		Joe changed his mind, '\n's are gone.
		<26>	 8/27/94	ATM		Prepended '\n's to all the ranking strings.
		<25>	 8/23/94	ATM		Changed XBAND Points to be wins.
		<24>	 8/23/94	ATM		Increased the size of a buffer.
		<23>	 8/23/94	ATM		Previous terribly important test case didn't work.
		<22>	 8/23/94	ATM		Changed rankings to work on wins (scale these vs opponent
									strength someday).
		<21>	 8/20/94	ATM		Added a new test case.
		<20>	 8/20/94	ATM		Changed secret rankings, logging.
		<19>	 8/19/94	DJ		make compile on mac
		<18>	 8/19/94	ATM		Rewrote Server_SendRanking.
		<17>	 8/18/94	BET		Fix said ranking jizz.
		<16>	 8/17/94	BET		Add sekrit ranking jizz.
		<15>	 8/12/94	ATM		Converted to Logmsg.
		<14>	  8/4/94	DJ		updated to newest rankings weirdness
		<13>	  8/2/94	DJ		commented out rankings until i update to newest stuff from Joe
		<12>	 7/26/94	DJ		sending rankings starting with dbid 1
		<11>	 7/26/94	DJ		sending 2 rankings
		<10>	 7/25/94	DJ		send 2 rankings
		 <9>	 7/20/94	DJ		added Server_Comm stuff
		 <8>	 7/19/94	DJ		tyring a dbid of 1
		 <7>	 7/19/94	DJ		sending dbid of the ranking (one per game)
		 <6>	 7/18/94	DJ		drive Ranking mgr
		 <5>	 7/12/94	DJ		using Server_TCheckError instead of TCheckError
		 <4>	  7/8/94	DJ		update to rankingmgr.h
		 <3>	  7/2/94	DJ		send real rankings
		 <2>	  6/5/94	DJ		nothing
		 <1>	 5/25/94	DJ		first checked in
	To Do:
*/


#include "ServerCore.h"
#include "Server.h"
#include "ServerDataBase.h"

#include "Messages.h"
#include "GameDB.h"
#include "RankingMgr.h"
#include "Server_Comm.h"
#include "DBTypes.h"
#include "SegaIn.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


RankingType *Server_BuildRanking(ServerState *state, RankingInfo *rankingInfo, unsigned short *bodySize, DBID *idp);

#define kNumRankStrings	5			// #of strings to send for basic ranking
#define kSecretChunk	(64*3)		// call it 64 bytes for every secret rank

// Given the player's current standings, generate the ranking data.
// Returns a pointer to newly malloc()ed memory, which must be freed
// by the caller.
//
// Returns NULL if unable to complete the request.
//
RankingType *Server_BuildRanking(ServerState *state, RankingInfo *rankingInfo, unsigned short *bodySize, DBID *idp)
{
	RankingType *rankings, metric;
	GameInfo *gameInfo;
	long numGames, gameID;
	int i, gm, rk, nextrk;
	char numBuf1[11], numBuf2[11], strBuf[64];
	char *vector[kNumRankStrings];
	unsigned char *ucp, *metric1, *metric2;
	unsigned short *usp;
	unsigned short size;

	if ((gameInfo = Common_GetGameInfo(&numGames)) == NULL) {
		Logmsg("WHOA: couldn't get GameInfo; Very Bad\n");
		return (NULL);
	}

	gameID = rankingInfo->gameID;		// ID to search for; affected by aliases

	for (gm = 0; gm < numGames; gm++) {
		if (gameInfo[gm].gameID == gameID) {
			if (gameInfo[gm].alias != 0) {
				// found an alias; change gameID and start over
				gameID = gameInfo[gm].alias;
				gm = -1;
				continue;
			}

			// found the game, find the right rank
			for (rk = 0; rk < gameInfo[gm].numRanks; rk++) {
				if (gameInfo[gm].ranks[rk].xpoints > rankingInfo->xpoints) {
					// we passed it, use the previous one
					nextrk = rk;
					rk--;
					break;
				}
			}
			if (rk == gameInfo[gm].numRanks) {	// highest rank?
				rk--;			// make this equal highest rank
				nextrk = rk;	// can't go any higher
			}

			break;
		}
	}
	if (gm == numGames) {
		Logmsg("Unable to find rank chart for %.8lx\n", rankingInfo->gameID);
		goto bail;
	}

	// XBAND Points
	sprintf(numBuf1, "%d", rankingInfo->xpoints);

	// Points Needed
	if (rk == nextrk)
		strcpy(numBuf2, "---");		// can't go no higher
	else
		sprintf(numBuf2, "%d",
			gameInfo[gm].ranks[nextrk].xpoints - rankingInfo->xpoints);

	vector[0] = gameInfo[gm].gameName;
	vector[1] = gameInfo[gm].ranks[rk].rankName;
	vector[2] = numBuf1;
	if (rk == nextrk)
		vector[3] = "---";			// that's all, folks!
	else
		vector[3] = gameInfo[gm].ranks[nextrk].rankName;
	vector[4] = numBuf2;

	Logmsg("Rank for 0x%.8lx: %s %s %s %s %s\n", rankingInfo->gameID,
		vector[0], vector[1], vector[2], vector[3], vector[4]);

	size = 0;
	for(i = 0; i < kNumRankStrings; i++)
		size += strlen(vector[i]) + 1;
	//size += sizeof(RankingType) - 1;
	// sizeof won't work; it's a 7 byte structure, but sizeof tells us
	// that it's 8 to keep things happy
	metric1 = (unsigned char *)&metric;
	metric2 = (unsigned char *)&(metric.rankData[0]);
	size += (metric2 - metric1);

	rankings = (RankingType *)malloc((long)size + kSecretChunk);

	rankings->gameID = 			gameID;		// use aliased gameID here
	rankings->userID = 			state->loginData.userID.userID;

	PackStrings(rankings->rankData, kNumRankStrings, vector);

	// Set up the secret rankings.  Right now, every game gets two.  All
	// of this stuff has to fit within kSecretChunk bytes.
	//
	rankings->numHiddenStats = 2;
	if (rankings->gameID == kNBAJamGameID || rankings->gameID == 0x39677bdb)
		rankings->numHiddenStats++;		// ATM

	// first secret rank: total points
	size += (size & 1);		// 16-bit word-align
	ucp = (unsigned char *)rankings;
	ucp += size;

	*ucp++ = 2, size++;		// make it a two-key sequence
	*ucp++ = 0, size++;		// pad
	usp = (unsigned short *) ucp;
	*usp++ = kUP, size += 2;
	*usp++ = kUP, size += 2;
	ucp = (unsigned char *) usp;
	sprintf(strBuf, "Total points for: %ld, against: %ld",
		rankingInfo->pointsFor, rankingInfo->pointsAgainst);
	strcpy((char *)ucp, strBuf);
	ucp += strlen(strBuf) +1, size += strlen(strBuf) +1;
	size += (size & 1);		// 16-bit word-align

	// second secret rank: win/loss, and my opinion
	size += (size & 1);		// 16-bit word-align
	ucp = (unsigned char *)rankings;
	ucp += size;

	*ucp++ = 2, size++;		// make it a two-key sequence
	*ucp++ = 0, size++;		// pad
	usp = (unsigned short *) ucp;
	*usp++ = kUP, size += 2;
	*usp++ = kDOWN, size += 2;
	ucp = (unsigned char *) usp;
	sprintf(strBuf, "Total wins: %ld, losses: %ld",
		rankingInfo->wins, rankingInfo->losses);
	if (rankingInfo->wins > rankingInfo->losses)
		strcat(strBuf, " (you rewl)");
	else
		strcat(strBuf, " (you suck)");
	strcpy((char *)ucp, strBuf);
	ucp += strlen(strBuf) +1, size += strlen(strBuf) +1;
	size += (size & 1);		// 16-bit word-align

	// third secret rank: only for NBA Jam
	if (rankings->numHiddenStats > 2) {
		size += (size & 1);		// 16-bit word-align
		ucp = (unsigned char *)rankings;
		ucp += size;

		*ucp++ = 7, size++;		// try 7 keys; max is 16 (up/down only sucks)
		*ucp++ = 0, size++;		// pad
		usp = (unsigned short *) ucp;
		//*usp++ = kDOWN, size += 2;
		*usp++ = kDOWN | kLEFT, size += 2;
		*usp++ = kDOWN, size += 2;
		*usp++ = kDOWN, size += 2;
		*usp++ = kDOWN, size += 2;
		*usp++ = kDOWN, size += 2;
		*usp++ = kDOWN, size += 2;
		*usp++ = kUP, size += 2;
		ucp = (unsigned char *) usp;
		strcpy(strBuf, "Dave, Brian, & Andy2 were here");
		strcpy((char *)ucp, strBuf);
		ucp += strlen(strBuf) +1, size += strlen(strBuf) +1;
		size += (size & 1);		// 16-bit word-align
	}

	// All done!
	*bodySize = size;
	*idp = gameInfo[gm].gameID;
	Common_FreeGameInfo(gameInfo);
	return (rankings);

bail:
	Common_FreeGameInfo(gameInfo);
	return (NULL);
}


// Send the player's rankings for every game he has played.
//
int Server_SendRanking(ServerState *state)
{
RankingType	*rankings;
RankingInfo *rankingInfo;
messOut		opCode;
DBID		id;
unsigned short		bodySize;
int i;

	Logmsg("Server_SendRanking\n");

	ASSERT(state->validFlags & kServerValidFlag_Account);
	if (!state->validFlags & kServerValidFlag_Account)
		return (kFucked);

	// For every entry in playerAccount.rankings, send them a description
	// of their rank.
	//
	for (i = 0; i < 2; i++) {		// BRAIN DAMAGE; should walk a list
		rankingInfo = &(state->account->playerAccount.rankingInfo[i]);

		if (!rankingInfo->gameID)
			continue;

		rankings = Server_BuildRanking(state, rankingInfo, &bodySize, &id);

		if (!rankings)
			continue;

		opCode = msReceiveRanking;
		Server_TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);
		Server_TWriteDataSync(state->session, sizeof(DBID), (Ptr)&id);
		Server_TWriteDataSync(state->session, sizeof(unsigned short), (Ptr)&bodySize);
		Server_TWriteDataSync(state->session, (long)bodySize, (Ptr)rankings);

		Logmsg("  Sent rankings for 0x%.8lx (%ld bytes, DBID=%ld)\n",
			rankings->gameID, (long)bodySize, (long)id);
		free(rankings);
	}


	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);
	Logmsg("Server_SendRanking done\n");

	return(kServerFuncOK);
}


int Server_SendDBConstants(ServerState *state, long numConsts, DBID *ids, long *constants)
{
messOut		opCode;

	Logmsg("Server_SendDBConstants\n");

	opCode = msSetConstants;
	Server_TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);

	Server_TWriteDataSync(state->session, sizeof(long), (Ptr)&numConsts);
	Server_TWriteDataSync(state->session, sizeof(DBID) * numConsts, (Ptr)ids);
	Server_TWriteDataSync(state->session, sizeof(long) * numConsts, (Ptr)constants);

	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	Logmsg("Server_SendDBConstants done\n");

	return(kServerFuncOK);
}




int Server_SendAddItemToDB(ServerState *state, DBType theType, DBID theID, long length, void *data)
{
messOut		opCode;

	Logmsg("Server_SendAddItemToDB\n");

	opCode = msAddItemToDB;
	Server_TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);

	Server_TWriteDataSync(state->session, sizeof(DBType), (Ptr)&theType);
	Server_TWriteDataSync(state->session, sizeof(DBID), (Ptr)&theID);
	Server_TWriteDataSync(state->session, sizeof(long), (Ptr)&length);
	Server_TWriteDataSync(state->session, length, (Ptr)data);

	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	Logmsg("Server_SendAddItemToDB done\n");

	return(kServerFuncOK);
}


/*

	7/17/94
	
	These have yet to be used/tested:
	
	DoDeleteRanking
	DoGetNumRankings
	DoGetFirstRankingID
	DoGetNextRankingID
	DoGetRankingData
	
	-dj
*/


