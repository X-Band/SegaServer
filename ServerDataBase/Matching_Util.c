/*
	File:		Matching_Util.c

	Contains:	Routines used by matching stuff

	Written by:	Andy McFadden

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<15>	 9/16/94	DJ		turned on MATCH_BY_AREA_CODE_ONLY
		<14>	 9/16/94	ATM		Changed purgatory queue logging by adding a new argument to
									DumpContestant.  Made specific queue expire people with a
									different timeout.
		<13>	 9/13/94	ATM		Changed to ReloadPhoneStuff.
		<12>	 9/12/94	DJ		added loading of Compuserve POP database as well at LATA
									database (didn't replace LATA because that is still used to stop
									east-west coast opponent matches)
		<11>	 9/12/94	ATM		Handle some new control messages.
		<10>	 9/12/94	ATM		Okay, try that again.
		 <9>	 9/12/94	ATM		Call LoadFakeLata.
		 <8>	 9/12/94	ATM		Implemented MatchingControl stuff.
		 <7>	 9/11/94	ATM		Added MATCH_BY_AREA_CODE_ONLY.  Not tested.
		 <6>	  9/8/94	ATM		Spiffed up the logging.
		 <5>	  9/7/94	ATM		Minor tweaks.
		 <4>	  9/7/94	ATM		Choked on cross-area-code calls.
		 <3>	  9/6/94	ATM		Fixed expiration code.
		 <2>	  9/6/94	ATM		Add TweakPhoneNumber.  Minor fixes.
		 <1>	  9/6/94	ATM		first checked in

	To Do:
*/


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "Matching.h"

//
// Turn on matching by area code.  If longdistance calling not enabled, box will dial only
// within its area code.  You can add gamephonenumbers of boxes that you want to be able to
// do long distance calling by editing the "LongDistance.phonenumbers" file in /var/catapult
// This file is read by SunSega in Server_ValidateLogin().  This will be obsoleted by the
// database admin tool once we are up on Oracle.  9/16/94  -dj
//
#define MATCH_BY_AREA_CODE_ONLY	1


// ===========================================================================
//		Prototypes for local routines
// ===========================================================================

int Matching_InitializeContestant(Contestant *contestant);
Err Matching_SanityCheckContestant(Contestant *contestant);

void Matching_DumpQueues(void);


// ===========================================================================
//		State for this region
// ===========================================================================

static int initialized = 0;				// sanity check

RegionGlobals gRegionGlobals;			// all about us

static GameInfo *gGameInfo = NULL;		// local copy of GameInfo
static long gNumGames;					// (not really global, just here)



// ===========================================================================
//		Startup and Shutdown
// ===========================================================================

//
// Initialize our little world.
//
// Returns TRUE on success, FALSE on failure.
//
Err Matching_MatchingStartup(void)
{
	MatchingGameData *mgd;
	GameInfo *gip;
	ListNode *lnode;
	int i;

	if (initialized) {
		Logmsg("HEY: tried to initialize region %ld twice!\n",
			gRegionGlobals.regionID);
		return (false);
	}

	// Start by zeroing everything to nuke the statistics.
	//
	memset(&gRegionGlobals, 0, sizeof(RegionGlobals));

	// We get this once, and use it forever.
	//
	if ((gGameInfo = Common_GetGameInfo(&gNumGames)) == NULL) {
		Logmsg("Init: unable to get game info\n");
		return (false);
	}
	if (gNumGames < 0 || gNumGames > 512) {
		Logmsg("ERROR: Init: bogus numGames %ld\n",
			gNumGames);
		return (false);
	}

	// Init the useful fields inside RegionGlobals
	//
	gRegionGlobals.regionID = 0;			// for now, only one region
	gRegionGlobals.boxArch = 'sega';		// we like SEGA, yes we do

	gRegionGlobals.globalQueue = NewSortedList();
	gRegionGlobals.specificQueue = NewSortedList();
	gRegionGlobals.purgatoryQueue = NewSortedList();

	// Create an MGD entry for every GameInfo record we have, even if the
	// entry is just an alias.  This makes it easier to search.  I'm
	// optimizing for the non-alias case, and assuming that there won't
	// be all that many aliases.
	gRegionGlobals.gameData = NewSortedList();

	for (i = 0; i < gNumGames; i++) {
		// Purify sez: these don't get freed in Shutdown.  It's right,
		// but we don't care. :-)
		mgd = (MatchingGameData *)malloc(sizeof(MatchingGameData));
		if (mgd == NULL) {
			Logmsg("ERROR: Init: malloc failed\n");
			return (false);
		}

		// init fields (notably statistics) to zero
		memset(mgd, 0, sizeof(MatchingGameData));

		mgd->gameID = gGameInfo[i].gameID;
		mgd->gameInfo = &gGameInfo[i];
		mgd->gameQueue = NewSortedList();		// this is the game queue

		lnode = NewSortedListNode((Ptr) mgd, gGameInfo[i].gameID);
		ASSERT(lnode);
		AddListNodeToSortedList(gRegionGlobals.gameData, lnode);
	}

	gRegionGlobals.startTime = time(0);

	// Load the LATA database.
	//
	DataBase_LoadFakeLata();
	DataBase_LoadCompuservePOPs();	// Load the Compuserve POPs

	// done!
	//
	Logmsg("Matching_MatchingStartup successful\n");
	initialized = true;
	return (true);
}

