/*
	File:		ServerCore.h

	Contains:	Server Core Entry Points

	Written by:	Brian Topping

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<32>	 9/14/94	ATM		Added a comment about StartGamePlay.
		<31>	 8/28/94	ATM		Changes for segb.
		<30>	 8/27/94	ATM		seg9 stuff.
		<29>	 8/25/94	BET		Add support for multiple NetErrorRecord types for both X25 and
									800 services.
		<28>	 8/25/94	ATM		Updated to seg7 for f2 ROMs.
		<27>	 8/21/94	DJ		new OS version (seg6)
		<26>	 8/20/94	DJ		oops
		<25>	 8/17/94	DJ		receiveloginversion2
		<24>	 8/16/94	DJ		senddateandtime
		<23>	 8/13/94	DJ		forceend
		<22>	 8/13/94	BET		Add kServerReceiveNetErrors.
		<21>	 8/12/94	DJ		calling validatesystem
		<20>	 8/12/94	DJ		supports multiple ROM versions
		<19>	 8/12/94	DJ		personification done right after validatelogin
		<18>	 8/11/94	DJ		made news send immediately
		<17>	 8/10/94	ATM		Changed timeout back to 10 minutes.
		<16>	 8/10/94	DJ		consuonancitoa
		<15>	  8/6/94	DJ		annaina
		<14>	  8/6/94	DJ		büggler
		<13>	 7/26/94	DJ		timeout set to 5 mins
		<12>	 7/25/94	DJ		moved enum here instead of in server.h
		<11>	 7/20/94	DJ		removed configstr from serverstate
		<10>	  7/3/94	DJ		param to CloseServerConnection to avoid TClose (cuz it can hang
									in error case)
		 <9>	 6/10/94	DJ		cheese
		 <8>	  6/5/94	BET		Make it multisession
		 <7>	  6/5/94	BET		Change interfaces
		 <6>	  6/4/94	DJ		just printfing around
		 <5>	  6/1/94	BET		printf
		 <4>	  6/1/94	BET		printf
		 <3>	 5/31/94	BET		Add printf hack
		 <2>	 5/26/94	BET		Update generation of __SERVER__
		 <3>	 5/25/94	DJ		A starting point?
		 <2>	 5/23/94	BET		A starting point?
		 <1>	 5/23/94	BET		Possible poo for you

	To Do:
*/

#ifndef __ServerCore__
#define __ServerCore__

#include "ServerState.h"
// #include "printf.h"
// #define printf bprintf

#if !defined(__SERVER__) && defined(__MWERKS__)
#define __SERVER__
#endif

// prototypes
Boolean DoCommand(ServerState *boxState);
ServerState *InitServer(char *config);
Boolean KillServer(ServerState *boxState);

Boolean CloseServerConnection(ServerState *boxState);
int InitServerConnection(ServerState *boxState);


/************************************************

kSegaIdentification is defined in Messages.h
It is built into the ROM of each box.
We change it for each new version of the ROM.  The starting value was a long == 'sega'.

We maintain a message dispatch table for each version of the ROM.
It is important with this cheesy scheme that you don't reorder the message enum (you can add to it
	anywhere, but swapping the order of existing messages will break it.  You would have to create
	new enums for versions with the swapped IDs).

Server_ReceiveBoxType (in Server_ReceiveLogin.c) is the first message received by the server.
It receives the kSegaIdentification into state->boxOSState.boxType.  It will call
	Server_SwapMessageDispatcher(state->boxOSState.boxType) to change the dispatcher, if required.

*************************************************/

//
// This is the list of old kSegaIdentifications that are supported, as well as the latest.
//
#define kBoxType0	((long)'sega')
#define kBoxType1	((long)'seg1')
#define kBoxType2	((long)'seg2')
#define kBoxType3	((long)'seg3')
#define kBoxType4	((long)'seg4')
#define kBoxType5	((long)'seg5')
#define kBoxType6	((long)'seg6')
#define kBoxType7	((long)'seg7')
#define kBoxType8	((long)'seg8')
#define kBoxType9	((long)'seg9')
#define kBoxTypeb	kSegaIdentification	/* <--- IF YOU CHANGE THIS... */

#define kNumMessageDispatchers	11	/* <--- UPDATE THIS!  the number of different message dispatchers (ie. ROM versions) supported */



enum
{
	kServerDownloadKoolStuff = 0,
	kServerReceiveBoxType,
	kServerReceiveLogin,
	kServerReceiveCreditDebitInfo,
	kServerReceiveGameID,
	kServerReceiveSystemVersion,
	kServerReceiveCompetitiveChallenge,
	kServerReceiveNGP,
	kServerReceiveSendQ,
	kServerReceiveAddressBookValidationQueries,
	kServerReceiveMail,
	kServerReceiveGameResults,
	kServerReceiveGameErrorResults,
	kServerReceiveNetErrors,
	kServerReceivePersonification,

	kServerValidateLogin,
	kServerValidateSystem,
	kServerProcessPersonifications,
	kServerUpdateNGPVersion,
	kServerUpdateGamePatch,
	kServerProcessSendQ,
	kServerProcessIncomingMail,
	kServerSendMail,
	kServerProcessAddrBookValidations,
	kServerSendClearSendQ,
	kServerSendAccountInfo,
	kServerSendRanking,
	kServerSendDateAndTime,

	kServerTests,
	
	// Ought to synchronize here, so we don't queue or match people who
	// failed halfway through the connection.
	kServerStartGamePlay,
	kServerSendProblemToken,
	kServerSendValidationToken,
	kServerUpdateDataBaseAccount,
	kServerEndCommunication,
	
	kServerLastAction
};


typedef struct MessageDispatcher
{
	long boxType;	// this is where the kSegaIdentification is stored.
	
	int (*functions[kServerLastAction])(ServerState *state);
} MessageDispatcher;


int Server_SwapMessageDispatcher(long boxType);
void Server_LoadMessageDispatchers(void);





//
// We tell the box to wait for 9 minutes for an opponent to dial it.
// We should not broker anyone to call someone if they have aged past 9 minutes.
// Actually we must allow some slop time for the dialer to disconnect from Catapult,
//	dismiss any dialogs, and dial the waiting slave.  1 minute.
//
// 
#define kBoxWaitForOpponentTimeout	(60L * 60L * 10L)	/* 10 minutes */
#define kUsualPeerDialingTime		(60L * 60L)			/* 1 minute */
#define kMaxTimeInWaitQ				(kBoxWaitForOpponentTimeout - kUsualPeerDialingTime)


#endif
