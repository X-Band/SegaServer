/*
	File:		Matching_Matching.c

	Contains:	Routines that do the actual matching

	Written by:	Andy McFadden

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <7>	 9/12/94	ATM		Use aliasedGameID instead of gameID.
		 <6>	  9/7/94	ATM		Set the magicCookie correctly.
		 <5>	  9/7/94	ATM		Minor tweaks.
		 <4>	  9/6/94	ATM		Fix it in the other place, too.
		 <3>	  9/6/94	ATM		You can specific-match against somebody with a different
									cartridge.  Neat.
		 <2>	  9/6/94	ATM		Add TweakPhoneNumber.  Minor fixes.
		 <1>	  9/6/94	ATM		first checked in

	To Do:
*/


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "Matching.h"

// ===========================================================================
//		Prototypes for local routines
// ===========================================================================

int Matching_Matchup(Contestant *contestant, Contestant *opponent,
	Matchup *matchup, MatchingGameData *mgd);
int Matching_Enqueue(Contestant *contestant, Matchup *matchup,
	MatchingGameData *mgd);
int Matching_AutoMatch_FIFO(Contestant *contestant, Matchup *matchup,
	MatchingGameData *mgd);
int Matching_SetValidFlags(Contestant *contestant, MatchingGameData *mgd,
	Contestant **prevOpponent);


// ===========================================================================
//		Specific matches
// ===========================================================================

//
// Entry point for specific matches.
//
// Puts return value in Matchup.result.
//
int Matching_Specific(Contestant *contestant, Matchup *matchup)
{
	Contestant *opponent;
	MatchingGameData *mgd;

	gRegionGlobals.numSpecificReq++;

	// Should we check DistanceOkay here?  Would require the client to
	// pass in the opponent phone number along with his boxSerialNumber.
	// While this makes it so that he can't end up on a queue for a player
	// he won't connect to, it will screw the guy over if his desired
	// challengee has moved and not yet dialed in with "New Number".

	//
	// Find info for this particular game.
	//
	if ((mgd = Matching_FindGameData(contestant->gameID)) == NULL) {
		Logmsg("Can't find game 0x%.8lx in Specific.  Not possible?\n",
			contestant->gameID);
		return (kNoSuchGameID);
	}
	mgd->numRequests++;
	contestant->aliasedGameID = mgd->gameID;

	//
	// Figure out if our opponent is already there.
	//
	opponent = Matching_FindMyChallengee(contestant);

	if (opponent == NULL) {
		// Not there, put me on the queue and wait for him to show up.
		//

		// Fun case: what if he's already been challenged by someone
		// else?  There's really nothing we can do, except tell the
		// user that they won't get matched unless the other guy challenges
		// back.  Question is, is it worth the CPU time to check...?

		Matching_Enqueue(contestant, matchup, mgd);
		return (false);

	} else {
		// We're in business, unless he's trying to challenge someone
		// else, is using a different cartridge, or has acceptChallenges off.
		//
		if (contestant->aliasedGameID != opponent->aliasedGameID) {
			matchup->result = kChallengeeWaitingForDifferentGame;
			return (false);
		}

		if (opponent->challengeFlags == kSpecificChallenge) {
			// Other guy wants to challenge someone specific.  Is it me?
			//
			// Okay to just compare box serial numbers.  We only get here
			// if both players have specifically challenged each other, so
			// if one side wants to change players, that's fine.  We could
			// be anal and block it, but I don't see the value.
			//
			if (Matching_CompareBoxSerial(&contestant->boxSerialNumber,
				&opponent->oppBoxSerialNumber))
			{
				// we challenged each other, I dial!
				//
				if (Matching_DistanceOkay(contestant, opponent)) {
					Matching_Matchup(contestant, opponent, matchup, mgd);
					return (true);
				}
				matchup->result = kChallengeeTooFar;
				return (false);
			} else {
				// I want him, he doesn't want me, I fail
				matchup->result = kChallengeeWaitingForDifferentUser;
				return (false);
			}

		} else if (opponent->challengeFlags == kAcceptChallenges) {
			// Other guy is auto, but willing to accept challenges.  In we go!
			//
			if (Matching_DistanceOkay(contestant, opponent)) {
				Matching_Matchup(contestant, opponent, matchup, mgd);
				return (true);
			}
			matchup->result = kChallengeeTooFar;
			return (false);
		} else {
			// Other guy is auto-matched and not accepting challenges.  Fail.
			//
			matchup->result = kChallengeeWaitingForAutoMatch;
			return (false);
		}
	}
}


// ===========================================================================
//		Auto-matches
// ===========================================================================

