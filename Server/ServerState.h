/*
	File:		ServerState.h

	Contains:	The state of a box

	Written by:	Dave Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<40>	  9/7/94	ATM		Removed extractedPhoneNumber.
		<39>	 8/30/94	ATM		Removed accountChangedMask.
		<38>	 8/25/94	BET		Add support for multiple NetErrorRecord types for both X25 and
									800 services.
		<37>	 8/23/94	DJ		added disallowGameConnect
		<36>	 8/21/94	DJ		state->personificationFlagsEditedByServer
		<35>	 8/21/94	DJ		boxFlags added to boxLogin data
		<34>	 8/20/94	DJ		serverUniqueID added to AddrBookInfo
		<33>	 8/19/94	BET		Make serverState->gameResult a pointer.
		<32>	 8/17/94	DJ		receiveloginversion2
		<31>	 8/13/94	BET		Add state changes for net errors.
		<30>	 8/11/94	DJ		personification change flags
		<29>	 8/10/94	DJ		personifications
		<28>	  8/5/94	DJ		player account stuff
		<27>	  8/5/94	DJ		added servervalidflag_addrbookvalidation
		<26>	  8/4/94	BET		Add personification.
		<25>	  8/4/94	DJ		added gameErrorResult
		<24>	  8/4/94	DJ		no more SNDQElement
		<23>	  8/3/94	DJ		changed the way challenge requests work (sends less data now)
		<22>	  8/2/94	DJ		free token is now in debitcardinfo
		<21>	 7/31/94	DJ		lastBoxState sent with login
		<20>	 7/27/94	DJ		CreditToken is now a long
		<19>	 7/20/94	DJ		removed configstr from serverstate
		<18>	 7/20/94	DJ		game results
		<17>	 7/19/94	DJ		gameName is now just a char*
		<16>	 7/18/94	DJ		smartcard stuff, opponentMagicCookie, etc
		<15>	 7/14/94	DJ		new mail
		<14>	 7/12/94	DJ		supporting new challenge format
		<13>	  7/6/94	DJ		address book validation
		<12>	 6/30/94	DJ		updated to new sendq format
		<11>	 6/15/94	DJ		no phone number in LoginData
		<10>	 6/11/94	DJ		added extractedPhoneNumber which is a cache of the box's phone
									number
		 <9>	 6/10/94	DJ		added ConfigString in server state
		 <8>	  6/9/94	BET		Fix hairy compiles based on TransportSlayer.h
		 <7>	  6/5/94	BET		Forgot to include TransportLayer.h on the checkin merge
		 <6>	  6/5/94	BET		Make it multisession
		 <5>	  6/5/94	DJ		box now sends # of mails in inbox so server won't send more than
									kMaxInBox and overflow its mailQ
		 <4>	  6/4/94	DJ		passing in a sessionrec to ServerState_Init
		 <3>	 5/31/94	DJ		added sdbUser (not used yet)
		 <2>	 5/27/94	DJ		updating to userIdentification struct
	To Do:

	I'll do this as necessary:
		We could set flags in the state and hook in the user and box so we can avoid
		researching for them all the time.

*/

#ifndef __ServerState__
#define __ServerState__

// These are included for type definitions (eg. phoneNumber, DBType, Mail)
//
#include "UsrConfg.h"
#include "DataBase.h"
#include "TransportStructs.h"
#include "SendQ.h"
#include "Mail.h"
#include "SmartCard.h"
#include "Personification.h"


typedef struct LoginData {
	userIdentification	userID;
	short				numMailsInBox;	// box can only have kMaxInBoxEntries mails in inbox.
	short				mailSerialNumbers[kMaxInBoxEntries];
	long				validationToken;
} LoginData;


typedef struct SystemVersionData {
	short			length;
	long			version;
} SystemVersionData;


typedef struct QItem {
	long			size;
	unsigned char	*data;
	DBID			theID;
} QItem;


typedef struct SendQData {
	// mail
	// address book entries for validation
	// game results
	// other personification poop

	short			count;
	QItem			*items;
} SendQData;


typedef struct ChallengeData {
	unsigned char		challengeType;
	userIdentification	userID;
} ChallengeData;


