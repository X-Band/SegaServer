/*
	File:		ServerDataBase.h

	Contains:	xxx put contents here xxx

	Written by:	David Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<64>	 9/18/94	ATM		Added numRankingInfo field.
		<63>	 9/17/94	ATM		Changed prototype for CompareStrings.
		<62>	 9/16/94	ATM		Allocated box debug1 as "9," flag.
		<61>	 9/16/94	ATM		boxFlags->boxModified, playerFlags->playerModified,
									acceptChallenges->playerFlags.  Added boxFlags.
		<60>	 9/15/94	DJ		added comments describing the use of the debug fields for box
									and player account
		<59>	 9/13/94	ATM		Changed to ReloadPhoneStuff.  Removed LataLookup.
		<58>	 9/12/94	DJ		added Compuserve POP lookup
		<57>	 9/12/94	ATM		Changed GameInfo prototypes to Common_.
		<56>	  9/7/94	ATM		phoneNumber result buf no longer passed to FindAccount,
									CreateAccount, and CreateBoxAccount.
		<55>	  9/6/94	ATM		Complete overhaul of matching system.
		<54>	  9/3/94	ATM		Added GameInfo stuff and xpoints field.
		<53>	  9/2/94	DJ		added mailInBox to track serial numbers of mail on the box
		<52>	 8/30/94	DJ		SPOOGEBALL around old UpdateUserPhoneNumber RPC stuff
		<51>	 8/30/94	DJ		fixed prototype for Database_UpdateUserPhoneNumber
		<50>	 8/30/94	ATM		Changed account flags constants.
		<49>	 8/26/94	ATM		Overhaul of FindAccount and CreateAccount.
		<48>	 8/25/94	ATM		Added boxFlags and accountFlags.
		<47>	 8/25/94	DJ		added passwd to boxAccount
		<46>	 8/25/94	ATM		Added "your ROMs are out of whack" constant
		<45>	 8/25/94	ATM		boxType and acceptChallenges stuff.
		<44>	 8/24/94	ATM		Added reload stuff.
		<43>	 8/22/94	ATM		Changed proto for FindGameOpponentByRank.
		<42>	 8/22/94	ATM		Fiddle faddle.
		<41>	 8/22/94	ATM		Added ServerNetErrorRecord.
		<40>	 8/21/94	ATM		Revised some more.  Added altPopPhone last time, too.
		<39>	 8/21/94	ATM		Major revisions to BoxAccount and PlayerAccount.
		<38>	 8/20/94	ATM		De-RPC NewGame and AddGamePatch.
		<37>	 8/19/94	ATM		De-RPC UpdateRankings.
		<36>	 8/19/94	ATM		Added ranking and connect records to PlayerAccount and
									BoxAccount (in a limited, klugy sort of way).
		<35>	 8/18/94	DJ		kchallengeewaitingforautomatch
		<34>	 8/18/94	ATM		Added update flags for credits.
		<33>	 8/18/94	ATM		Added some new structures, updated BoxAccount.
		<32>	 8/12/94	DJ		prindwaitq
		<31>	 8/12/94	ATM		Added prototype for DataBase_LataLookup.
		<30>	 8/10/94	ATM		Changed GetCurrentTime return value to unsigned.
		<29>	 8/10/94	ATM		Added area code munging stuff.
		<28>	 8/10/94	ATM		Added homeTown to BoxAccount.
		<27>	  8/9/94	DJ		added 2 more errors
		<26>	  8/9/94	DJ		make errors not based on kNumErrors
		<25>	  8/6/94	ATM		Changed CreateAccount and UpdateUserHandle prototypes for
									uniquifier.
		<24>	  8/5/94	ATM		Fixed the silly 1L shift shit.
		<23>	  8/5/94	ATM		Fixed phoneNumber and 1L stuff in shifts.
		<22>	  8/5/94	ATM		Added Account and BoxAccount.
		<21>	  8/5/94	ATM		Added PlayerAccount stuff.
		<20>	 7/29/94	BET		Overload the names of the RPC client side
		<19>	 7/25/94	DJ		5 days of major hacking, mostly consisting of redoing the waitqs
									to support  competitive challenges and waitq aging
		<18>	 7/20/94	DJ		no ServerState passed to dbase routines
		<17>	 7/19/94	DJ		gameName is now just a char*
		<16>	 7/18/94	DJ		magicCookie to AddToWaitList and FindOpponent
		<15>	 7/13/94	DJ		added DataBase_GetGameName
		<14>	 7/12/94	DJ		updated Database_GenerateUniqueBoxSerialNumber to new boxsernum
									formalt
		<13>	  7/8/94	DJ		added send box programming msgs or something
		<12>	  7/7/94	DJ		added Database_UpdateUserPhoneNumber
		<11>	  7/6/94	DJ		address book validation
		<10>	  7/2/94	DJ		GameResult struct for rankings
		 <9>	 6/30/94	DJ		broadcast mail
		 <8>	 6/30/94	DJ		a new error for system patch adding
		 <7>	 6/11/94	DJ		split out errors from Errors.h.  also added a bunch of functions
									for account creation and finding
		 <6>	  6/5/94	DJ		deleting sent mail
		 <5>	  6/2/94	BET		News
		 <4>	 5/31/94	DJ		mail handling apis
		 <3>	 5/29/94	DJ		tweaks to apis
		 <2>	 5/29/94	DJ		added KoolStuff
		 <1>	 5/27/94	DJ		first checked in

	To Do:
*/

