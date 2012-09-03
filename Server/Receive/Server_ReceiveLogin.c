
/*
	File:		Server_ReceiveLogin.c

	Contains:	Server ReceiveLogin function

	Written by:	Dave Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<34>	 8/27/94	ATM		Made "software version we don't understand" dialog happier,
									threw in a log message.
		<33>	 8/22/94	DJ		printing out more of the boxFlags
		<32>	 8/21/94	DJ		a new long flag in receivelogin
		<31>	 8/20/94	DJ		no serverUniqueID in userIdentification
		<30>	 8/18/94	DJ		tweaked a logsmsg msg
		<29>	 8/17/94	DJ		receiveloginversion2
		<28>	 8/13/94	DJ		boxtype calls forceend if unsupported rom version
		<27>	 8/12/94	DJ		supports multiple ROM versions
		<26>	 8/12/94	ATM		Converted to Logmsg.
		<25>	 8/10/94	BET		Added scriptID brokering vi phoneNumber structure.
		<24>	  8/8/94	DJ		SendDialog takes boolean whether to stick or disappear in 3 sec.
		<23>	  8/4/94	BET		Changed a field name.
		<22>	  8/2/94	DJ		fixed a printef
		<21>	 7/31/94	DJ		lastBoxState sent with login
		<20>	 7/27/94	DJ		receiving login validation token
		<19>	 7/20/94	DJ		added Server_Comm stuff
		<18>	 7/19/94	DJ		receiving serialnumbers of mail inbox so we can backup if box
									dies sometime
		<17>	 7/18/94	DJ		separated boxType from login msg
		<16>	 7/17/94	BET		Make printf's go to gLogFile instead of stdout, which is the
									modem.
		<15>	 7/15/94	DJ		nothing
		<14>	 7/14/94	DJ		added segaID and box state and OS state
		<13>	 7/12/94	DJ		using Server_TCheckError instead of TCheckError
		<12>	  7/1/94	DJ		making server handle errors from the comm layer
		<11>	 6/30/94	DJ		boxsernum no longer sent at login (is part of userID)
		<10>	 6/29/94	BET		(Really DJ) Clean up after KON.
		 <9>	 6/15/94	DJ		simulating ANI with state->boxPhoneNumber
		 <8>	 6/11/94	DJ		trying to figure out where ANI fits in this hack-o server
		 <7>	  6/9/94	BET		Fix hairy compiles based on TransportSlayer.h
		 <6>	  6/5/94	DJ		reading # mails in box inbox at login time
		 <5>	  6/4/94	DJ		making everything take a ServerState instead of SessionRec
		 <4>	  6/1/94	DJ		phone numbers are now kPhoneNumberSize in length, not
									kPhoneNumberSize + 1
		 <3>	 5/31/94	DJ		printing userinfo
		 <2>	 5/27/94	DJ		updated to userIdentifier struct
	To Do:
*/

#include "ServerCore.h"
#include "Server.h"
#include "Messages.h"
#include "UsrConfg.h"
#include "Server_Comm.h"
#include "ServerCore.h"

#include <stdio.h>


int Server_ReceiveBoxType(ServerState *state)
{
unsigned char 	opCode;
char			msg[256];

	Logmsg("Server_ReceiveBoxType\n");

	if(Server_TReadDataSync(state->session, 1, (Ptr)&opCode) != noErr)
		return(kServerFuncAbort);

	if(opCode != msBoxType){
		// fucked
		Logmsg("Wrong opCode.  Expected %d, got %d\n", msBoxType, opCode);
		return(kServerFuncAbort);
	}

	Server_TReadDataSync(state->session, sizeof(long), (Ptr)&state->boxOSState.boxType );

	if(Server_SwapMessageDispatcher(state->boxOSState.boxType) != kServerFuncOK)
	{
		char versBuf[5];

		*(long *)versBuf = state->boxOSState.boxType;
		versBuf[4] = '\0';

		sprintf(msg, "Your box has a software version (%s) that the server does not understand.  Call Catapult for help.", versBuf);
		Server_SendLargeDialog(state, msg, true);
		Logmsg("BAILING: told the box that it's ROM is too old (%s)\n", versBuf);
		return(kServerFuncForceEnd);
	}
	
	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	state->validFlags |= kServerValidFlag_BoxType;

	Logmsg("Server_ReceiveBoxType done\n");

	return(kServerFuncOK);
}