//
// Entry point for auto-matches.
//
// Puts return value in Matchup.result.
//
int Matching_AutoMatch(Contestant *contestant, Matchup *matchup)
{
	Contestant *opponent;
	MatchingGameData *mgd;

	gRegionGlobals.numAutoReq++;

	//
	// Find info for this particular game.
	//
	if ((mgd = Matching_FindGameData(contestant->gameID)) == NULL) {
		Logmsg("Can't find game 0x%.8lx in AutoMatch.  Not possible?\n",
			contestant->gameID);
		return (kNoSuchGameID);
	}
	mgd->numRequests++;
	contestant->aliasedGameID = mgd->gameID;

	//
	// First, check the challengeFlags and look for challenges.
	//
	if (contestant->challengeFlags == kAcceptChallenges) {
		opponent = Matching_FindMyChallenger(contestant);

		if (opponent != NULL) {
			// Somebody wants me!  Make sure they're playing the same
			// game, and are within calling range.
			//
			if (contestant->aliasedGameID == opponent->aliasedGameID) {
				if (Matching_DistanceOkay(contestant, opponent)) {
					Matching_Matchup(contestant, opponent, matchup, mgd);
					return (true);
				}
			}
			// We've rejected the challenger, who will eventually time out.
			Logmsg("NOTE: challenge %ld-->%ld ignored because LD call\n",
				opponent->contestantID, contestant->contestantID);
		}

		// no challengers, continue on...
	}

	//
	// If the game queue is empty, we have few options.
	//
	if (NumListNodesInList(mgd->gameQueue) == 0) {
		Matching_Enqueue(contestant, matchup, mgd);
		return (false);
	}

	//
	// What happens next is policy-dependent, so call a subroutine that
	// implements a particular matching policy.
	//
	return (Matching_AutoMatch_FIFO(contestant, matchup, mgd));
}

//
// Very simple matching policy: match the newcomer with the first
// person he can legally play against.
//
int Matching_AutoMatch_FIFO(Contestant *contestant, Matchup *matchup,
	MatchingGameData *mgd)
{
	Contestant *prevOpponent, *qcon;
	ListNode *lnode;

	if (Matching_SetValidFlags(contestant, mgd, &prevOpponent) == false)
		return (false);

	// For this simple policy, it's okay to match with your previous opponent.
	//
	if (prevOpponent) {
		Logmsg("Found previous opponent on list; marking valid\n");
		prevOpponent->flags = kMatchValid;
	}

	// Find the first guy with the "valid" flag set.
	//
	for (lnode = GetFirstListNode(mgd->gameQueue); lnode != NULL; \
		lnode = GetNextListNode(lnode))
	{
		qcon = (Contestant *)GetListNodeData(lnode);
		if (qcon == NULL) {
			// SetValidFlags should've already verified this!
			Logmsg("ERROR: null list data, I give up\n");
			Matching_Abort();
			return (false);
		}

		if (qcon->flags == kMatchInvalid)
			continue;

		// Gotcha.
		//
		Matching_Matchup(contestant, qcon, matchup, mgd);
		return (true);
	}

	// Nobody to play, queue him.
	//
	Matching_Enqueue(contestant, matchup, mgd);
	return (true);
}


// ===========================================================================
//		Utility routines
// ===========================================================================

//
// Initialize the "flags" field in the queued Contestants for the game queue.
//
// If, for the specified contestant, the queued guy is:
//	- in a valid calling range
//	- not the last person this guy played
//	- (whatever else I can think of)
// then mark the entry as "valid".  Otherwise, mark it as "invalid".
//
// We could do this by juggling lists, which might make a later operation
// faster, but malloc() and memcpy() calls should be avoided whenever
// possible, and shuffling list nodes around will not be very cheap.
//
// If the previous opponent *is* found, then we set a pointer to him.  Some
// strategies may want to match previous fighters if the queued one has
// been on for a while.
//
// There is no reason to call this routine for a challenge request.
//
int Matching_SetValidFlags(Contestant *contestant, MatchingGameData *mgd,
	Contestant **prevOpponent)
{
	ListNode *lnode;
	Contestant *qcon;

	*prevOpponent = NULL;

	if (contestant->challengeFlags == kSpecificChallenge) {
		Logmsg("Why are you calling Matching_SetValidFlags?\n");
		Matching_Abort();
		return (false);
	}
	if (mgd->gameQueue == NULL) {
		Logmsg("In SVF with an invalid gameQueue for 0x%.8lx\n",
			mgd->gameID);
		Matching_Abort();
		return (false);
	}

	// Run through every contestant on the game queue.
	//
retry:
	for (lnode = GetFirstListNode(mgd->gameQueue); lnode != NULL; \
		lnode = GetNextListNode(lnode))
	{
		qcon = (Contestant *)GetListNodeData(lnode);
		if (qcon == NULL) {
			Logmsg("No data associated with a queued node!\n");
			Matching_Abort();
			// try to recover
			if (RemoveListNodeFromList(lnode) != kNoError)
				return (false);
			if (DisposeListNode(lnode) != kNoError)
				return (false);
			goto retry;
		}

		qcon->flags = kMatchValid;		// everybody starts out this way

		// If he can't call long distance, then remove anyone who is too far.
		//
		if (!contestant->callLongDistance) {
			// Doing a full LATA lookup against every queued player will
			// probably suck.  Assume Matching_DistanceOkay() operates
			// in an efficient manner.
			if (!Matching_DistanceOkay(contestant, qcon)) {
				qcon->flags = kMatchInvalid;
				continue;
			}
		}

		// If we haven't found his previous opponent yet, check if this is it.
		// For auto-requests, the previous opponent gets passed in via
		// the "opp" fields in Contestant.
		//
		if (*prevOpponent == NULL &&
			Matching_CompareBoxSerial(&contestant->oppBoxSerialNumber,
				&qcon->boxSerialNumber) &&
			contestant->oppPlayer == qcon->player)
		{
			*prevOpponent = qcon;
			qcon->flags = kMatchInvalid;
			continue;
		}

		// Looks like this one was valid.  Keep going...
	}

	return (true);
}