#ifndef __ServerDataBase__
#define __ServerDataBase__

#include "Server.h"
#include "UsrConfg.h"
#include "BoxSer.h"	// for the Results struct.
#include "Mail.h"
#include "News.h"
#include "GameDB.h"		// for gameName
#include "Errors.h"
#include "PlayerDB.h"

enum DatabaseErrors {
	kInvalidUserID 			= -2000,
	kNoSuchUserAccount,
	kNoSuchGameID,
	kDuplicateGameID,
	kPatchVersionNotHigher,
	kAccountInvalid,
	kAddingSystemPatchIsFucked,
	kAccountAlreadyExists,
	kChallengeeWaitingForDifferentGame,
	kChallengeeWaitingForDifferentUser,
	kChallengeeNotAvailable,
	kChallengeeWaitingForAutoMatch,
	kChallengeeTooFar,
	kDifferentRomVersion,
	kOpponentNotRegistered,
	kNoSuchBoxAccount,
	kInvalidArguments,
	kInvalidPhoneNumber,
	
	kUnexpectedEOF,
	kUnexpectedCode

};


/*
 * When building the client side RPC calls, we overload the definitions
 * and change the names.  The following disgusting hack achieves this
 * without the use of prototypes.  clientstubs.c also includes this
 * file and hence we don't even have to change the names of the functions
 * which overload the real routines present only in the server.
 * Only those functions that are overload are to be listed here.  Be
 * careful, not all of them are done this way.
 */