//
// Shut it down.
//
// Useful mainly as a way to dump the statistics before bailing.
//
// Returns (true) on success, (false) on failure.
//
Err Matching_MatchingShutdown(void)
{
	char arch[5];
	time_t now;

	if (!initialized) {		// didn't start up, don't shut down
		Logmsg("GLITCH: Shutdown without Startup\n");
		return (true);
	}

	now = time(0);

	// This is just to make the stats pretty, in case nobody has logged
	// in for a while and we've got a bunch of guys sitting around
	// waiting to be expired.
	//
	Matching_ExpireContestants(now);

	// Dump the statistics
	//
	*(long *)arch = gRegionGlobals.boxArch;
	arch[4] = '\0';
	Logmsg("\n");
	Logmsg("----- MATCHING STATS for arch '%.4s' region %d\n",
		arch, gRegionGlobals.regionID);

	Logmsg("  startTime 0x%.8lx, elapsed time %ld seconds\n",
		gRegionGlobals.startTime, now - gRegionGlobals.startTime);
	Logmsg("  numConnections      %ld\n", gRegionGlobals.numConnections);
	Logmsg("  numRequests         %ld\n", gRegionGlobals.numRequests);
	Logmsg("  numSuccesses        %ld\n", gRegionGlobals.numSuccesses);
	Logmsg("  numLongDistance     %ld\n", gRegionGlobals.numLongDistance);
	Logmsg("\n");
	Logmsg("  numSpecificReq      %ld\n", gRegionGlobals.numSpecificReq);
	Logmsg("  numSpecificSuc      %ld\n", gRegionGlobals.numSpecificSuc);
	Logmsg("  numAutoReq          %ld\n", gRegionGlobals.numAutoReq);
	Logmsg("  numAutoSuc          %ld\n", gRegionGlobals.numAutoSuc);
	Logmsg("  numAutoSpecificSuc  %ld\n", gRegionGlobals.numAutoSpecificSuc);
	Logmsg("\n");
	Logmsg("  numEnqueued         %ld\n", gRegionGlobals.numEnqueued);
	Logmsg("  numTimeouts         %ld\n", gRegionGlobals.numTimeouts);
	Logmsg("  numRequeued         %ld\n", gRegionGlobals.numRequeued);
	Logmsg("  numReborn           %ld\n", gRegionGlobals.numReborn);
	Logmsg("  timeEnqueued        %ld\n", gRegionGlobals.timeEnqueued);
	Logmsg("\n");

	// Free up data structures.
	//
	Common_FreeGameInfo(gGameInfo);
	gGameInfo = NULL;
	DisposeList(gRegionGlobals.globalQueue);
	gRegionGlobals.globalQueue = NULL;
	DisposeList(gRegionGlobals.gameData);
	gRegionGlobals.gameData = NULL;

	initialized = 0;

	Logmsg("Matching_MatchingShutdown successful\n");
	return (true);
}


