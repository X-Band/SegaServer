/*
	File:		Matching.h

	Contains:	Structures and prototypes for matching stuff

	Written by:	Andy McFadden

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <7>	 9/18/94	ATM		DataBase_List.h --> Common_List.h.
		 <6>	 9/16/94	ATM		Changed DumpContestant prototype.  Added new timeout for
									"specific" queue.
		 <5>	 9/12/94	ATM		Added new control messages.
		 <4>	 9/12/94	ATM		Implemented MatchingControl stuff.
		 <3>	  9/7/94	ATM		Added skillLevel field.
		 <2>	  9/6/94	ATM		Add TweakPhoneNumber.
		 <1>	  9/6/94	ATM		first checked in

	To Do:
*/


#ifndef unix			// not expected to work on a Mac; get over it
#error "This doesn't work on a Mac!"
!!! blah blah blah blah !!!
#else

#include <sys/types.h>

#include "Server.h"
#include "ServerDataBase.h"
#include "ServerCore.h"			// need kMaxTimeInWaitQ
#include "Common_List.h"


//
// Overall structure:
//
//	"auto-match" means a Find Opponent match.
//	"specific match" means a player-list match.
//
//	Matching may be divided into "regions".  Players are never auto-matched
//	against players in other regions, but may do specific matches.  Each
//	region is handled by a separate UNIX process, probably on separate
//	machines (that's the point of having regions... in case we overflow).
//
//	If we split things into regions, then all challenge requests need to
//	go to a separate process, or people can't challenge each other across
//	regions.  OTOH, this hoses the acceptChallenges flag.  We may end up
//	having to search through several regions to verify that the opponent
//	doesn't exist, or just have acceptChallenges cause RPCs to the process
//	that handles specific challenges.  Yuck.
//
//	All state is encapsulated in case we go insane and try to do creation
//	and deletion of regions automatically.
//
//	I expect to be using separate process for SEGA, SNES, etc.  There's
//	no reason to combine them, and it makes the statistics a whole lot
//	more useful if they're separate.
//
//	Addition of a new game is a rare and traumatic event.  Since they can
//	be added to knownGames long before a patch is available, I don't see
//	a need to have them added dynamically.
//
//	Database accesses should be kept to an *absolute* minimum.  I expect
//	these processes to be soaked with matching requests, and blocking on
//	a database lookup is simply unacceptable.  LATA stuff will have to
//	be worked out...
//
//	Each enqueued contestant is placed on two different queues.  The
//	"game" queue (of which the specific match queue is one) is used when
//	searching on a particular game.  The "global" queue contains everyone
//	who is on any queue anywhere; it's used for things like the 10-minute
//	timeout.
//


// After this many seconds have elapsed, the next matching request will
// cause the matcher to look for people have been in the queue or in
// purgatory for too long.
//
#define kMatchScanInterval	20

// The max time in the game/spec queues is specified by kMaxTimeInWaitQ,
// which is in ServerCore.h and is in 1/60ths of a second.  Go figure.
//
#define kMaxTimeInWaitQueue (kMaxTimeInWaitQ/60)

// Max wait time for a specific challenge is longer, because Kon wants
// it that way.  Give 'em an hour.
//
#define kMaxTimeInSpecificQueue (60*60)

// Max time in purgatory.  Give 'em 5 minutes to get things sorted out.
//
#define kMaxTimeInPurgatory	(60*5)


// Contestant.challengeFlags should be one of:
//
enum {
	kIgnoreChallenges = 0,
	kAcceptChallenges,
	kSpecificChallenge
};
typedef long ContestantID;

// Contestant.flags should be one of:
//
enum {
	kMatchInvalid = 0,
	kMatchValid = 1,
	kMatchCopied = 2,
	kMatchPurgatory = 3
};

//
// One of these exists for every person who wants to be matched.
//
typedef struct Contestant {
	// contestant info; this must be passed in
	//
	long			gameID;
	BoxSerialNumber	boxSerialNumber;
	char			player;				// (0-3) player # on box
	UserName		userName;
	phoneNumber		boxPhoneNumber;
	long			challengeFlags;		// enum
	short			callLongDistance;	// true or false
	long			romID;				// "seg6" or whatever; no longer needed?
	BoxSerialNumber	oppBoxSerialNumber;	// serial # of previous opponent, OR
	char			oppPlayer;			//  serial # of desired specific oppnt
	RankingInfo		rankingInfo;		// needed for skill level

	// state info
	//
	ContestantID	contestantID;		// serial #; also used as "magic cookie"
	long			aliasedGameID;		// gameID, but after chasing aliases
	time_t			queuedWhen;			// when put on queue
	ListNode		*globalListNode;	// ptr to ListNode on global queue
	ListNode		*gameListNode;		// ptr to ListNode on game/spec queue
	long			flags;				// used during processing
	long			skillLevel;			// computed; may not be the same as rank

	// purgatory info
	// (after we match two people, but before we forget the queued one entirely)
	//
	time_t			purgatoryWhen;		// when put in purgatory
	ContestantID	opponentID;			// who our opponent was
} Contestant;