#ifdef	CLIENTRPC
#define	ServerDataBase_Initialize RPC_Initialize
#define	ServerDataBase_Shutdown RPC_Shutdown
#define	ServerDataBase_SaveToDisk RPC_SaveToDisk
#define	Database_FindAccount RPC_FindAccount
#define	Database_CreateAccount RPC_CreateAccount
#define	Database_GenerateUniqueBoxSerialNumber RPC_GenerateUniqueBoxSerialNumber
#define	Database_GetTimeStamp RPC_GetTimeStamp
#define	Database_IncrementTimeStamp RPC_IncrementTimeStamp
#define	Database_UpdateUserHandle RPC_UpdateUserHandle
//#define	Database_UpdateUserPhoneNumber RPC_UpdateUserPhoneNumber
#define	DataBase_FindUserIdentification RPC_FindUserIdentification
#define	Database_FindPlayerInfo RPC_FindPlayerInfo
#define	DataBase_GetLatestSystemVersion RPC_GetLatestSystemVersion
#define	DataBase_GetLatestSystemKeyframeVersion RPC_GetLatestSystemKeyframeVersion
#define	DataBase_GetSystemNumPatches RPC_GetSystemNumPatches
#define	DataBase_GetSystemPatch RPC_GetSystemPatch
#define	DataBase_AddSystemPatch RPC_AddSystemPatch
#define	DataBase_GetKoolStuff RPC_GetKoolStuff
#define	DataBase_GetNewsPage RPC_GetNewsPage
#define	DataBase_GetNumNewsPages RPC_GetNumNewsPages
#define	DataBase_GetLatestNGPVersion RPC_GetLatestNGPVersion
#define	DataBase_GetLatestNGP RPC_GetLatestNGP
#define	DataBase_GetLatestGameVersion RPC_GetLatestGameVersion
#define	DataBase_GetGamePatch RPC_GetGamePatch
#define	DataBase_GetGameName RPC_GetGameName
//#define	DataBase_NewGame RPC_NewGame
//#define	DataBase_AddGamePatch RPC_AddGamePatch
#define	DataBase_AddGameResult RPC_AddGameResult
//#define	DataBase_UpdateRanking RPC_UpdateRanking
//#define	DataBase_FindGameOpponentByRank RPC_FindGameOpponentByRank
//#define	DataBase_AddOpponentToGameWaitList RPC_AddOpponentToGameWaitList
//#define	DataBase_RemoveOpponentFromGameWaitList RPC_RemoveOpponentFromGameWaitList
//#define	DataBase_AddOpponentToChallengeWaitList RPC_AddOpponentToChallengeWaitList
//#define	DataBase_FindChallengeOpponent RPC_FindChallengeOpponent
//#define	DataBase_AddOpponentToGeneralWaitList RPC_AddOpponentToGeneralWaitList
//#define	DataBase_RemoveOpponentFromGeneralWaitList RPC_RemoveOpponentFromGeneralWaitList
#define	DataBase_AddMailToIncoming RPC_AddMailToIncoming
#define	DataBase_AddMailToBroadcast RPC_AddMailToBroadcast
#define	DataBase_GetNumIncomingMail RPC_GetNumIncomingMail
#define	DataBase_GetNumBroadcastMail RPC_GetNumBroadcastMail
#define	DataBase_GetIncomingMail RPC_GetIncomingMail
#define	DataBase_GetBroadcastMail RPC_GetBroadcastMail
#define	DataBase_MarkMailAsSent RPC_MarkMailAsSent
#define	DataBase_RemoveSentMail RPC_RemoveSentMail
#define	DataBase_AgeWaitQ RPC_AgeWaitQ
#define DataBase_GetAccount RPC_GetAccount
#define DataBase_UpdateAccount RPC_UpdateAccount
//#define DataBase_LataLookup RPC_LataLookup	/* this is the cheezy old stuff */
#define DataBase_POPLookup	RPC_POPLookup	/* this is the real Compuserve POP lookup */
#define DataBase_Reload RPC_Reload
#endif

//
// Boot the database
//
Err ServerDataBase_Initialize(void);
Err ServerDataBase_Shutdown(void);
void ServerDataBase_SaveToDisk(void);

//
// Server calls this to release any message data that is returned by DB queries.
//
void DataBase_ReleasePreformedMessage(PreformedMessage *msg);
void DataBase_ReleaseServerNewsPage(ServerNewsPage *page);



//
// Get and update player account struct
//

// One of these for each connect; pointer in BoxAccount
typedef struct ConnectInfo {
	long				date;			// unix time(2) format
	unsigned char		playerNumber;	// 0-3
	unsigned char		connectType;	// as per Challnge.h
	BoxSerialNumber		opponentBox;	// for challenge requests only
	long				numMailSent;	// ?? want messages themselves?
	long				numMailRcvd;	// ??
	long				previousGameID;	// results from previous game
	GameResult			previousGameResults;	// actual results; should be DB reference
	GameResult			otherGameResults;	// crash record; should be DB reference
	//NetErrorRecord		netErrors;	// put back when RPC is gone
} ConnectInfo;