// ===========================================================================
//		Main entry points from RPC calls
// ===========================================================================

//
// Entry point for all matching requests.
//
Matchup *Matching_FindMatch(Contestant *contestant)
{
	GameInfo *thisGameInfo;
	static Matchup matchup;
	Err err;
	static time_t lastExpire = 0;
	time_t now;

	if (!initialized) {
		Logmsg("Matching was not initialized, aborting.\n");
		Matching_Abort();
		return (NULL);
	}

	gRegionGlobals.numConnections++;		// incr for every RPC function

	//
	// Look for deadwood every so often.
	//
	now = time(0);
	if (now - lastExpire > kMatchScanInterval) {
		Matching_ExpireContestants(now);
		lastExpire = now;
	}

	//
	// Init the matchup struct.  Not strictly necessary.
	//
	memset(&matchup, 0, sizeof(Matchup));

	//
	// Sanity-check the contestant args.
	//
	if ((err = Matching_SanityCheckContestant(contestant)) != kNoError) {
		matchup.result = err;
		goto bail;
	}

	gRegionGlobals.numRequests++;		// incr for all requests

	//
	// Initialize the rest of the fields.
	//
	Matching_InitializeContestant(contestant);

	Logmsg("--- ENTRY: => %ld <= '%s' (%s)\n",
		contestant->contestantID, contestant->userName,
		contestant->boxPhoneNumber.phoneNumber);

	// DEBUG: dump the queues before we go to work
	Logmsg("New contestant:\n");
	Matching_DumpContestant(contestant, MDC_VERBOSE);
	Matching_DumpQueues();

	//
	// If he's on a queue already, remove him.
	//
	Matching_DequeueIfQueued(contestant);

	//
	// Depending on challengeFlags, do an auto or specific match.
	//
	switch (contestant->challengeFlags) {
	case kIgnoreChallenges:
	case kAcceptChallenges:
		// auto-match
		Matching_AutoMatch(contestant, &matchup);
		break;
	case kSpecificChallenge:
		// specific match
		Matching_Specific(contestant, &matchup);
		break;
	default:
		Logmsg(
			"How the HELL did we get here?  Stupid sanity checker.\n");
		Matching_Abort();
		matchup.result = kFucked;
		break;
	}

bail:
	// DEBUG: dump the queues before we bail
	Matching_DumpQueues();

	//
	// Return the Matchup struct to the caller.
	//
	Logmsg("--- RETURN:");
	switch (matchup.result) {
	case kMatchDial:
		Logmsg(" kMatchDial");
		break;
	case kMatchWait:
		Logmsg(" kMatchWait");
		break;
	case kNoSuchGameID:
		Logmsg(" REJECT (kNoSuchGameID)");
		break;
	case kChallengeeWaitingForDifferentGame:
		Logmsg(" REJECT (kChallengeeWaitingForDifferentGame)");
		break;
	case kChallengeeWaitingForDifferentUser:
		Logmsg(" REJECT (kChallengeeWaitingForDifferentUser)");
		break;
	case kChallengeeNotAvailable:
		Logmsg(" REJECT (kChallengeeNotAvailable)");
		break;
	case kChallengeeWaitingForAutoMatch:
		Logmsg(" REJECT (kChallengeeWaitingForAutoMatch)");
		break;
	case kChallengeeTooFar:
		Logmsg(" REJECT (kChallengeeTooFar)");
		break;
	case kDifferentRomVersion:
		Logmsg(" REJECT (kDifferentRomVersion)");
		break;
	case kOpponentNotRegistered:
		Logmsg(" REJECT (kOpponentNotRegistered)");
		break;
	case kInvalidArguments:
		Logmsg(" REJECT (kInvalidArguments)");
		break;
	case kInvalidPhoneNumber:
		Logmsg(" REJECT (kInvalidPhoneNumber)");
		break;
	default:
		Logmsg(" %ld", matchup.result);
		break;
	}
	Logmsg("\n");

	return (&matchup);
}


