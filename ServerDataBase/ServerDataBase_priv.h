/*
	File:		ServerDataBase_priv.h

	Contains:	xxx put contents here xxx

	Written by:	David Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<33>	 9/18/94	ATM		DataBase_List.h --> Common_List.h.
		<32>	 9/13/94	ATM		Changed to ReloadPhoneStuff.
		<31>	 9/12/94	DJ		added Compuserve POP lookup and loading
		<30>	  9/7/94	ATM		phoneNumber result buf no longer passed to FindAccount,
									CreateAccount, and CreateBoxAccount.
		<29>	  9/6/94	ATM		#ifdef out all references to SDBWaiting.
		<28>	  9/1/94	ATM		Add LataIsLocal prototype.
		<27>	 8/26/94	ATM		Overhaul of FindAccount and CreateAccount.
		<26>	 8/25/94	ATM		boxType and acceptChallenges stuff.
		<25>	 8/24/94	ATM		Added reload stuff.
		<24>	 8/20/94	ATM		Moved prototypes for NewGame and AddGamePatch in here.
		<23>	 8/19/94	ATM		Proto for DataBase_LoadGamePatches.
		<22>	 8/16/94	ATM		New prototype for FindGame.
		<21>	 8/15/94	ATM		Changed SDBBox to hold full BoxSerialNumber.
		<20>	 8/13/94	ATM		Added prototype for transmogrifier.
		<19>	 8/13/94	ATM		Removed ref to gLogFile.
		<18>	 8/12/94	ATM		Added prototype for LoadFakeLata.
		<17>	 8/12/94	ATM		Added Statusmsg.
		<16>	 8/11/94	ATM		Added prototype for ReplaceEscapedChars.
		<15>	  8/8/94	ATM		New stuff for loading News.
		<14>	  8/6/94	ATM		Added stuff for new uniquification code.
		<13>	  8/5/94	ATM		PlayerAccount --> Account.
		<12>	  8/5/94	ATM		Added PlayerAccount stuff.
		<11>	  8/3/94	DJ		made work on mac
		<10>	 7/25/94	DJ		added the general waitq
		 <9>	 7/19/94	DJ		gameName is now just a char*, also I added mailSerialNumber to
									SDBUser
		 <8>	 7/18/94	DJ		magicCookie stuff
		 <7>	  7/6/94	DJ		address book validation
		 <6>	  7/3/94	DJ		added Database_FindUserByHandle
		 <5>	 6/30/94	DJ		broadcast mail
		 <4>	 6/30/94	DJ		SDBSystem_LoadSystemPatches
		 <3>	 6/11/94	DJ		added waitQs on games for users waiting to play
		 <2>	  6/5/94	DJ		added SDBMail so that deleting of sent mail is possible
		 <1>	 5/31/94	DJ		The database

	To Do:
*/



#ifndef __ServerDataBase_priv__
#define __ServerDataBase_priv__

#include "ServerDataBase.h"
#include "GameDB.h"
#include "Common_List.h"

#include "ServerDataBase_FILE.h"

#include <stdio.h>


//
// DB mail header.
//

typedef struct SDBMail {
	Mail	*mail;
	Boolean	sentToBox;
} SDBMail;

//
// DB news header.
//
typedef struct SDBNewsPaper {
	DBType		pageType;
	ListHead	*pageList;		// list of ServerNewsPage;
} SDBNewsPaper;

//
// DB news header.
//
typedef struct SDBNews {
	ListHead	*paperList;		// list of SDBNewsPaper;
} SDBNews;

//
// User database
//

typedef struct SDBUser {
	userIdentification	userID;
	
	short				mailSerialNumber;
	ListHead			*incomingMail;
	
	long				lastBroadcastMailSent;	// timestamp of last broadcast mail ever sent.

	// game results list (of all games ever played?)
	// ranking for each gameID.
	// all mail ever sent?
	// every time user logged in.
	PlayerAccount		playerAccount;
	
} SDBUser;

typedef struct SDBBox {
	BoxSerialNumber		boxSerialNumber;	// should be a BoxSerialNumber??
	phoneNumber	boxPhoneNumber;		// move this into boxAccount
	SDBUser		*users[4];
	
	// other account information...
	// name, address, restrictions, credit card, whatever...
	BoxAccount	boxAccount;
} SDBBox;

typedef struct SDBUsers {
	long		timeStamp;
	long		uniqueBoxSerialNumber;
	long 		numUsers;
	ListHead	*list;
} SDBUsers;


//
// Rankings and Opponent waiting queue.
//
typedef struct SDBRanking{
	// user id
	// which game
	// subclassed: game ranking info
	long shit;
} SDBRanking;

#ifdef SPOOGEBALL
typedef struct SDBWaitingNode {

	Boolean				isChallenge;

	userIdentification 	userID;
	long 				gameID;
	long				boxType;
	long				acceptChallenges;
	userIdentification 	challengeUserID;

	phoneNumber			opponentPhoneNumber;
	long				opponentMagicCookie;
	long				addedToWaitQTimestamp;
} SDBWaitingNode;

typedef struct SDBWaiting{
	ListHead		*waitingQ;
} SDBWaiting;
#endif