// A fatter form of NetErrorRecord, for keeping a long running total.
typedef struct ServerNetErrorRecord {
	unsigned long			serverConnects;				// may be redundant if cleared each server contact
	unsigned long			peerConnects;				// may be redundant if cleared each server contact
	unsigned long			framingError;
	unsigned long			overrunError;
	unsigned long			packetError;
	unsigned long			callWaitingInterrupt;
	unsigned long			noDialtoneError;
	unsigned long			serverBusyError;
	unsigned long			peerBusyError;
	unsigned long			serverDisconnectError;
	unsigned long			peerDisconnectError;
	unsigned long			serverAbortError;			// if we were connected to the server and asked for abort
	unsigned long			peerAbortError;				
	unsigned long			serverNoAnswerError;		// Originate timed out
	unsigned long			peerNoAnswerError;			// Originate timed out
	unsigned long			serverHandshakeError;		// Connection started but failed.
	unsigned long			peerHandshakeError;			// Connection started but failed.
	unsigned long			serverX25NoServiceError;	// if there are no circuits available
	unsigned long			callWaitingError;			// we got call waiting during connection
	unsigned long			remoteCallWaitingError;		// remote modem got call waiting
	unsigned long			scriptLoginError;			// selected script flaked out
} ServerNetErrorRecord; 


// One of these for every day of the week (or one for Mon-Fri and one for
// Sat-Sun).  Contained in BoxAccount.
typedef struct Restriction {		// need to define what these mean
	short start;
	short end;
} Restriction;

// Stuff about the box; contained in Account.
typedef struct BoxAccount {
	unsigned long		magicID;			// temporary kluge
	BoxSerialNumber		box;
	Password			password;			// used by the server to disable the box when no credits in account.
	Hometown			homeTown;
	phoneNumber			gamePhone;
	phoneNumber			popPhone;
	phoneNumber			altPopPhone;
	unsigned long		gamePlatform;
	unsigned long		authCode;			// ??
	unsigned long		userCredits;		// credits the user paid for
	unsigned long		freeCredits;		// credits we gave away
	Restriction			restrictInfo[7];	// one for each day of the week
	unsigned long		boxFlags;			// stuff like local/LD
	short				osLength;			// what IS this?
	long				osVersion;
	long				ngpVersion;
	ConnectInfo			connections[1];		// BRAIN DAMAGE; should be *
	ServerNetErrorRecord netErrorTotals;
	unsigned long		debug0;				// temporary debugging flotsam
	unsigned long		debug1;
	unsigned long		debug2;
	unsigned long		debug3;
} BoxAccount;

// boxFlags
#define kBoxFlags_dialLongDistance	(1L)	// 0=local only, 1=LD okay

// Address book entries.  Array contained in PlayerAccount.
typedef struct AddressBook {
	unsigned char		serverUniqueID;	// 0-9 ?
	BoxSerialNumber		box;
	unsigned char		playerNumber;	// 0-3
	unsigned long		lastPlayed;		// SEGA date: when last played
	unsigned long		wins;
	unsigned long		losses;
} AddressBook;

// One of these for every cartridge played on the box; pointer in PlayerAccount
typedef struct RankingInfo {
	long				gameID;
	long				wins;
	long				losses;
	long				pointsFor;
	long				pointsAgainst;
	long				xpoints;		// XBAND points
} RankingInfo;

