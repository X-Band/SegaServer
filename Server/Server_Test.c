/*
	File:		Server_Test.c

	Contains:	Various tests of server messages

	Written by:	Josh Horwich

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <5>	 8/12/94	ATM		Converted to Logmsg.
		 <4>	  8/2/94	DJ		commented out rankings for now (they've changed all to hell)
		 <3>	 7/31/94	DJ		no writeable strings anymore
		 <2>	 7/21/94	JBH		Added a bunch of nasty comm test code.
		 <1>	 7/19/94	DJ		first checked in
	To Do:
*/

#include "ServerCore.h"
#include "Server.h"
#include "ServerState.h"
#include "ServerDataBase.h"
#include "DBTypes.h"
#include "Mail.h"
#include "SendQ.h"
#include "BoxSer.h"
#include "RankingMgr.h"

#include "Messages.h"
#include "PlayerDB.h"
#include "TransportLayer.h"

#include <stdio.h>
#include <string.h>


static int DoRankingTests(ServerState *state);
static int DoDBTests(ServerState *state);
static int DoDBConstTests(ServerState *state);

int Server_SendTests(ServerState *state)
{

	DBID	theID;
	long length;
	char newString[40] = "help me test this.\0";
	unsigned char 	opCode;

	Logmsg("Server_SendTests\n");


//
// Delete writeable strings is no more.  no need to delete them on the box, says kon.
//  7/31/94
//
#if 0
	Logmsg("Testing DeleteWriteableString\n");

	opCode = msDeleteWriteableString;
	TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);

	theID = 3;

	TWriteDataSync(state->session, sizeof(DBID), (Ptr) &theID);
#endif

	
	Logmsg("Testing ReceiveWriteableString\n");
	opCode = msReceiveWriteableString;
	TWriteDataSync(state->session, sizeof(messOut), (Ptr) &opCode);
	
	theID = 128;		/* override the thank-you message */
	TWriteDataSync(state->session, sizeof(DBID), (Ptr) &theID);

	
	length = strlen( newString ) + 1;
	TWriteDataSync(state->session, sizeof(long), (Ptr) &length);
	
	TWriteDataSync(state->session, length, (Ptr) newString);

	Logmsg("Server_SendTests done\n");

//	if(Server_TCheckError() != noErr)
//		return(kServerFuncAbort);


	return (DoRankingTests(state));
}

static int DoRankingTests(ServerState *state)
{
	DBID theID;
	unsigned char opCode;
	short numRankings;
	RankingType	rankings;

#ifdef JOEBROKETHIS

	Logmsg("----------------------\n");
	Logmsg("DoRankingTests\n");
	Logmsg("DoDeleteRanking\n");
	
	opCode = msDeleteRanking;
	TWriteDataSync(state->session, sizeof(messOut), (Ptr) &opCode);
	
	theID = 1;			/* user 0 MK rank */
	TWriteDataSync(state->session, sizeof(DBID), (Ptr) &theID);
	
	Logmsg("DoGetNumRankings\n");
	opCode = msGetNumRankings;
	TWriteDataSync(state->session, sizeof(messOut), (Ptr) &opCode);

	if(TReadDataSync(state->session, sizeof(short), (Ptr)&numRankings) != noErr)
		return(kServerFuncAbort);

	Logmsg("Num Rankings is %d\n", numRankings);
	
	// we expect 1 ranking to be in place from the server. Let's add another to make the next
	// tests interesting
	
	Logmsg("DoReceiveRankings\n");
	rankings.gameID = kMortalKombatGameID;
	rankings.userID	= 2;
	strcpy(rankings.gameName, "Mortal Kombat II");
	strcpy(rankings.curLevelString, "Black Belt");
	strcpy(rankings.curPointsString, "69");
	strcpy(rankings.nextLevelString, "No Belt");
	strcpy(rankings.nextPointsString, "200");
	theID = 1;	// BRAIN DAMAGE.  Each game should have its own DBID for rankings

	opCode = msReceiveRanking;
	TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);
	TWriteDataSync(state->session, sizeof(DBID), (Ptr)&theID);
	TWriteDataSync(state->session, sizeof(RankingType), (Ptr)&rankings);
	
	// OK, now we expect 2 rankings
	opCode = msGetNumRankings;
	TWriteDataSync(state->session, sizeof(messOut), (Ptr) &opCode);

	if(TReadDataSync(state->session, sizeof(short), (Ptr)&numRankings) != noErr)
		return(kServerFuncAbort);

	Logmsg("Num Rankings is now %d\n", numRankings);
	
	Logmsg("DoGetFirstRankingID.\n");
	
	opCode = msGetFirstRankingID;
	TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);
	if(TReadDataSync(state->session, sizeof(DBID), (Ptr)&theID) != noErr)
		return(kServerFuncAbort);

	numRankings = theID;
	Logmsg("First Ranking ID is %d\n", numRankings);
	
	if (theID != 1)
		Logmsg("***ERROR*** expected ID 1.\n");
	
	
	Logmsg("DoGetNextRankingID.\n");
	opCode = msGetNextRankingID;
	TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);
	TWriteDataSync(state->session, sizeof(DBID), (Ptr) &theID);
	
	if(TReadDataSync(state->session, sizeof(DBID), (Ptr)&theID) != noErr)
		return(kServerFuncAbort);

	numRankings = theID;
	Logmsg("Next Ranking ID is %d\n", numRankings);
	
	if (theID != 0)
		Logmsg("***ERROR*** expected ID 0.\n");
		
		
	Logmsg("DoGetRankingData\n");
	opCode = msGetRankingData;
	TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);
	theID = 1;
	TWriteDataSync(state->session, sizeof(DBID), (Ptr) &theID);

	rankings.userID = 10;		/* dummy value */
	strcpy(rankings.curLevelString, "<<nothing>>");

	if(TReadDataSync(state->session, sizeof(RankingType), (Ptr)&rankings) != noErr)
		return(kServerFuncAbort);

	if (rankings.userID != 2)
		Logmsg("***ERROR*** wrong userID in ranking.\n");

	Logmsg("curLevelString is %s\n", rankings.curLevelString);
	
	Logmsg("----------------------\n");
	
