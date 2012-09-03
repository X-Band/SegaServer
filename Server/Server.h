/*
	File:		Server.h

	Contains:	xxx put contents here xxx

	Written by:	Dave Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<51>	 9/18/94	ATM		Server_Log.h --> Common_Log.h.  Incremented HAPPY_REGION.
		<50>	 9/17/94	ATM		Added INLINE for those about to rock.
		<49>	 9/16/94	DJ		xband free credits now == 32 not 35
		<48>	 9/16/94	ATM		HAPPY_REGION reaches double digits, film at 11.
		<47>	 9/11/94	ATM		Moved logging stuff into its own header, so you don't have to
									include all of Server.h if you just want logs.
		<46>	  9/7/94	ATM		Added LOG_RPC.
		<45>	  9/7/94	ATM		Incremented HAPPY_REGION.
		<44>	  9/6/94	ATM		Added LOG_MATCHING.
		<43>	  9/4/94	ATM		HAPPY_REGION gets even happier.
		<42>	  9/3/94	ATM		Changed LOG_DEBUG to LOG_DBUG.
		<41>	  9/1/94	ATM		Changed all the logging stuff.
		<40>	 8/28/94	ATM		Proto for Server_SEG9_ReceiveMail.
		<39>	 8/26/94	DJ		added Server_RestoreBox
		<38>	 8/25/94	BET		Change HAPPY_REGION to 5.
		<37>	 8/25/94	BET		Update for multiple NetErrorRecord types.
		<36>	 8/25/94	ATM		Increased HAPPY_REGION to 4.
		<35>	 8/25/94	DJ		add Server_SendClearMiscQueues
		<34>	 8/25/94	ATM		Add prototype for StatusPrintGameResults.
		<33>	 8/21/94	DJ		added Server_RestoreBox.c
		<32>	 8/21/94	DJ		setlocalaccessphonenumber takes 2 phone nums (2nd is fallback
									number)
		<31>	 8/20/94	BET		Added Server_SendOpponentNameString.
		<30>	 8/20/94	DJ		receiveaddressbookverifications2
		<29>	 8/19/94	DJ		added Server_SendWorthString
		<28>	 8/19/94	ATM		HAPPY_REGION is now 3.  Is there a better way to do this?
		<27>	 8/18/94	ATM		Incremented HAPPY_REGION.
		<26>	 8/17/94	ATM		Added Crashmsg per request.
		<25>	 8/17/94	ATM		Moved HAPPY_VERSION in here.
		<24>	 8/17/94	DJ		receiveloginversion2
		<23>	 8/17/94	ATM		Added Loghexdump.
		<22>	 8/16/94	DJ		senddebitsmartcard
		<21>	 8/16/94	DJ		senddateandtime
		<20>	 8/13/94	DJ		forceendcomm
		<19>	 8/13/94	DJ		Sever_ValidateLogin returns int instead of err
		<18>	 8/13/94	BET		Add Server_ReceiveNetErrors.
		<17>	 8/12/94	ATM		Added Server_GameName.
		<16>	 8/11/94	ATM		Added message logging stuff.
		<15>	 8/10/94	ATM		Moved area code munging out, added phoneNumber prettifier.
		<13>	  8/9/94	ATM		Phone number tweak routines for area codes.
		<12>	  8/8/94	DJ		SendDialog takes boolean whether to stick or disappear in 3 sec.
		<11>	  8/5/94	DJ		playeraccout stuff
		<10>	  8/5/94	ATM		Added #include "AddressBook.h"
		 <9>	  8/5/94	DJ		new address book poop: Server_SendCorrelateAddressBookEntry
		 <8>	  8/4/94	DJ		added Server_ReceiveGameErrorResults
		 <7>	  8/3/94	ATM		Fuck.
		 <6>	  8/3/94	ATM		Added memory watch debug stuff.
		 <5>	  8/3/94	DJ		handy dandy utility: Server_SendUserIdentification
		 <4>	  8/2/94	DJ		removed sendbitmap and sendtext
		 <3>	 7/29/94	DJ		added Server_SendCreditToken
		 <2>	 7/25/94	DJ		5 days of hacking, including: added Server_SendDBConstants
		 <1>	 7/20/94	DJ		first checked in
		<29>	 7/20/94	BET		Blew away all the "Brain Damaged Shit" that was in the last
									checkin.
		<28>	 7/20/94	DJ		added Server_Comm stuff
		<27>	 7/19/94	DJ		gamename is now just a char*
		<26>	 7/19/94	DJ		server tests routine
		<25>	 7/18/94	DJ		added #defines for msg order
		<24>	 7/16/94	BET		unix-ise include file capitalization.
		<23>	 7/15/94	DJ		added largedialog
		<22>	 7/14/94	DJ		new mail msgs
		<21>	 7/13/94	DJ		added Server_SendRegisterPlayer
		<20>	 7/12/94	DJ		updated Server_SendNewBoxSerialNumber to send new boxser format
		<19>	  7/8/94	DJ		msgs to set box phone numbers
		<18>	  7/6/94	DJ		address book validation
		<17>	  7/2/94	DJ		rankings
		<16>	 6/30/94	DJ		new news
		<15>	 6/11/94	DJ		added kServerFuncEnd retVal
		<14>	 6/10/94	DJ		Server_DebugService
		<13>	  6/9/94	BET		Fix hairy compiles based on TransportSlayer.h
		<12>	  6/5/94	DJ		removed extra proto for Server_SendBitmap
		<11>	  6/4/94	DJ		making everything take a ServerState instead of SessionRec
		<10>	  6/2/94	BET		news
		 <9>	  6/1/94	DJ		api tweaks
		 <8>	 5/31/94	DJ		Mail
		 <7>	 5/29/94	DJ		KoolStuff
		 <6>	 5/27/94	DJ		Server driving: accounts, opponents, patches, system validation,
									etc.
		 <5>	 5/26/94	BET		Update for new world order
		 <4>	 5/26/94	DJ		added LoopBack for network debugging
		 <3>	 5/25/94	DJ		fixed include prob
		 <2>	 5/25/94	DJ		added text sending
		 <2>	 5/25/94	DJ		fucked with project farts
	To Do:
*/