// All the relevant information about a specific player on a box.  Contained
// in Account.
typedef struct PlayerAccount {
	unsigned long		magicID;			// used for kluge
	UserName			userName;
	short				iconID;
	short				colorTableID;
	char				*openTaunt;
	char				*personInfo;
	Password			password;
	unsigned long		birthday;
	unsigned long		playerFlags;		// stuff like acceptChallenges
	unsigned long		autoMatchConnects;
	unsigned long		challengeConnects;
	unsigned long		msgCheckConnects;
	unsigned long		failedServerConnects;
	unsigned long		failedClientConnects;
	short				numRankingInfo;		// #of elements in rankingInfo
	RankingInfo			rankingInfo[2];		// BRAIN DAMAGE; should be *
	AddressBook			addressBook[kMaxAddressBookEntries];
	unsigned long		debug0;				// temporary debugging flotsam
	unsigned long		debug1;
	unsigned long		debug2;
	unsigned long		debug3;
	short				mailInBox[kMaxInBoxEntries];	// track which mail items are on the box.
	long				customIconSize;
	unsigned char *		customIcon;			// this better come last (RPC)
	// char				*msgState
	// msgChannel		*msgChannels
} PlayerAccount;

// playerFlags
#define kPlayerFlags_acceptChallenges	(1L)

// Account is the box info plus info on one of the four players
typedef struct Account {
	BoxAccount			boxAccount;
	PlayerAccount		playerAccount;

	long				boxModified;
	long				playerModified;
} Account;

// API routines
Account *DataBase_GetAccount(const userIdentification *userID);
Err DataBase_UpdateAccount(const Account *account);
Err DataBase_FreeAccount(Account *account);


// definition of bit fields for Account->boxModified and playerModified
#define kPA_none					(0L)
#define kPA_magicID					(1L)
#define kPA_userName				(1L<<1)
#define kPA_iconID					(1L<<2)
#define kPA_openTaunt				(1L<<3)
#define kPA_colorTableID			(1L<<4)
#define kPA_personInfo				(1L<<5)
#define kPA_password				(1L<<6)
#define kPA_birthday				(1L<<7)
#define kPA_msgState				(1L<<8)
#define kPA_msgChannels				(1L<<9)
#define kPA_autoMatchConnects		(1L<<10)
#define kPA_challengeConnects		(1L<<11)
#define kPA_msgCheckConnects		(1L<<12)
#define kPA_failedServerConnects	(1L<<13)
#define kPA_failedClientConnects	(1L<<14)
#define kPA_ranking					(1L<<15)		// numRankInf and rankInf
#define kPA_addressBook				(1L<<16)		// all 10 entries
#define kPA_playerFlags				(1L<<17)
#define kPA_customIcon				(1L<<18)		// customIconSize, customIcon
#define kPA_mailInBox				(1L<<19)		// all 10 entries
//#define kPA_all						(~0L)

#define kPA_debug					(1L<<31)
//
// This is the list of debug fields that are used in the PlayerAccount:
//	debug0: used for redial loops.  set by mail to "redial".
//	debug1: unused.
//	debug2: unused.
//	debug3: unused.

#define kBA_none					(0L)
#define kBA_box						(1L)
#define kBA_homeTown				(1L<<1)
#define kBA_gamePhone				(1L<<2)
#define kBA_popPhone				(1L<<3)		// includes altPopPhone
#define kBA_gamePlatform			(1L<<4)
#define kBA_authCode				(1L<<5)
#define kBA_restrictInfo			(1L<<6)
#define kBA_osVersion				(1L<<7)		// does osLength as well
#define kBA_ngpVersion				(1L<<8)
#define kBA_gameInfo				(1L<<9)
#define kBA_credits					(1L<<10)
#define kBA_password				(1L<<11)
#define kBA_boxFlags				(1L<<12)
//#define kBA_all						(~0L)
// no entry for netErrorTotals; assume it gets updated every time

#define kBA_debug					(1L<<31)
//
// This is the list of debug fields that are used in the BoxAccount:
//	debug0: (has something to do with boxes being turned off)
//	debug1: set to 1 if the "dial 9 to get out" hack is in place
//	debug2: unused.
//	debug3: unused.



//
// Check the validity of the account.
//

//typedef struct UserAccountInfo {
//	userIdentification	userID;
//	phoneNumber			boxPhoneNumber;
//} UserAccountInfo;