//
// Version 2 of ReceiveLogin (8/16/94).
// 	- Josh added 2 longs that tell us how much free memory is available on the box.
//
int Server_ReceiveLoginVersion2(ServerState *state)
{
unsigned char 	opCode;
register short	a;
long			totalFree, maxFree;

	Logmsg("Server_ReceiveLogin\n");

	if(Server_TReadDataSync(state->session, 1, (Ptr)&opCode) != noErr)
		return(kServerFuncAbort);

	if(opCode != msLogin){
		// fucked
		Logmsg("Wrong opCode.  Expected %d, got %d\n", msLogin, opCode);
		return(kServerFuncAbort);
	}

	Server_TReadDataSync(state->session, sizeof(long), (Ptr)&state->boxOSState.osFree );
	Server_TReadDataSync(state->session, sizeof(long), (Ptr)&state->boxOSState.dbFree );

	totalFree = state->boxOSState.osFree >> 16;
	maxFree = state->boxOSState.osFree & 0xffff;
	Logmsg(" OS total free memory = %ld, max free memory = %ld.\n", totalFree, maxFree);

	totalFree = state->boxOSState.dbFree >> 16;
	maxFree = state->boxOSState.dbFree & 0xfffe;	// clear the last bit
	Logmsg(" DB total free memory = %ld, max free memory = %ld.\n", totalFree, maxFree);


/*
	kCurUserAcceptsChallenges		= 0x01,
	kCurUserMoved					= 0x02,
	kOSHeapTrashed					= 0x04,
	kDBHeapTrashed					= 0x08,
	kBoxIDTrashed					= 0x10,
	kQwertyKeyboard					= 0x20,
	kCallWaitingEnabled				= 0x40
*/

	Server_TReadDataSync(state->session, sizeof(long), (Ptr)&state->boxOSState.boxFlags );
	//
	// Print out what the various boxFlags mean.
	//
	if(state->boxOSState.boxFlags & kOSHeapTrashed)
		Logmsg(" *** OS Heap Trashed ***\n");
	else
		Logmsg(" OS Heap OK\n");
	if(state->boxOSState.boxFlags & kDBHeapTrashed)
		Logmsg(" *** DB Heap Trashed ***\n");
	else
		Logmsg(" DB Heap OK\n");
	if(state->boxOSState.boxFlags & kBoxIDTrashed)
		Logmsg(" *** Box ID Trashed ***\n");
	else
		Logmsg(" Box ID OK\n");

	if(state->boxOSState.boxFlags & kCurUserMoved)
		Logmsg(" Box is dialing 1-800 because it has moved\n");
	if(state->boxOSState.boxFlags & kCurUserAcceptsChallenges)
		Logmsg(" User accepts challenges\n");
	else
		Logmsg(" User does not accept challenges\n");
	if(state->boxOSState.boxFlags & kQwertyKeyboard)
		Logmsg(" Using Qwerty keyboard\n");
	else
		Logmsg(" Using ABC keyboard\n");
	if(state->boxOSState.boxFlags & kCallWaitingEnabled)
		Logmsg(" Call waiting enabled\n");
	else
		Logmsg(" Call waiting not enabled\n");






	Server_TReadDataSync(state->session, sizeof(long), (Ptr)&state->boxOSState.lastBoxState );


	// == BRAIN DAMAGE ==  6/10/94
	//
	// This simulates an ANI notification of the phone number.  Normally it will not be sent by the
	// box (especially not at every connection!).  It will be found by the 1-800 server by calling
	// ANI_GetPhoneNumber(&state->boxPhoneNumber);
	//
	Server_TReadDataSync(state->session, sizeof(phoneNumber), (Ptr)&state->boxPhoneNumber );
	state->boxPhoneNumber.phoneNumber[kPhoneNumberSize - 1] = 0;	// extraneous.

	Server_TReadDataSync( state->session, sizeof(userIdentification), (Ptr)(Ptr)&state->loginData.userID);
	state->loginData.userID.userName[kUserNameSize - 1] = 0;		// extraneous.

	Server_TReadDataSync( state->session, sizeof(short), (Ptr)&state->loginData.numMailsInBox);

	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	ASSERT_MESG(state->loginData.numMailsInBox <= kMaxInBoxEntries, "Fatal Box Error: numMailsInBox must be <= kMaxInBoxEntries");

	for(a = 0; a < state->loginData.numMailsInBox; a++)
		Server_TReadDataSync( state->session, sizeof(short), (Ptr)&state->loginData.mailSerialNumbers[a]);

	Server_TReadDataSync( state->session, sizeof(long), (Ptr)&state->loginData.validationToken);

	state->validFlags |= kServerValidFlag_BoxPhoneNumber;
	state->validFlags |= kServerValidFlag_Login;

	Logmsg("UserID box serial number = box %ld region %ld\n", state->loginData.userID.box.box, state->loginData.userID.box.region);
	Logmsg("UserID = %d\n", state->loginData.userID.userID);
	Logmsg("User name = %s\n", state->loginData.userID.userName);
	Logmsg("Home town = %s\n", state->loginData.userID.userTown);
	Logmsg("Box flags = %ld\n", state->boxOSState.boxFlags);
//	Logmsg("colortable = %ld\n", (long)state->loginData.userID.colortable);
	Logmsg("Phone number = %s\n", state->boxPhoneNumber.phoneNumber);
	Logmsg("Num mails in box = %ld\n", (long)state->loginData.numMailsInBox);
	for(a = 0; a < state->loginData.numMailsInBox; a++)
		Logmsg("    serialno: %ld\n", (long)state->loginData.mailSerialNumbers[a]);

	Logmsg("Server_ReceiveLogin done\n");
	return(kServerFuncOK);
}