typedef struct GameIDData {
	long		gameID;
	long		version;
} GameIDData;


typedef struct AddrValidItem {
	userIdentification	userIdent;
	unsigned char		ownerUserID;
	unsigned char		serverUniqueID;
} AddrValidItem;

typedef struct AddrValidationData {
	short			count;
	AddrValidItem	*items;
} AddrValidationData;


typedef struct MailItem {
	short			size;
	Mail			*mail;
} MailItem;

typedef struct IncomingMail {
	short			count;
	MailItem		*mailItems;
} IncomingMail;

typedef struct BoxOSState {
	long	boxType;
	long	osState;	// <--- no longer sent after version2 of ReceiveLogin (8/16/94)
	long	lastBoxState;
	
	long	osFree;
	long	dbFree;
	
	long	boxFlags;
} BoxOSState;

typedef struct CreditDebitInfo {
	unsigned char			usingType;

	XBANDCard				debitCardInfo;
} CreditDebitInfo;

typedef struct ServerState{

	CreditDebitInfo		creditDebitInfo;

	BoxOSState			boxOSState;

	//phoneNumber			extractedPhoneNumber;	// 	extracted from the DB at CheckAccount() time

	phoneNumber			boxPhoneNumber;	// this phone number can come from several places:
											//	- ani
											//	- message to change local number.

	char				configBuffer[kConfigStrLength];	// for configuring TListen()

	long				validFlags;

	LoginData			loginData;
	SystemVersionData	systemVersionData;
	
	long				NGPVersion;
	
	SendQData			sendQData;

	AddrValidationData	addrValidationData;

	IncomingMail		incomingMail;

	ChallengeData		challengeData;

	GameIDData			gameIDData;

	GameResult			*gameResult;
	GameResult			gameErrorResult;

	PersonificationSetupRec	 userPersonification;
	unsigned char		personificationFlags;
	unsigned char		personificationFlagsEditedByServer;	// which fields the server edited (added kustom icon, filtered taunt, etc)

	struct Account		*account;
	
	NetErrorRecord		boxNetErrors800;
	NetErrorRecord		boxNetErrorsX25;

	// the GameTalk connection record.
	struct SessionRec	*session;

	short				serverProgress;
	Boolean				disallowGameConnect;		// set to true by ValidateLogin and UpdateGamePatch if unsupported game or no credits or bad restrictions.

} ServerState;


//
// These flags let us track what's been sent to the server from the box
//
#define kServerValidFlag_None				0
#define kServerValidFlag_Login				1L
#define kServerValidFlag_SystemVersion		(1L<<1)
#define kServerValidFlag_NGPVersion			(1L<<2)
#define kServerValidFlag_SendQ				(1L<<3)
#define kServerValidFlag_ChallengeOrCompete	(1L<<4)
#define kServerValidFlag_GameID				(1L<<5)
#define kServerValidFlag_SessionRec			(1L<<6)
#define kServerValidFlag_ConfigString		(1L<<8)
#define kServerValidFlag_BoxPhoneNumber		(1L<<9)
//#define kServerValidFlag_ExtractedPhoneNumber		(1L<<10)
#define kServerValidFlag_AddrValidation		(1L<<11)
#define kServerValidFlag_IncomingMail		(1L<<12)
#define kServerValidFlag_CreditDebitInfo	(1L<<13)
#define kServerValidFlag_BoxType			(1L<<14)
#define kServerValidFlag_GameResults		(1L<<15)
#define kServerValidFlag_GameErrorResults	(1L<<16)
#define kServerValidFlag_AddressBook		(1L<<17)
#define kServerValidFlag_Account			(1L<<18)
#define kServerValidFlag_Personifications	(1L<<19)
#define kServerValidFlag_NetErrors800		(1L<<20)
#define kServerValidFlag_NetErrorsX25		(1L<<21)

//
// Routines.
//
void ServerState_Init(ServerState *state, struct SessionRec *session, char *configString );
void ServerState_Print(ServerState *state);
void ServerState_PrintUserIdentification(const userIdentification *userID);
void ServerState_Empty(ServerState *state);

#endif __ServerState__