Err Database_FindAccount(const userIdentification *userID, Account **account);
Err Database_CreateAccount(userIdentification *userID, long *result, Account **account);
void Database_GenerateUniqueBoxSerialNumber(BoxSerialNumber *boxSerialNumber);
long Database_GetTimeStamp(void);
long Database_IncrementTimeStamp(void);

Err Database_UpdateUserHandle(userIdentification *userID, long *result);
#ifdef SPOOGEBALL
Err Database_UpdateUserPhoneNumber(const userIdentification *userID, const phoneNumber *boxPhoneNumber);
#else
Err Database_UpdateUserPhoneNumber(Account *account, const phoneNumber *boxPhoneNumber);
#endif

// result codes for CreateAccount and UpdateUserHandle
#define kUserNameUniquified		1L

// gets the latest userIdentification for the user account.
Err DataBase_FindUserIdentification(const userIdentification *userID, userIdentification *updatedUserID);


//
// Find player info for address book validation.
//

PlayerInfo *Database_FindPlayerInfo(const userIdentification *playerID, const userIdentification *userID);
void Database_ReleasePlayerInfo(PlayerInfo *playerInfo);

//
// Update the system version and required patches.
//
long DataBase_GetLatestSystemVersion(void);
long DataBase_GetLatestSystemKeyframeVersion(void);
long DataBase_GetSystemNumPatches(long version);
PreformedMessage *DataBase_GetSystemPatch(long version, long patchNum);
Err DataBase_AddSystemPatch(long version, long patchNum, PreformedMessage *patch);


//
// Send kool stuff to the box (news, stats, scores, ads, etc).
//
PreformedMessage *DataBase_GetKoolStuff(void);

//
// Send news to the box
//
ServerNewsPage *DataBase_GetNewsPage(long pageNum, DBType pagetype);
long DataBase_GetNumNewsPages(DBType pagetype);

//
// Update the NGPVersion information if not up to date.
//
long DataBase_GetLatestNGPVersion(void);
PreformedMessage *DataBase_GetLatestNGP(void);


//
// Game Info
//
#define kMaxNumRanks    16	// arbitrary limit; box doesn't know or care

typedef char GameName[kGameNameSize];
#define kGameRankSize	34	// seems like a nice number; box doesn't care
typedef char RankName[kGameRankSize];

typedef struct GameRankInfo {
	long	xpoints;		// #of XBAND points needed to reach this level
	RankName rankName;		//  (first "xpoints" in list must be zero!)
} GameRankInfo;

typedef struct GameInfo {
	// basic info
	//
	long	gameID;
	GameName gameName;
	long	patchVersion;	// set by DataBase_SetupGamePatches; don't save
	long	alias;			// if set, this game is an alias for some other game

	// ranking info
	//
	long	numRanks;		// #of different ranks
	DBID	rankID;			// DBID of ranking strings for this game
	GameRankInfo ranks[kMaxNumRanks];	// ranking strings for this game
} GameInfo;

// functions to get at it
//
GameInfo *Common_GetGameInfo(long *numGames);
void Common_FreeGameInfo(GameInfo *gameInfo);



//
// Update Game Patch
//
long DataBase_GetLatestGameVersion(long gameID);
PreformedMessage *DataBase_GetGamePatch(long gameID);
char *DataBase_GetGameName(long gameID, char *gameName);

//
// Add a new game to the database.  Update to a new game patch version.
//
Err DataBase_NewGame(long gameID, long version, char *gameName, PreformedMessage *patch);
Err DataBase_AddGamePatch(long gameID, long version, PreformedMessage *patch);


//
// Add Game Results to the database
//
int DataBase_AddGameResult(userIdentification *userID, const GameResult *result);
Err DataBase_UpdateRanking(Account *account, const GameResult *result);