//
// Put this player on the appropriate queues.
//
int Matching_Enqueue(Contestant *contestant, Matchup *matchup,
	MatchingGameData *mgd)
{
	Logmsg("ENQUEUE: %ld (%s) for %s",
		contestant->contestantID, contestant->userName,
		mgd->gameInfo->gameName);
	if (contestant->challengeFlags == kSpecificChallenge) {
		Logmsg(" (spec for (%ld,%ld)[%ld])\n",
			contestant->oppBoxSerialNumber.box,
			contestant->oppBoxSerialNumber.region,
			contestant->oppPlayer);
	} else {
		Logmsg(" (auto queue size %ld)\n",
			NumListNodesInList(mgd->gameQueue));
	}

	// Fill out the queue-related fields in the "Contestant" structure.
	//
	contestant->queuedWhen = time(0);
	//matchup->result = kOpponentNotRegistered;		// should we do this?
	matchup->result = kMatchWait;
	matchup->magicCookie = contestant->contestantID;

	// Add to queues.
	//
	Matching_AddToQueues(contestant, mgd);

	// (contestant is now INVALID; the copy on the queue is now the real one)

	// Update the global "queue" statistics.
	//
	gRegionGlobals.numEnqueued++;

	return (true);
}


//
// Match two people together.  This is called for the person who will dial.
//
// (The MatchingGameData is only passed in to update the success statistics.)
//
int Matching_Matchup(Contestant *contestant, Contestant *opponent,
	Matchup *matchup, MatchingGameData *mgd)
{
	if (contestant == NULL || opponent == NULL || matchup == NULL || mgd == NULL) {
		Logmsg("NULL args to Matching_Matchup, aborting.\n");
		Matching_Abort();
		return (false);
	}

	// Fill out the Matchup structure.
	//
	matchup->result = kMatchDial;
	matchup->magicCookie = contestant->contestantID;

	matchup->oppMagicCookie = opponent->contestantID;
	matchup->oppBoxSerialNumber = opponent->boxSerialNumber;
	matchup->oppPlayer = opponent->player;
	matchup->oppPhoneNumber = opponent->boxPhoneNumber;
	Matching_TweakOpponentPhoneNumber(&matchup->oppPhoneNumber,
		&contestant->boxPhoneNumber);		// remove the area code, if local
	strcpy(matchup->oppUserName, opponent->userName);

	// Pull the opponent out of the queue.
	//
	Matching_DequeueForMatch(opponent, mgd);

	// Update the global and game "success" statistics.
	//
	mgd->numSuccesses += 2;							// both: succeeded

	Logmsg("MATCH: %ld (%s) with %ld (%s)",
		contestant->contestantID, contestant->userName,
		opponent->contestantID, opponent->userName);

	switch (contestant->challengeFlags) {
	case kIgnoreChallenges:							// both: successful auto
		gRegionGlobals.numAutoSuc += 2;
		Logmsg(" auto/auto");
		break;

	case kAcceptChallenges:
		gRegionGlobals.numAutoSuc++;				// me: successful auto

		switch (opponent->challengeFlags) {
		case kIgnoreChallenges:
		case kAcceptChallenges:
			gRegionGlobals.numAutoSuc++;			// him: successful auto
			Logmsg(" auto/auto");
			break;
		case kSpecificChallenge:
			gRegionGlobals.numAutoSpecificSuc++;	// him: successful specific
			Logmsg(" auto/spec");
			break;
		}
		break;

	case kSpecificChallenge:
		switch (opponent->challengeFlags) {
		case kIgnoreChallenges:
			// this should be impossible
			Logmsg("Impossible!\n");
			break;
		case kAcceptChallenges:
			gRegionGlobals.numAutoSpecificSuc++;	// me: successful specific
			gRegionGlobals.numAutoSuc++;			// him: successful auto
			Logmsg(" spec/auto");
			break;
		case kSpecificChallenge:
			gRegionGlobals.numSpecificSuc += 2;		// both: successful specific
			Logmsg(" spec/spec");
			break;
		}
		break;

	default:
		Logmsg(" - Bogus challengeFlags %ld in Matchup!\n",
			contestant->challengeFlags);
		break;
	}
	Logmsg("\n");

	// should update numLongDistance somehow

	return (true);
}