//
// This is what we send back.
//
// Note that, if the person is enqueued, we don't send back the userName.
// This means that it won't be available for the "wait for %s to show up"
// dialog that you get when you challenge someone.  This is fine, since
// the caller should already have it.
//
enum {
	// two possible ways to return successfully
	//
	kMatchWait = 0,						// player is on a queue
	kMatchDial							// player needs to dial opponent

	// also uses DatabaseErrors enum from ServerDataBase.h
};

typedef struct Matchup {
	// everybody gets these
	//
	Err				result;				// enum
	long			magicCookie;		// the enchanted Oreo

	// if result is kMatchDial, these are set:
	//
	long			oppMagicCookie;		// the other guy's cookie
	BoxSerialNumber	oppBoxSerialNumber;	// The Enemy
	char			oppPlayer;			// (0-3) The Enemy's player number
	phoneNumber		oppPhoneNumber;		// number to call
	UserName		oppUserName;		// name to display while dialing
} Matchup;


//
// One of these for every game we know about.
//
// (Called "Matching" GameData to differentiate from the various other
// game-related data structures lurking about.)
//
typedef struct MatchingGameData {
	// useful stuff
	//
	long			gameID;				// what game this is for
	GameInfo		*gameInfo;			// pointer to appropriate entry
	ListHead		*gameQueue;			// the per-game queue

	// statistics
	//
	long			numRequests;		// #of matching requests
	long			numSuccesses;		// want == numRequests; +2 per match
} MatchingGameData;



//
// Global values for an entire region.
//
typedef struct RegionGlobals {
	// useful stuff
	//
	long			regionID;			// what region am I
	long			boxArch;			// 'snes', 'sega', etc
	long			nextContestantID;	// serial #s, starting from 0

	// pointers to our data
	//
	ListHead		*globalQueue;		// list of Contestants, sort by conID
										//  everyone registered, !purgatory
	ListHead		*specificQueue;		// list of Contestants, sort by conID
										//  people waiting for specific opps
	ListHead		*purgatoryQueue;	// list of Contestants, sort by conID
										//  all the guys in purgatory
	ListHead		*gameData;			// list of MatchingGameData, sort gameID
										//  game queues are inside MGD struct
	// statistics
	//
	time_t			startTime;			// when these stats were initialized
	long			numConnections;		// #of RPC connects (incl control stuff)
	long			numRequests;		// #of matching requests (either kind)
	long			numSuccesses;		// want this to be == numRequests
	long			numLongDistance;	// #of long-distance matches

	long			numSpecificReq;		// #of specific challenge requests
	long			numSpecificSuc;		// #of specific challenge pairs
	long			numAutoReq;			// #of auto-match requests
	long			numAutoSuc;			// #of auto-match successes
	long			numAutoSpecificSuc;	// #of auto-matches reqs-->specific

	long			numEnqueued;		// #of people put on a queue
	long			numTimeouts;		// #of people booted by timer
	long			numRequeued;		// #of people who re-reg while on queue
	long			numReborn;			// #of people restored from purgatory
	long			timeEnqueued;		// total time spent on queue for all
} RegionGlobals;

//
// Global (to Matching) variables
//
extern RegionGlobals gRegionGlobals;


// ===========================================================================
//		Matching_Control stuff
// ===========================================================================

//
// Input to the Control routine.
//
// The MatchControl type is a union of every possible control message.  All
// of the control messages *must* start with "long type"; that's how we
// tell them apart.  Try to keep the messages brief.
//
// The MatchControlResult is a variable-sized structure like those used
// elsewhere in the server.  The content of the "data" element depends
// entirely on what MatchControl was called with.
//
typedef struct MatchControl_Ping {
	long type;
	// Just see if matcher is alive.
} MatchControl_Ping;

typedef struct MatchControl_Shutdown {
	long type;

	// Shut down and exit.  Use a validation flag to ensure a mis-set type
	// doesn't nuke the whole thing.  For obvious reasons, this doesn't
	// return anything.
	//
	long validate;		// must == kMatchShutdownValidate
} MatchControl_Shutdown;
#define kMatchShutdownValidate	0xD5AAD5AA

typedef struct MatchControl_Reload {
	long type;
	// Reload database files, notably the LATA stuff.
} MatchControl_Reload;

typedef struct MatchControl_DoExpire {
	long type;

	// Causes Matching_ExpireContestants to be called.  If "when" is nonzero,
	// it will be used as the current time (so by passing time + 2 minutes,
	// you can cause everything to expire two minutes fast).
	//
	long when;
} MatchControl_DoExpire;

typedef struct MatchControl_GetContestant {
	long type;

	// Get the Contestant struct for a given ContestantID.
	//
	ContestantID contestantID;
} MatchControl_GetContestant;

typedef struct MatchControl_DequeueContestant {
	long type;

	// Remove a Contestant from the queue.
	//
	ContestantID contestantID;
} MatchControl_DequeueContestant;