//	if (Server_TCheckError() != noErr)
//		return (kServerFuncAbort);
	
	return ( DoDBTests( state ) );

#else
	return(kServerFuncOK);
#endif JOEBROKETHIS

}

static int DoDBTests( ServerState *state )
{
	DBID theID;
	DBType theType;
	Ptr inDataPtr, outDataPtr;
	char inBuffer[200], outBuffer[200];
	unsigned char opCode;
	long length;
	Boolean shitHappened = false;		// not yet !!
	
	Logmsg("Beginning DB Tests.\n");
	
	Logmsg("Adding 2 DB items.\n");
	theType = 200;		/* some unused DB type, we hope! */
	inDataPtr = (Ptr ) inBuffer;	/* some space! */
	outDataPtr = (Ptr ) outBuffer;	/* some space! */
	
	*inDataPtr++ = 'H';
	*inDataPtr++ = 'e';
	*inDataPtr++ = 'l';
	*inDataPtr++ = 'l';
	*inDataPtr++ = 'o';
	*inDataPtr++ = 0;
	inDataPtr = (Ptr ) inBuffer;

	// write out a few new db items	
	opCode = msAddItemToDB;
	
	theID = 1;
	length = strlen(inBuffer) + 1;
	
	{
		short smallSize = length;
		Logmsg("SENDING SIZE is %d\n", smallSize);
	}
	
	if (length != 6)
		Logmsg("***ERROR*** 000000 I can't even call strlen right!\n");
	TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);
	TWriteDataSync(state->session, sizeof(DBType), (Ptr) &theType);
	TWriteDataSync(state->session, sizeof(DBID), (Ptr) &theID);
	TWriteDataSync(state->session, sizeof(long), (Ptr) &length);
	TWriteDataSync(state->session, length, inDataPtr);
	
	theID = 2;
	*inDataPtr++ = 'B';
	*inDataPtr++ = 'y';
	*inDataPtr++ = 'e';
	*inDataPtr++ = '!';
	*inDataPtr++ = 0;
	inDataPtr = (Ptr ) inBuffer;
	
	length = strlen(inBuffer) + 1;
	TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);
	TWriteDataSync(state->session, sizeof(DBType), (Ptr) &theType);
	TWriteDataSync(state->session, sizeof(DBID), (Ptr) &theID);
	TWriteDataSync(state->session, sizeof(long), (Ptr) &length);
	TWriteDataSync(state->session, length, inDataPtr);

	// now let's see if we can query the id's from the db
	
	opCode = msGetTypeIDsFromDB;
	TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);
	TWriteDataSync(state->session, sizeof(DBType), (Ptr) &theType);
	
	{
		short count;
		short iii, temp;
		
		if(TReadDataSync(state->session, sizeof(short), (Ptr)&count) != noErr)
			return(kServerFuncAbort);
			
		Logmsg("%d ID's in the database.\n", count);
		
		for (iii=0;iii<count;iii++) {
			if(TReadDataSync(state->session, sizeof(DBID), (Ptr)&theID) != noErr)
				return(kServerFuncAbort);
			
			temp = theID;
			Logmsg("entry #%d is ID %d.\n", iii, temp);
		}
	}
	
	// now let's read individual item ID's from the DB
	
	Logmsg("Testing GetFirstItemIDFromDB.\n");
	opCode = msGetFirstItemIDFromDB;
	TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);
	TWriteDataSync(state->session, sizeof(DBType), (Ptr) &theType);
	
	if(TReadDataSync(state->session, sizeof(DBID), (Ptr)&theID) != noErr)
		return(kServerFuncAbort);
		
	if (theID == 2)
		Logmsg("Received ID 2 as expected.\n");
	else {
		Logmsg("***ERROR*** 11111 received ID %d.\n", theID);
		shitHappened = true;
	}
	
	Logmsg("Testing GetNextItemIDFromDB.\n");
	opCode = msGetNextItemIDFromDB;
	TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);
	TWriteDataSync(state->session, sizeof(DBType), (Ptr) &theType);
	TWriteDataSync(state->session, sizeof(DBID), (Ptr) &theID);
	
	if(TReadDataSync(state->session, sizeof(DBID), (Ptr)&theID) != noErr)
		return(kServerFuncAbort);
		
	if (theID == 1)
		Logmsg("Received ID 1 as expected.\n");
	else {
		Logmsg("***ERROR*** 22222received ID %d.\n", theID);
		shitHappened = true;
	}
	
	// now let's grab an item from the DB
	Logmsg("Testing GetItemFromDB.\n");
	opCode = msGetItemFromDB;
	theType = 200;
	theID = 1;
	TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);
	TWriteDataSync(state->session, sizeof(DBType), (Ptr) &theType);
	TWriteDataSync(state->session, sizeof(DBID), (Ptr) &theID);
	
	{
		long size;
		if(TReadDataSync(state->session, sizeof(long), (Ptr)&size) != noErr)
			return(kServerFuncAbort);
			
		if (size==6) {
			Logmsg("Item length is 6, as expected.\n");
		} else {
			short smallSize = size;
			Logmsg("***ERROR*** 33333 item length is %d.\n", smallSize);
			shitHappened = true;
		}
		
		if(TReadDataSync(state->session, size, outDataPtr) != noErr)
			return(kServerFuncAbort);
			
		Logmsg("Entry is %s\n", outBuffer);
	}
	
	// now let's grab an item from the DB
	Logmsg("Testing GetItemFromDB.\n");
	opCode = msGetItemFromDB;
	theType = 200;
	theID = 2;
	TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);
	TWriteDataSync(state->session, sizeof(DBType), (Ptr) &theType);
	TWriteDataSync(state->session, sizeof(DBID), (Ptr) &theID);
	
	{
		long size;
		if(TReadDataSync(state->session, sizeof(long), (Ptr)&size) != noErr)
			return(kServerFuncAbort);
			
		if (size==5) {
			Logmsg("Item length is 5, as expected.\n");
		} else {
			short smallSize = size;
			Logmsg("***ERROR*** 33333 item length is %d.\n", smallSize);
			shitHappened = true;
		}
		
		if(TReadDataSync(state->session, size, outDataPtr) != noErr)
			return(kServerFuncAbort);
			
		Logmsg("Entry is %s\n", outBuffer);
	}
	
	// now let's delete our crap
	Logmsg("Testing DoDeleteItemFromDB.\n");
	opCode = msDeleteItemFromDB;
	theID = 2;
	TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);
	TWriteDataSync(state->session, sizeof(DBType), (Ptr) &theType);
	TWriteDataSync(state->session, sizeof(DBID), (Ptr) &theID);

	// there's one left. 

	opCode = msGetTypeIDsFromDB;
	TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);
	TWriteDataSync(state->session, sizeof(DBType), (Ptr) &theType);
	
	{
		short count;
		short iii, temp;
		
		if(TReadDataSync(state->session, sizeof(short), (Ptr)&count) != noErr)
			return(kServerFuncAbort);
			
		Logmsg("%d ID's in the database.\n", count);
		
		if (count != 1) {
			Logmsg("***ERROR*** 44444should only be one left.\n");
			shitHappened = true;
		}
		
		for (iii=0;iii<count;iii++) {
			if(TReadDataSync(state->session, sizeof(DBID), (Ptr)&theID) != noErr)
				return(kServerFuncAbort);
			
			temp = theID;
			Logmsg("entry #%d is ID %d.\n", iii, temp);
		}
	}
	
	opCode = msDeleteItemFromDB;
	theID = 1;
	TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);
	TWriteDataSync(state->session, sizeof(DBType), (Ptr) &theType);
	TWriteDataSync(state->session, sizeof(DBID), (Ptr) &theID);
	
	// and then there were 0. Let's count again.
	opCode = msGetTypeIDsFromDB;
	TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);
	TWriteDataSync(state->session, sizeof(DBType), (Ptr) &theType);
	{
		short count;
		short iii, temp;
		
		if(TReadDataSync(state->session, sizeof(short), (Ptr)&count) != noErr)
			return(kServerFuncAbort);
			
		Logmsg("%d ID's in the database.\n", count);
		
		if (count != 0) {
			Logmsg("***ERROR*** 55555 should only be 0, there are %d left.\n", count);
			shitHappened = true;
		}

		for (iii=0;iii<count;iii++) {
			if(TReadDataSync(state->session, sizeof(DBID), (Ptr)&theID) != noErr)
				return(kServerFuncAbort);
			
			temp = theID;
			Logmsg("entry #%d is ID %d.\n", iii, temp);
		}
	}
	
	if (shitHappened)
		Logmsg("***********ERROR*************");
	else
		Logmsg("Done testing DB stuff, all's OK.\n");
		
	retu