//
// Do all sorts of fascinating things.
//
// The result sometimes has to be allocated dynamically, and can't be
// freed until the next time we get control back from the RPC stuff.  So
// we either return a pointer to a static struct, or a pointer to dynamic
// mem that gets freed next time through.
//
// Returns NULL on error.
//
MatchControlResult *Matching_MatchingControl(MatchControl *matchControl)
{
	static MatchControlResult *result = NULL;
	static MatchControlResult_Status resultStatus = {
		kMatchCtlRes_Status, 0, 0 };
	time_t now;

	if (!initialized) {
		Logmsg("Matching was not initialized, aborting.\n");
		Matching_Abort();
		return (NULL);
	}

	gRegionGlobals.numConnections++;		// incr for every RPC function

	// Free the one we allocated last time.
	//
	if (result != NULL) {
		free(result);
		result = NULL;
	}

	Logmsg("CONTROL: rcvd request %ld\n", matchControl->type);

	// Handle this request.
	//
	switch (matchControl->type) {
	case kMatchCtl_Ping:
		resultStatus.result = kNoError;
		return ((MatchControlResult *) &resultStatus);
	case kMatchCtl_Shutdown:
		if (matchControl->shutdown.validate == kMatchShutdownValidate) {
			Matching_MatchingShutdown();
			Logmsg("EXITING\n");
			exit(0);
		} else {
			Logmsg("WARNING: Bogus shutdown request\n");
			resultStatus.result = kFucked;
			return ((MatchControlResult *) &resultStatus);
		}
	case kMatchCtl_Reload:
		DataBase_ReloadPhoneStuff();
		resultStatus.result = kNoError;
		return ((MatchControlResult *) &resultStatus);
	case kMatchCtl_DoExpire:
		if ((now = matchControl->doExpire.when) == 0)
			now = time(0);
		Matching_ExpireContestants(now);
		resultStatus.result = kNoError;
		return ((MatchControlResult *) &resultStatus);

	case kMatchCtl_GetContestant:
	case kMatchCtl_DequeueContestant:
	case kMatchCtl_AddContestant:
	case kMatchCtl_GetQueues:
		Logmsg("WARNING: unimplemented control function\n");
		resultStatus.result = kFucked;
		return ((MatchControlResult *) &resultStatus);

	default:
		Logmsg("ERROR: unrecognized control request\n");
		return (NULL);
	}

	/*NOTREACHED*/
}


// ===========================================================================
//		Minor utility routines
// ===========================================================================