typedef struct MatchControl_AddContestant {
	long type;

	// Add a Contestant to the queue.  Bad idea unless you know what you're
	// doing.
	//
	Contestant contestant;
} MatchControl_AddContestant;

typedef struct MatchControl_GetQueues {
	long type;

	// Boolean values for which queues to get.  We *must* get them all
	// in one RPC, or run the risk of inconsistent data.  Of course,
	// UDP RPCs may choke if the data size exceeds 8K...
	//
	short getGlobalQueue;
	short getPurgatoryQueue;
	short getGameQueues;
} MatchControl_GetQueues;

// union of all of the above
//
typedef union MatchControl {
	long type;
	MatchControl_Ping ping;
	MatchControl_Shutdown shutdown;
	MatchControl_Reload reload;
	MatchControl_DoExpire doExpire;
	MatchControl_GetContestant getContestant;
	MatchControl_DequeueContestant dequeueContestant;
	MatchControl_AddContestant addContestant;
	MatchControl_GetQueues getQueues;
} MatchControl;

//
// Return values from the Control routine.  All MatchControl calls cause
// a result to be returned.
//
// As above, the result structure is a union of all possible result
// structures.  In a few cases, a large chunk of data is returned... in
// that case we use the "dangling data" structure seen in various other
// places in the server.
//
// Once again, the first element in each type MUST be "type".  Also, the
// second element in each msut be "size" so that the RPC routines can deal
// with dynamically allocated structures.  (For convenience, size == 0 is
// equivalent to size == sizeof(MatchControlResult).)
//
typedef struct MatchControlResult_Status {
	long type;
	long size;

	// Used for simple calls.  This just returns whether or not the call
	// succeeded (kNoError on success, something else on error).
	//
	Err  result;
} MatchControlResult_Status;

typedef struct MatchControlResult_Contestant {
	long type;
	long size;

	// Return from GetContestant.
	//
	Err  result;
	Contestant contestant;
} MatchControlResult_Contestant;

typedef struct MatchControlResult_QueueContents {
	long type;
	long size;

	// Here follow "size" bytes of data, where "size" is from 0 to 8K.
	// Exact format TBD.
	//
	Err  result;
	unsigned char data[1];
} MatchControlResult_QueueContents;

// union of all of the above
//
typedef union MatchControlResult {
	struct {
		long type;
		long size;
	} generic;
	MatchControlResult_Status status;
	MatchControlResult_Contestant contestant;
	MatchControlResult_QueueContents queueContents;
} MatchControlResult;


// Enumerated types for the "type" field.
//
enum {
	kMatchCtl_Ping,
	kMatchCtl_Shutdown,
	kMatchCtl_Reload,
	kMatchCtl_GetContestant,
	kMatchCtl_DequeueContestant,
	kMatchCtl_AddContestant,
	kMatchCtl_DoExpire,
	kMatchCtl_GetQueues,

	kMatchCtlRes_NoResult,			// NULL pointer
	kMatchCtlRes_Status,
	kMatchCtlRes_Contestant,
	kMatchCtlRes_QueueContents,
};


// ===========================================================================
//		Function prototypes
// ===========================================================================

// Matching_Util.c - public RPC calls
//
#ifdef CLIENTRPC
/*Err RPC_MatchingStartup(void);*/
/*Err RPC_MatchingShutdown(void);*/
Matchup *RPC_FindMatch(Contestant *contestant);
MatchControlResult *RPC_Control(MatchControl *matchControl);
#else
// Matching_Util.c - calls from inside RPC handler
//
Err Matching_MatchingStartup(void);
Err Matching_MatchingShutdown(void);
Matchup *Matching_FindMatch(Contestant *contestant);
MatchControlResult *Matching_MatchingControl(MatchControl *matchControl);
#endif

// Matching_Util.c - internal
//
MatchingGameData *Matching_FindGameData(long gameID);
int Matching_CompareBoxSerial(BoxSerialNumber *box1, BoxSerialNumber *box2);
int Matching_TweakOpponentPhoneNumber(phoneNumber *oppPhone, const phoneNumber *myPhone);
int Matching_DistanceOkay(Contestant *contestant, Contestant *opponent);
int Matching_AddToQueues(Contestant *contestant, MatchingGameData *mgd);
int Matching_DequeueIfQueued(Contestant *contestant);
int Matching_DequeueForMatch(Contestant *opponent, MatchingGameData *mgd);
Contestant *Matching_FindMyChallenger(Contestant *contestant);
Contestant *Matching_FindMyChallengee(Contestant *contestant);
int Matching_ExpireContestants(time_t now);
void Matching_DumpQueues(void);
void Matching_DumpContestant(Contestant *contestant, int brief);
enum { MDC_BRIEF, MDC_VERBOSE };

// Matching_Matching.c - internal
//
int Matching_Specific(Contestant *contestant, Matchup *matchup);
int Matching_AutoMatch(Contestant *contestant, Matchup *matchup);


// This should be abort() for debugging, and blank for production.
//
#define Matching_Abort()	abort()
//#define Matching_Abort()

#endif /*unix*/