//
// Receive the login message from the box
//
int Server_ReceiveLogin(ServerState *state)
{
unsigned char 	opCode;
register short	a;

	Logmsg("Server_ReceiveLogin\n");

	if(Server_TReadDataSync(state->session, 1, (Ptr)&opCode) != noErr)
		return(kServerFuncAbort);

	if(opCode != msLogin){
		// fucked
		Logmsg("Wrong opCode.  Expected %d, got %d\n", msLogin, opCode);
		return(kServerFuncAbort);
	}

	Server_TReadDataSync(state->session, sizeof(long), (Ptr)&state->boxOSState.osState );
	Server_TReadDataSync(state->session, sizeof(long), (Ptr)&state->boxOSState.lastBoxState );


	// == BRAIN DAMAGE ==  6/10/94
	//
	// This simulates an ANI notification of the phone number.  Normally it will not be sent by the
	// box (especially not at every connection!).  It will be found by the 1-800 server by calling
	// ANI_GetPhoneNumber(&state->boxPhoneNumber);
	//
	Server_TReadDataSync(state->session, sizeof(phoneNumber), (Ptr)&state->boxPhoneNumber );
	state->boxPhoneNumber.phoneNumber[kPhoneNumberSize - 1] = 0;	// extraneous.

	Server_TReadDataSync( state->session, sizeof(userIdentification), (Ptr)(Ptr)&state->loginData.userID);
	state->loginData.userID.userName[kUserNameSize - 1] = 0;		// extraneous.

	Server_TReadDataSync( state->session, sizeof(short), (Ptr)&state->loginData.numMailsInBox);

	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	ASSERT_MESG(state->loginData.numMailsInBox <= kMaxInBoxEntries, "Fatal Box Error: numMailsInBox must be <= kMaxInBoxEntries");

	for(a = 0; a < state->loginData.numMailsInBox; a++)
		Server_TReadDataSync( state->session, sizeof(short), (Ptr)&state->loginData.mailSerialNumbers[a]);

	Server_TReadDataSync( state->session, sizeof(long), (Ptr)&state->loginData.validationToken);

	state->validFlags |= kServerValidFlag_BoxPhoneNumber;
	state->validFlags |= kServerValidFlag_Login;

	Logmsg("UserID box serial number = box %ld region %ld\n", state->loginData.userID.box.box, state->loginData.userID.box.region);
	Logmsg("UserID = %d\n", state->loginData.userID.userID);
	Logmsg("User name = %s\n", state->loginData.userID.userName);
	Logmsg("Home town = %s\n", state->loginData.userID.userTown);
//	Logmsg("serverUniqueID = %ld\n", (long)state->loginData.userID.serverUniqueID);
	Logmsg("Phone number = %s\n", state->boxPhoneNumber.phoneNumber);
	Logmsg("Num mails in box = %ld\n", (long)state->loginData.numMailsInBox);
	for(a = 0; a < state->loginData.numMailsInBox; a++)
		Logmsg("    serialno: %ld\n", (long)state->loginData.mailSerialNumbers[a]);

	Logmsg("Server_ReceiveLogin done\n");
	return(kServerFuncOK);
}

/*
void DoSendLoginMessageOpCode( void * notUsed )
{
phoneNumber	*thePhoneNumber;
userName	*theUserName;
messOut		sendDummy;
long		boxSerialNumber;

	sendDummy = msLogin;
	SendNetDataASync( msOutgoingOpCodeSize, (Ptr)&sendDummy );
	thePhoneNumber = GetBasePhoneNumber( );
	theUserName = GetUserName( GetCurUserID() );
	boxSerialNumber = GetBoxSerialNumber();
	SendNetDataASync( 4, (Ptr)&boxSerialNumber );
	SendNetDataASync( sizeof(phoneNumber), (Ptr)thePhoneNumber );
	SendNetDataASync( kUserNameSize, (Ptr)theUserName );
}

*/