//
// Make sure all of the arguments are valid.
//
// Returns a value suitable for Matchup.result.
//
Err Matching_SanityCheckContestant(Contestant *contestant)
{
	GameInfo *gameInfo;
	int i;

	// see if the game exists
	//
	for (i = 0, gameInfo = gGameInfo; i < gNumGames; i++, gameInfo++) {
		if (gameInfo->gameID == contestant->gameID)
			break;
	}
	if (i == gNumGames) {
		Logmsg("REJECT: gameID 0x%.8lx not recognized\n",
			contestant->gameID);
		Matching_Abort();
		return (kNoSuchGameID);
	}

	// how to check boxSerialNumber?

	// check the player #
	//
	if (contestant->player < 0 || contestant->player > 3) {
		Logmsg("REJECT: invalid player # %d\n", contestant->player);
		return (kInvalidArguments);
	}

	// check userName
	//
	if (*contestant->userName == '\0') {
		Logmsg("REJECT: blank userName for '%s'\n",
			contestant->boxPhoneNumber.phoneNumber);
		return (kInvalidArguments);
	}

	// Normalize his phone number, and verify that it's sane.
	//
	{
		phoneNumber tmpPhone;
		char *cp, *wp;
		int len;

		// Start by stripping the number down to bare digits.
		//
		cp = contestant->boxPhoneNumber.phoneNumber;
		wp = tmpPhone.phoneNumber;
		while (*cp) {
			if (isdigit(*cp))
				*wp++ = *cp;
			cp++;

			if (wp - tmpPhone.phoneNumber > kPhoneNumberSize) {
				Logmsg("ERROR: huge phone number '%s'\n",
					contestant->boxPhoneNumber.phoneNumber);
				Matching_Abort();
				return (kInvalidPhoneNumber);
			}
		};
		*wp = '\0';

		// See if it's remotely valid.  Since we have ANI, a number less
		// than 10 characters should never happen, and we don't need the
		// "add 408 to it" kluge here.
		//
		len = strlen(tmpPhone.phoneNumber);
		if (len < 10) {
			Logmsg("REJECT: short phone # '%s'\n",
				contestant->boxPhoneNumber.phoneNumber);
			return (kInvalidPhoneNumber);
		}

		// If it starts with '0', or it starts with '1' and isn't exactly
		// 11 chars long, nuke 'em.
		//
		if (tmpPhone.phoneNumber[0] == '0' ||
			(tmpPhone.phoneNumber[0] == '1' && len != 11))
		{
			Logmsg("REJECT: invalid phone # '%s'\n",
				contestant->boxPhoneNumber.phoneNumber);
			return (kInvalidPhoneNumber);
		}

		// Copy it back, prepending a 1 if necessary.
		//
		if (tmpPhone.phoneNumber[0] != '1')
			sprintf(contestant->boxPhoneNumber.phoneNumber, "1%s",
				tmpPhone.phoneNumber);
		else
			strcpy(contestant->boxPhoneNumber.phoneNumber,
				tmpPhone.phoneNumber);
	}


	// check challengeFlags
	//
	if (contestant->challengeFlags != kMatchInvalid &&
		contestant->challengeFlags != kAcceptChallenges &&
		contestant->challengeFlags != kSpecificChallenge)
	{
		Logmsg("REJECT: invalid challengeFlags %d\n",
			contestant->challengeFlags);
		return (kInvalidArguments);
	}

	// callLongDistance is boolean

	// romID isn't used

	// how to check oppBoxSerialNumber?  (either prev opponent or desired opp)

	// Check the oppPlayer #.  Can be -1 if this is an auto-match and we
	// didn't have a previous opponent.  Might be allowable for a specific
	// if we want to be flexible.
	//
	if (contestant->oppPlayer < 0 || contestant->player > 3) {
		if (contestant->oppPlayer != -1 ||
			contestant->challengeFlags == kSpecificChallenge)
		{
			Logmsg(
				"REJECT: invalid oppPlayer # %d\n", contestant->oppPlayer);
			return (kInvalidArguments);
		}
	}

	// check the rankingInfo
	//
	if (contestant->rankingInfo.gameID != contestant->gameID) {
		Logmsg(
			"REJECT: Ranking info for wrong game (0x%.8lx vs 0x%.8lx)\n",
			contestant->rankingInfo.gameID,
			contestant->gameID);
	}

	return (kNoError);
}

//
// Init the rest of the fields, mainly to 0 or NULL.
//
// This is where the contestantID gets set.
//
int Matching_InitializeContestant(Contestant *contestant)
{
	// assign a serial number; this gets used as the "magic cookie"
	//
	contestant->contestantID = gRegionGlobals.nextContestantID++;

	// zero the rest of the fields that weren't passed in
	//
	contestant->aliasedGameID = 0;
	contestant->queuedWhen = 0;
	contestant->globalListNode = NULL;
	contestant->gameListNode = NULL;
	contestant->flags = kMatchValid;
	contestant->skillLevel = 0;

	contestant->purgatoryWhen = 0;
	contestant->opponentID = 0;

	return (true);
}

//
// Find the MatchingGameData struct for a given GameID.
//
// Returns NULL if the struct couldn't be found.
//
MatchingGameData *Matching_FindGameData(long gameID)
{
	MatchingGameData *mgd;
	ListNode *lnode;
	int sane = 0;

	if (gRegionGlobals.gameData == NULL) {
		Logmsg("Called FindGameData w/o initializing gameData!\n");
		return (NULL);
	}

	// Run through the mgd list until we find it.  If it's an alias for
	// another game, rewind and try again.
	//
	while (1) {
		if (sane++ > 16) {
			// friends don't let friends loop forever
			Logmsg("ERROR: somebody hosed, aborting\n");
			Matching_Abort();
			return (NULL);
		}

		if ((lnode = SearchList(gRegionGlobals.gameData, gameID)) == NULL) {
			Logmsg("ERROR: unable to find game 0x%.8lx [%d]\n",
				gameID, sane);
			return (NULL);
		}
		mgd = (MatchingGameData *)GetListNodeData(lnode);

		if (mgd->gameInfo == NULL) {
			Logmsg("ERROR: mgd w/o gameInfo?\n");
			Matching_Abort();
			return (NULL);
		}
		if (mgd->gameInfo->alias != 0) {
			gameID = mgd->gameInfo->alias;
			continue;
		}

		// eureka!
		break;
	}

	return (mgd);
}