//
// Game patches.
//
typedef struct SDBGameNode {
	long				gameID;
	long				version;
	char				gameName[kGameNameSize];
	PreformedMessage	*patch;
	ListHead			*rankings;
#ifdef SPOOGEBALL
	SDBWaiting			*waiting;
#endif
} SDBGameNode;

typedef struct SDBGames {
	ListHead	*list;
	long		opponentMagicCookie;
	
#ifdef SPOOGEBALL
	SDBWaiting			*generalWaiting;
#endif
} SDBGames;


//
// Network Game Patch list
//
typedef struct SDBNGP {
	NGPData	*ngp;
} SDBNGP;

SDBNGP *SDBNGP_New(void);



//
// list of system updates
// some are keyframes, some are incremental updates
//
typedef struct SDBSystemNode {
	long 				version;
	Boolean				keyframe;
	long				numPatches;
	PreformedMessage	**patches;
} SDBSystemNode;

typedef struct SDBSystem {
	ListHead	*list;
} SDBSystem;


//
// the Server DataBase
//
typedef struct SDB {
	SDBSystem	*system;
	SDBNGP		*ngp;
	SDBGames	*games;
	SDBUsers	*users;
	SDBNews		*news;
	ListHead	*broadcastMail;
} SDB;


// Main database creation and get routines
//
SDB *SDB_New(void);
Err SDB_Dispose(SDB *sdb);
Err SDB_Save(FILE *fp);
Err SDB_Restore(FILE *fp);

SDBSystem *SDB_GetSystem(SDB *sdb);
SDBUsers *SDB_GetUsers(SDB *sdb);
SDBNGP *SDB_GetNGP(SDB *sdb);
SDBGames *SDB_GetGames(SDB *sdb);
SDBNews *SDB_GetNews(SDB *sdb);


// System creation and assorted routines
//
SDBSystem *SDBSystem_New(void);
Err SDBSystem_Save(FILE *fp);
Err SDBSystem_Restore(FILE *fp);
Err SDBSystem_Print(void);
Err SDBSystem_LoadSystemPatches(void);


// User account routines
//
SDBUser *Database_FindUser(const userIdentification *userID );
SDBUser *Database_FindUserByHandle(const char *userName );
SDBBox *Database_FindBoxAccount(SDBUsers *users, const BoxSerialNumber *boxSerialNumber);
SDBBox *Database_CreateBoxAccount( SDBUsers *users, const userIdentification *userID);
SDBUser *Database_CreateBoxUserAccount(SDBBox *box, const userIdentification *userID);

// Uniquifier
//
Boolean DataBase_UniquifyHandle(userIdentification *userID);
Boolean IsHandleWidthLegal( char *cString );
#define kMaxNameUnique 8192

// PlayerAccount and BoxAccount internal routines
//
Err DataBase_InitBoxAccount(SDBBox *box);
Err DataBase_InitPlayerAccount(SDBUser *user);


// Users (a holder for the list of user accounts).
//
SDBUsers *SDBUsers_New(void);


// Game routines
//
SDBGames *SDBGames_New(void);
SDBGameNode *Database_FindGame(long gameID, long raw);
long Database_NewOpponentMagicCookie(void);

#ifdef SPOOGEBALL
// Waiting queue (one per game)
//
SDBWaiting *SDBWaiting_New(void);
void SDBWaiting_AddOpponent(	SDBWaiting 			*waiting,
								Boolean				isChallenge,
								userIdentification 	*userID,
								long 				gameID,
								long				boxtype,
								long				acceptChallenges,
								userIdentification 	*challengeUserID,
								const phoneNumber 	*thePhoneNumber,
								long				opponentMagicCookie);
#endif

// Handy treats
//
Mail *Database_DuplicateMail(const Mail *mail);

// Mail routines
//
Err DataBase_LoadBroadcastMail(void);
Err DataBase_AddStampedMailToBroadcast(const Mail *mail, long timeStamp);

// News routines
SDBNews *SDBNews_New(void);
Err DataBase_LoadNewsPages(void);
Boolean DataBase_ParseNewsDataLine(char *line, FontID *fontid, \
	char *color0, char *color1, char *color2, char *color3);
void DataBase_ReplaceEscapedChars(char *str);
Boolean DataBase_TranslateConstant(char *line, long *value);

//
// Add a new game to the database.  Update to a new game patch version.
//
Err DataBase_NewGame(long gameID, long version, char *gameName, PreformedMessage *patch);
Err DataBase_AddGamePatch(long gameID, long version, PreformedMessage *patch);


//
// Faux LATA stuff
//
Err DataBase_LoadFakeLata(void);
Err DataBase_LoadCompuservePOPs(void);	/* real Compuserve POP database */
int DataBase_LataIsLocal(const phoneNumber *boxPhoneNumber, const phoneNumber *opponentPhoneNumber);

long DataBase_TransmogrifyGameID(long GameID);

void DataBase_LoadGamePatches(void);

//
// Vicious reload routines
//
void DataBase_ReloadPhoneStuff(void);
void DataBase_ReloadNewsPages(void);
void DataBase_ReloadBroadcastMail(void);
void DataBase_ReloadGamePatches(void);

//
// GLOBALS
//
#ifndef __DataBase_Core__
extern SDB *gSDB;
#endif

#endif __ServerDataBase_priv__