//
// LATA stuff
//
// This is the cheezy old stuff
//
//Err DataBase_LataLookup(phoneNumber *boxPhone, phoneNumber *popPhone, phoneNumber *altPopPhone, Hometown homeTown);
//
// This is the real Compuserve POP file
//
Err DataBase_POPLookup(phoneNumber *boxPhone, phoneNumber *popPhone, phoneNumber *altPopPhone, Hometown homeTown);


#ifdef SPOOGEBALL
//
// Find an opponent.
//
// These are the game-specific waitlists.  You only get added to the general waitlist if you are challenging someone.
//


Boolean DataBase_FindGameOpponentByRank(userIdentification 	*userID,
									long 				gameID,
									long				boxType,
									long				acceptChallenges,
									phoneNumber 		*boxPhoneNumber,
									userIdentification	*opponentUserID,
									phoneNumber 		*opponentPhoneNumber,
									long				*opponentMagicCookie);

Err DataBase_AddOpponentToGameWaitList(	userIdentification 	*userID,
									long 				gameID,
									long				boxType,
									long				acceptChallenges,
									const phoneNumber 	*thePhoneNumber,
									long				*opponentMagicCookie);

Err DataBase_RemoveOpponentFromGameWaitList(		userIdentification 	*userID,
									long gameID);


//
// Challenge specific calls.
//
Err DataBase_AddOpponentToChallengeWaitList(	userIdentification 	*userID,
									long 				gameID,
									long				boxType,
									long				acceptChallenges,
									userIdentification 	*challengeUserID,
									const phoneNumber 	*thePhoneNumber,
									long				*opponentMagicCookie);

Err DataBase_FindChallengeOpponent(userIdentification 	*userID,
									long 				gameID,
									long				boxType,
									long				acceptChallenges,
									userIdentification 	*challengeUserID,
									phoneNumber 		*boxPhoneNumber,
									phoneNumber 		*opponentPhoneNumber,
									long				*opponentMagicCookie);

//
// This is the general waitlist that everyone is added to.  Challengers only get added to this list, and not to a
// game-specific list.
//
Boolean DataBase_AddOpponentToGeneralWaitList(	userIdentification 	*userID,
									long 				gameID,
									long				boxType,
									long				acceptChallenges);

Err DataBase_RemoveOpponentFromGeneralWaitList(	userIdentification 	*userID,
									Boolean				removeFromGameListToo);


void DataBase_PrintWaitQ(void);
#endif /*SPOOGEBALL*/

//
// Mail handling
//
Err DataBase_AddMailToIncoming(const Mail *mail);
Err DataBase_AddMailToBroadcast(const Mail *mail);
long DataBase_SizeofMail(const Mail *mail);
long DataBase_GetNumIncomingMail(const userIdentification *userID);
long DataBase_GetNumBroadcastMail(const userIdentification *userID);
Mail *DataBase_GetIncomingMail(const userIdentification *userID, long mailIndex);
Mail *DataBase_GetBroadcastMail(const userIdentification *userID, long mailIndex);
void DataBase_PrintMail(const Mail *mail);	// print it to the screen for debugging.
void DataBase_MarkMailAsSent(const userIdentification *userID, long mailIndex);
void DataBase_RemoveSentMail( const userIdentification *userID);

unsigned long DataBase_GetCurrentTime(void);
void DataBase_AgeWaitQ(long	maxTimeInWaitQ);


Boolean Database_CompareUserIdentifications(userIdentification *challengeUserID, userIdentification *userID);
int Database_CompareStrings(register const char *str1, register const char *str2);
// In proper fucked-up C style, this returns true if they match, false if different.
Boolean Database_CompareBoxSerialNumbers(BoxSerialNumber *box1, BoxSerialNumber *box2);

//
// Phone number string tweaking.  These should be in *_priv.h, but until
// we have LATA working we need to use them over in the Server_* code.
//
Err DataBase_SplitPhone(const char *number, char *areaCode, char *prefix, char *suffix);
Err DataBase_TweakOpponentPhoneNumber(phoneNumber *oppPhone, const phoneNumber *myPhone);


#endif __ServerDataBase__