#ifndef __Server_h__
#define __Server_h__


#include "Errors.h"

#if !defined(__SERVER__) && defined(__MWERKS__)
#define __SERVER__
#endif

#define DEBUG		1
#define MESSAGES	1
#define DEBUGSERVICE	1		// turns on network debugging (loopbacks, etc.)

// This should be "inline" or blank.  Requires "gcc -O2" to do anything
// useful.  If they didn't specify it with -D, they probably don't want it.
//
#ifndef INLINE
# define INLINE
#endif


#define HAPPY_REGION	11		// debugging hack, used to invalidate accounts

#define kFreeXBandCredits	32	// number of free credits given during Alpha & Beta test.


#include "ServerState.h"		// for ServerState
#include "SegaGraphics.h"		// for segaBitMap
#include "News.h"				// for NewsPage
#include "PreformedMessage.h"
#include "DataBase.h"
#include "GameDB.h"				// for gameName
#include "AddressBook.h"		// for AddressBook
#include "Common_Log.h"			// all them logging goodies

typedef int (*ServerFunc)(struct SessionRec *session);

enum {
	kServerFuncOK,
	kServerFuncSkipNext,
	kServerFuncAbort,		// fatal comm error
	kServerFuncEnd,			// can't finish transaction (example: unsupported game)
	kServerFuncForceEnd		// same as kServerFuncEnd, but drops line. does not wait for box's endcomm msg
};




//
// Load the database with initial accounts and game patches.
//
//void Server_SetupAccounts(void);
//void Server_SetupGamePatches(void);


//
// Debugging of the network (sends loopbacks, etc)
//
#ifdef DEBUGSERVICE

int Server_DebugService(ServerState *state);

#else

#define Server_DebugService(a)

#endif

//
// returns a long in sega date format for today.  used for mail and news to timestamp.
//
long Server_GetSegaDate(void);

//
// Handlers for receiving from box
//
int Server_ReceiveBoxType(ServerState *state);
int Server_ReceiveLogin(ServerState *state);
	int Server_ReceiveLoginVersion2(ServerState *state);	// 8/16/94 version of ReceiveLogin
int Server_ReceiveGameID(ServerState *state);
int Server_ReceiveSystemVersion(ServerState *state);
int Server_ReceiveCompetitiveChallenge(ServerState *state);
int Server_ReceiveNGP(ServerState *state);
int Server_ReceiveSendQ(ServerState *state);
int Server_ReceiveAddressBookValidationQueries(ServerState *state);
	int Server_ReceiveAddressBookValidationQueries2(ServerState *state);	// 8/19/94 version.
int Server_EndCommunication(ServerState *state);
int Server_ForceEndCommunication(ServerState *state);
int Server_ReceiveMail(ServerState *state);
int Server_SEG9_ReceiveMail(ServerState *state);	// temp compat value
int Server_ReceiveCreditDebitInfo(ServerState *state);
	// helper routine for Server_ReceiveCreditDebitInfo and Server_SendDebitSmartcart
	int Server_ReceiveCreditDebitInfoIntoStruct(ServerState *state, CreditDebitInfo *creditDebitInfo);

int Server_ReceiveGameResults(ServerState *state);
int Server_ReceiveGameErrorResults(ServerState *state);
int Server_ReceivePersonification(ServerState *state);
int Server_ReceiveNetErrorsOld(ServerState *state);
int Server_ReceiveNetErrorsCombined(ServerState *state);
int Server_ReceiveNetErrorsNew(ServerState *state);