//
// Compare two BoxSerialNumbers.
//
// (Might want to make this a macro or inline.)
//
int Matching_CompareBoxSerial(BoxSerialNumber *box1, BoxSerialNumber *box2)
{
	if ((box1->box == box2->box) && (box1->region == box2->region))
		return (true);
	else
		return (false);
}


// ===========================================================================
//		Phone stuff
// ===========================================================================

//
// Determine whether the caller is close enough to the callee.  Uses
// the caller's callLongDistance flag and the LATA database.
//
// Doing a full LATA lookup against every queued player will
// probably suck.  We need a fast way to strcmp against the
// area code.  Problem is, they may be able to play against
// adjacent area codes, so we need to determine adjacent areas
// once and then deal with it.
//
// OTOH, two prefixes in the same area code might not be close enough...
//
int Matching_DistanceOkay(Contestant *contestant, Contestant *opponent)
{
	if (contestant->callLongDistance)		// right now, it's ALWAYS false
		return (true);
		
#ifdef MATCH_BY_AREA_CODE_ONLY
	// match EXCLUSIVELY by area code
	if (strncmp(contestant->boxPhoneNumber.phoneNumber,
				opponent->boxPhoneNumber.phoneNumber, 4) == 0)
		return (true);
	else
		return (false);
#endif

	// No LD, see if they're close enough.
	//
	// BRAIN DAMAGE: can't use DB routines that won't exist in this process!
	//
	return (DataBase_LataIsLocal(&contestant->boxPhoneNumber,
		&opponent->boxPhoneNumber));
}

//
// Given the box's phone number and the opponent's phone number, tweak
// the opponent number so that it has an area code on it iff he's in a
// different area code than we are.
//
// Assumes the phone numbers have been normalized.
//
int Matching_TweakOpponentPhoneNumber(phoneNumber *oppPhone, const phoneNumber *myPhone)
{
	phoneNumber tmpPhone;

	// If the first 4 chars match (1xxx, where xxx is the area code), then
	// remove the first four from the oppPhone number.
	//
	if (strncmp(myPhone->phoneNumber, oppPhone->phoneNumber, 4) == 0) {
		// Area codes match, so strip it off.
		//
		tmpPhone = *oppPhone;
		strcpy(oppPhone->phoneNumber, tmpPhone.phoneNumber+4);

		if (strlen(oppPhone->phoneNumber) != 7) {
			Logmsg("GLITCH: nuked the opp phone number!  '%s'\n",
				oppPhone->phoneNumber);
			Matching_Abort();
			return (false);
		}
	}

	return (true);
}


// ===========================================================================
//		Queue routines
// ===========================================================================

//
// Add the contestant to the global and game/specific queues.  All fields
// of the contestant structure should be filled out.
//
// Contestant is either a new caller or is coming back from purgatory.
//
// This makes a copy of the contestant, since the original may have come
// up from RPC, in which case it'd be a static struct.  The flags field
// in the original will be set to kMatchCopied to indicate that the
// structure has been duplicated and changes to it will be lost.
//
// Returns TRUE if all succeeded, FALSE on failure.
//
int Matching_AddToQueues(Contestant *contestant, MatchingGameData *mgd)
{
	Contestant *newc;
	ListNode *lnode;

	if (mgd->gameQueue == NULL) {
		Logmsg("ERROR: uninitialized game queue for 0x%.8lx\n",
			contestant->gameID);
		Matching_Abort();
		return (false);
	}

	if (contestant->flags != kMatchValid) {
		Logmsg("WARNING: AddToQueues when not kMatchValid\n");
		// oh, go ahead
	}

	// Make a copy of it for the queues, and invalidate the old.
	//
	newc = (Contestant *)malloc(sizeof(Contestant));
	if (newc == NULL) {
		Logmsg("ERROR: malloc failed in AddToQueues\n");
		return (false);
	}
	memcpy(newc, contestant, sizeof(Contestant));

	contestant->flags = kMatchCopied;	// maybe hose the ID to make it obvious?

	// Add it.  Note we need to create two different list nodes, but they
	// can share the same contestant struct.
	//
	lnode = NewSortedListNode((Ptr) newc, newc->contestantID);
	ASSERT(lnode);
	AddListNodeToSortedList(gRegionGlobals.globalQueue, lnode);
	newc->globalListNode = lnode;

	lnode = NewSortedListNode((Ptr) newc, newc->contestantID);
	ASSERT(lnode);
	if (newc->challengeFlags == kSpecificChallenge)
		AddListNodeToSortedList(gRegionGlobals.specificQueue, lnode);
	else
		AddListNodeToSortedList(mgd->gameQueue, lnode);
	newc->gameListNode = lnode;

	return (true);
}

