/*
	File:		Server_ReceiveDebitCard.c

	Contains:	xxx put contents here xxx

	Written by:	Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <9>	 8/26/94	ATM		Added SEG8 versions for new as-yet-undefined XBANDCard struct.
		 <8>	 8/16/94	DJ		added receivecreditdebitinfointostruct
		 <7>	 8/12/94	ATM		Converted to Logmsg.
		 <6>	  8/2/94	DJ		free token is now in debitcardinfo not a separate lon
		 <5>	 7/29/94	DJ		more printf info
		 <4>	 7/27/94	DJ		CreditToken is now a long
		 <3>	 7/20/94	DJ		added Server_Comm stuff
		 <2>	 7/18/94	DJ		receiveCreditDebitInfo
		 <1>	 7/17/94	DJ		first checked in

	To Do:
*/

#include "ServerCore.h"
#include "Server.h"
#include "Messages.h"
#include "Server_Comm.h"
#include <stdio.h>

#include "SmartCard.h"


// temporary thang
int Server_SEG8_ReceiveCreditDebitInfoIntoStruct(ServerState *state, CreditDebitInfo *creditDebitInfo);
int Server_SEG8_ReceiveCreditDebitInfo(ServerState *state);

int Server_SEG8_ReceiveCreditDebitInfo(ServerState *state)
{
int err;

	Logmsg("Server_SEG8_ReceiveCreditDebitInfo\n");

	if (state->boxOSState.boxType > kBoxType8) {
		Logmsg("HOLY FUCK we're in the wrong place, boxType = 0x.8lx\n",
			state->boxOSState.boxType);
		abort();
	}
	err = Server_SEG8_ReceiveCreditDebitInfoIntoStruct(state, &state->creditDebitInfo);

	if(err == kServerFuncOK)
	{
		Logmsg("Server_SEG8_ReceiveCreditDebitInfo done\n");
		state->validFlags |= kServerValidFlag_CreditDebitInfo;
	}

	return(err);
}

// <SEG8 def of XBANDCard
typedef struct {
	long	token;			// a magic token may travel with this card
	short	type;			// kCardGPM896 or kCardGPM103
	short	rechargesLeft;	// times card can be recharged (if GPM896)
	short	creditsLeft;	// credits remaining on card after debit
	unsigned char problem;	// if problem token used, identifies "why" it was used.
	char	pad[1];			// pad out Mac to same size as Sparc unix.	
	long	serialNumber;	// every card has unique serial number for tracking
} SEG8_XBANDCard;

//
// helper routine for Server_ReceiveCreditDebitInfo and Server_SendDebitSmartcard
//
int Server_SEG8_ReceiveCreditDebitInfoIntoStruct(ServerState *state, CreditDebitInfo *creditDebitInfo)
{
unsigned char 	opCode;

	Server_TReadDataSync( state->session, sizeof(unsigned char), (Ptr)&opCode );
	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	if(opCode != msSendCreditDebitInfo){
		Logmsg("Wrong opCode.  Expected %d, got %d\n", msSendCreditDebitInfo, opCode);
		return(kServerFuncAbort);
	}

	Server_TReadDataSync( state->session, sizeof(unsigned char), (Ptr)&creditDebitInfo->usingType );

	switch(creditDebitInfo->usingType)
	{
		case kUsingCreditToken:
				Server_TReadDataSync( state->session, sizeof(SEG8_XBANDCard), (Ptr)&creditDebitInfo->debitCardInfo );
				Logmsg("Credit Token %ld\n", (long)creditDebitInfo->debitCardInfo.token);
			break;
		case kUsingDebitCard:
				Server_TReadDataSync( state->session, sizeof(SEG8_XBANDCard), (Ptr)&creditDebitInfo->debitCardInfo );
				Logmsg("Smart card serialno: %ld, credits left: %ld, recharges left: %ld\n",
						creditDebitInfo->debitCardInfo.serialNumber,
						(long)creditDebitInfo->debitCardInfo.creditsLeft,
						(long)creditDebitInfo->debitCardInfo.rechargesLeft);
			break;
		
		case kUsingCreditCard:
				Logmsg("Using credit card account\n");
			break;
		
		default:
				Logmsg("Unknown usingType #%ld!  Kill the connection.\n",
					creditDebitInfo->usingType);
				return(kServerFuncAbort);
			break;
	}

	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);


	return(kServerFuncOK);
}




int Server_ReceiveCreditDebitInfo(ServerState *state)
{
int err;

	Logmsg("Server_ReceiveCreditDebitInfo\n");

	if (state->boxOSState.boxType <= kBoxType8) {
		Logmsg("HOLY FUCK we're in the wrong place, boxType = 0x.8lx\n",
			state->boxOSState.boxType);
		abort();
	}
	err = Server_ReceiveCreditDebitInfoIntoStruct(state, &state->creditDebitInfo);

	if(err == kServerFuncOK)
	{
		Logmsg("Server_ReceiveCreditDebitInfo done\n");
		state->validFlags |= kServerValidFlag_CreditDebitInfo;
	}

	return(err);
}

//
// helper routine for Server_ReceiveCreditDebitInfo and Server_SendDebitSmartcard
//
int Server_ReceiveCreditDebitInfoIntoStruct(ServerState *state, CreditDebitInfo *creditDebitInfo)
{
unsigned char 	opCode;

	Server_TReadDataSync( state->session, sizeof(unsigned char), (Ptr)&opCode );
	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	if(opCode != msSendCreditDebitInfo){
		Logmsg("Wrong opCode.  Expected %d, got %d\n", msSendCreditDebitInfo, opCode);
		return(kServerFuncAbort);
	}

	Server_TReadDataSync( state->session, sizeof(unsigned char), (Ptr)&creditDebitInfo->usingType );

	switch(creditDebitInfo->usingType)
	{
		case kUsingCreditToken:
				Server_TReadDataSync( state->session, sizeof(XBANDCard), (Ptr)&creditDebitInfo->debitCardInfo );
				Logmsg("Credit Token %ld\n", (long)creditDebitInfo->debitCardInfo.token);
			break;
		case kUsingDebitCard:
				Server_TReadDataSync( state->session, sizeof(XBANDCard), (Ptr)&creditDebitInfo->debitCardInfo );
				Logmsg("Smart card serialno: %ld, credits left: %ld, recharges left: %ld\n",
						creditDebitInfo->debitCardInfo.serialNumber,
						(long)creditDebitInfo->debitCardInfo.creditsLeft,
						(long)creditDebitInfo->debitCardInfo.rechargesLeft);
			break;
		
		case kUsingCreditCard:
				Logmsg("Using credit card account\n");
			break;
		
		default:
				Logmsg("Unknown usingType #%ld!  Kill the connection.\n", creditDebitInfo->usingType);
				return(kServerFuncAbort);
			break;
	}

	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);


	return(kServerFuncOK);
}