//
// Drive the box
//
int Server_Drive(ServerState *state);
int Server_UpdateDataBaseAccount(ServerState *state);	// called before EndCommunication to update Account and do things.



int Server_SendLoopback(ServerState *state, long numLoops, Boolean sticky);
int Server_SendWaitForOpponent(ServerState *state, long magicCookie);
int Server_SendOpponentPhoneNumber(ServerState *state, const phoneNumber *opponentPhoneNumber, long magicCookie);
int Server_SendNewBoxSerialNumber(ServerState *state, BoxSerialNumber *boxSerialNumber);
int Server_SendUnsupportedGame(ServerState *state);
int Server_SendDialog(ServerState *state, char *str, Boolean sticky);
int Server_SendLargeDialog(ServerState *state, char *str, Boolean sticky);

int Server_SendQDefDialog(ServerState *state, short when, char *cString, DBID templat, short minTime, short maxTime);
int Server_SendRanking(ServerState *state);
int Server_SendAddItemToDB(ServerState *state, DBType theType, DBID theID, long length, void *data);
int Server_SendClearSendQ(ServerState *state);
int Server_SendClearMiscQueues(ServerState *state);  // used instead of ClearSendQ in ROM version 7 (seg7)
int Server_SendSetBoxPhoneNumber(ServerState *state, phoneNumber *newBoxPhoneNumber);
int Server_SendSetLocalAccessPhoneNumber(ServerState *state, phoneNumber *newAccessPhoneNumber, phoneNumber *fallbackAccessPhoneNumber, Boolean redail);
int Server_SendRegisterPlayer(ServerState *state, long timeoutValue, char *gameName);
int Server_SendRegisterChallengePlayer(ServerState *state, long timeoutValue, char *gameName, char *challengeName);
int Server_SendAccountInfo(ServerState *state);
int Server_SendRestrictions(ServerState *state);
int Server_SendCreditInfo(ServerState *state);
int Server_SendNewBoxHometown(ServerState *state, Hometown town);
int Server_SendNewCurrentUserName(ServerState *state, UserName name);
int Server_SendDBConstants(ServerState *state, long numConsts, DBID *ids, long *constants);
int Server_SendProblemToken(ServerState *state);
int Server_SendValidationToken(ServerState *state);
int Server_SendCreditToken(ServerState *state, unsigned long smartCardToken);
int Server_SendDateAndTime(ServerState *state);
int Server_SendDebitSmartcard(ServerState *state, long numDebits);
int Server_SendWorthString(ServerState *state);
int Server_SendOpponentNameString(ServerState *state, char *opponentName);

int Server_SendTests(ServerState *state);


int Server_SendCorrelateAddressBookEntry(ServerState *state,
										userIdentification *userID,
										unsigned char ownerUserID,
										unsigned char serverUniqueID);

int Server_SendPlayerInfo(				ServerState *state,
										struct PlayerInfo *playerInfo,
										unsigned char ownerUserID);

int Server_SendDeleteAddressBookEntry(	ServerState *state,
										userIdentification *userID,
										unsigned char ownerUserID);


// sends a preformed message (eg. mail, patch, Zeus-generated bitmap).
int Server_SendPreformedMessage(ServerState *state, PreformedMessage *mesg);

// news 
typedef struct ServerNewsPage {
	DBType		type;
	long		length;
	NewsPage	*page;
} ServerNewsPage;

int Server_SendFirstNewsPage(ServerState *state, ServerNewsPage *page);
int Server_SendNoNewsPage(ServerState *state, DBType pagetype);
int Server_SendNewsPage(ServerState *state, ServerNewsPage *page);

//
// Do the real server work.
//
int Server_SendSystemPatches(ServerState *state, long version);
int Server_ValidateSystem(ServerState *state);
int Server_ValidateLogin(ServerState *state);
int Server_UpdateNGPVersion(ServerState *state);
int Server_UpdateGamePatch(ServerState *state);
int Server_ProcessSendQ(ServerState *state);
int Server_StartGamePlay(ServerState *state);
int Server_SendMail(ServerState *state);
int Server_ProcessAddrBookValidations(ServerState *state);
int Server_ProcessIncomingMail(ServerState *state);
int Server_ProcessPersonifications(ServerState *state);

int Server_DownloadKoolStuff(ServerState *state);


//
// In case the box loses its poor little mind.  Called by Server_ValidateLogin().
//
int Server_RestoreBox(ServerState *state, Boolean callingFrom800);


//
// Handy dandy utility
//

int Server_SendUserIdentification(ServerState *state, userIdentification *userID);

//
// Phone number thang
//
Err Server_MakePhoneNumberPretty(phoneNumber *newAccessPhoneNumber);


void StatusPrintGameResults(ServerState *state);

#endif __Server_h__