//
// If this guy (or anybody else from his box) is already on a queue
// somewhere, remove him.
//
// Need to check the purgatory queue as well.
//
// Returns TRUE if it was found and removed, FALSE if not.
//
int Matching_DequeueIfQueued(Contestant *contestant)
{
	ListNode *lnode;
	Contestant *qcon;

	// Scan the global queue.  If he's there, he must've re-registered
	// before getting matched.
	//
	for (lnode = GetFirstListNode(gRegionGlobals.globalQueue); \
		lnode != NULL; lnode = GetNextListNode(lnode))
	{
		qcon = (Contestant *)GetListNodeData(lnode);
		if (Matching_CompareBoxSerial(&contestant->boxSerialNumber,
			&qcon->boxSerialNumber))
		{
			// Found him.  Remove him from the general queue and the
			// game queue.
			//
			if (qcon->globalListNode != lnode) {
				Logmsg(
					"WARNING: funky ListNode mismatch in DequeueIfQueued\n");
				Matching_DumpQueues();
			}
			Logmsg("NOTE: new ID %ld replaces old ID %ld\n",
				contestant->contestantID, qcon->contestantID);

			RemoveListNodeFromList(qcon->globalListNode);
			DisposeListNode(qcon->globalListNode);
			RemoveListNodeFromList(qcon->gameListNode);
			DisposeListNode(qcon->gameListNode);
			free(qcon);

			// update stats
			//
			gRegionGlobals.numRequeued++;
			gRegionGlobals.timeEnqueued += time(0) - qcon->queuedWhen;

			//return (true);
			break;		// (can't be on purgatory list, but check for now)
		}
	}

	// Scan the purgatory list.  Being on here means the user either had a
	// short game, or re-registered while his chosen opponent was dialing.
	//
	for (lnode = GetFirstListNode(gRegionGlobals.purgatoryQueue); \
		lnode != NULL; lnode = GetNextListNode(lnode))
	{
		qcon = (Contestant *)GetListNodeData(lnode);
		if (Matching_CompareBoxSerial(&contestant->boxSerialNumber,
			&qcon->boxSerialNumber))
		{
			// Found him out here in purgatory.  Just nuke this node.
			//
			Logmsg("NOTE: new ID %ld replaces purgatory %ld\n",
				contestant->contestantID, qcon->contestantID);

			RemoveListNodeFromList(lnode);
			DisposeListNode(lnode);
			free(qcon);

			return (true);
		}
	}

	return (false);
}

//
// Look for the guy I want to challenge.  Uses the oppBoxSerialNumber
// and oppPlayer fields to find him.  Look on the global queue, so we'll
// find both the game queues and the specific queue.
//
// Returns NULL if the contestant can't be found.
//
Contestant *Matching_FindMyChallengee(Contestant *contestant)
{
	Contestant *qcon;
	ListNode *lnode;

	for (lnode = GetFirstListNode(gRegionGlobals.globalQueue); \
		lnode != NULL; lnode = GetNextListNode(lnode))
	{
		qcon = (Contestant *)GetListNodeData(lnode);

		if (Matching_CompareBoxSerial(&contestant->oppBoxSerialNumber,
			&qcon->boxSerialNumber))
		{
			if (contestant->oppPlayer == qcon->player)
				return (qcon);

			// somebody else from the same box is logged on... give up now
			return (NULL);
		}
	}

	return (NULL);
}

//
// Look for a guy on the "specific" queue who wants to challenge me.  Returns
// the one who dialed in first.
//
Contestant *Matching_FindMyChallenger(Contestant *contestant)
{
	Contestant *qcon;
	ListNode *lnode;

	for (lnode = GetFirstListNode(gRegionGlobals.specificQueue); \
		lnode != NULL; lnode = GetNextListNode(lnode))
	{
		qcon = (Contestant *)GetListNodeData(lnode);

		if (Matching_CompareBoxSerial(&contestant->boxSerialNumber,
			&qcon->oppBoxSerialNumber))
		{
			if (contestant->player == qcon->oppPlayer)
				return (qcon);

			// He wants somebody else from my box, but maybe someone else
			// is interested in *me*.  Keep looking...
		}
	}

	return (NULL);
}

//
// Used when the contestant's opponent is found.  The opponent goes into
// the "purgatory" list, in case the current contestant chokes and dies
// during the next part of his connection.
//
int Matching_DequeueForMatch(Contestant *opponent, MatchingGameData *mgd)
{
	ListNode *lnode;

	if (opponent == NULL || mgd == NULL) {
		Logmsg("NULL args in DequeueForMatch\n");
		Matching_Abort();
		return (false);
	}
	if (opponent->globalListNode == NULL || opponent->gameListNode == NULL) {
		Logmsg("Trying to dequeue for match but queues not set\n");
		Matching_Abort();
		return (false);
	}

	// throw out the game queue node
	//
	RemoveListNodeFromList(opponent->gameListNode);
	DisposeListNode(opponent->gameListNode);
	opponent->gameListNode = NULL;

	// move the global list node to the purgatory list
	//
	RemoveListNodeFromList(opponent->globalListNode);
	AddListNodeToSortedList(gRegionGlobals.purgatoryQueue,
		opponent->globalListNode);
	opponent->purgatoryWhen = time(0);		// reset his queuedWhen
	opponent->flags = kMatchPurgatory;		// show that he's almost gone

	// update stats
	//
	gRegionGlobals.timeEnqueued += time(0) - opponent->queuedWhen;

	return (true);
}

//
// Expire anybody who has been around for more than kMaxTimeInWaitQueue
// seconds (kMaxTimeInSpecificQueue for specific challenges).
//
int Matching_ExpireContestants(time_t now)
{
	ListNode *lnode;
	Contestant *qcon;

	Logmsg("Expiring contestants\n");

	// start with the global queue
	//
restart_global:
	for (lnode = GetFirstListNode(gRegionGlobals.globalQueue); \
		lnode != NULL; lnode = GetNextListNode(lnode))
	{
		qcon = (Contestant *)GetListNodeData(lnode);

		if ( (qcon->challengeFlags == kSpecificChallenge &&
				(now - qcon->queuedWhen > kMaxTimeInSpecificQueue)) ||
			(now - qcon->queuedWhen > kMaxTimeInWaitQueue))
		{
			ListNode *prevLnode = GetPrevListNode(lnode);

			// boot them
			//
			Logmsg("EXPIRE: removing %ld: '%s' (%s)\n",
				qcon->contestantID, qcon->userName,
				qcon->boxPhoneNumber.phoneNumber);
			Statusmsg("Matcher: Removed '%s' (%s)\n",
				qcon->userName, qcon->boxPhoneNumber.phoneNumber);

			if (qcon->globalListNode != lnode) {
				Logmsg("GLITCH: Oddity in Expire\n");
				Matching_Abort();
			}
			RemoveListNodeFromList(qcon->gameListNode);
			DisposeListNode(qcon->gameListNode);
			RemoveListNodeFromList(qcon->globalListNode);
			DisposeListNode(qcon->globalListNode);
			free(qcon);

			// update stats
			//
			gRegionGlobals.numTimeouts++;
			gRegionGlobals.timeEnqueued += now - qcon->queuedWhen;

			// back up, so GetNext gets us the removed one's next
			lnode = prevLnode;
			if (lnode == NULL) goto restart_global;
		}
	}

	// do the same f