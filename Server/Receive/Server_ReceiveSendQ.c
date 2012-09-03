
/*
	File:		Server_ReceiveSendQ.c

	Contains:	Server receive whatever is in the box's send queue

	Written by:	Dave Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<25>	  9/3/94	ATM		Set a string to keep random chars out of log.
		<24>	 8/28/94	ATM		Mail update for segb.
		<23>	 8/26/94	ATM		Added bulletproofing to mail; long "to" and "title" line were
									hosing us.
		<22>	 8/25/94	ATM		Added some more logging stuff.
		<21>	 8/20/94	DJ		serverUniqueID is in playerinfo, not userIdentification
		<20>	 8/20/94	DJ		receiveaddressverifications2 handles addr verif only for the
									current user
		<19>	 8/12/94	ATM		Converted to Logmsg.
		<18>	  8/5/94	DJ		added kserverflagvalid_addrbook
		<17>	  8/5/94	DJ		all new addr book poo
		<16>	  8/4/94	DJ		no more SNDQElement
		<15>	  8/3/94	DJ		minor tweak to receiving addr book stuff
		<14>	  8/2/94	DJ		updated to latest mail sending scheme
		<13>	 7/20/94	DJ		added Server_Comm stuff
		<12>	 7/18/94	DJ		new addr book stuff
		<11>	 7/17/94	BET		Make printf's go to gLogFile instead of stdout, which is the
									modem.
		<10>	 7/14/94	DJ		receiving mail
		 <9>	  7/8/94	DJ		i don't remember
		 <8>	  7/6/94	DJ		added Server_ReceiveAddressBookValidationQueries
		 <7>	  7/3/94	DJ		error tolerant
		 <6>	  7/1/94	DJ		making server handle errors from the comm layer
		 <5>	 6/30/94	DJ		updated to new sendq format
		 <4>	  6/9/94	BET		Fix hairy compiles based on TransportSlayer.h
		 <3>	  6/4/94	DJ		making everything take a ServerState instead of SessionRec
		 <2>	  6/1/94	DJ		wasn't mallocing enuf mems!
		 <1>	 5/25/94	DJ		first checked in
	To Do:
*/

#include "ServerState.h"
#include "Server.h"
#include "Messages.h"
#include "UsrConfg.h"
#include "Server_Comm.h"
#include <stdio.h>
#include <stdlib.h>


int Server_ReceiveSendQ(ServerState *state)
{

unsigned char 	opCode;
long i;

	Logmsg("Server_ReceiveSendQ\n");

	if(Server_TReadDataSync( state->session, 1, (Ptr)&opCode ) != noErr)
		return(kServerFuncAbort);

	if(opCode != msSendSendQElements){
		// fucked
		return(kServerFuncAbort);
	}
	
	if(Server_TReadDataSync( state->session, sizeof(short), (Ptr)&state->sendQData.count ) != noErr)
		return(kServerFuncAbort);
		
	Logmsg("Receiving %ld items\n", (long)state->sendQData.count);
	
	if(state->sendQData.count < 1)
	{
		state->sendQData.items = NULL;
		return(kServerFuncOK);
	}

	state->sendQData.items = (QItem *)malloc(state->sendQData.count * sizeof(QItem));
	if(!state->sendQData.items){
		// majorly fucked... out of memory
		Logmsg("Out of memory.. fucked!\n");
		return(kServerFuncAbort);
	}

	state->validFlags |= kServerValidFlag_SendQ;

	for(i = 0; i < state->sendQData.count; i++)
	{
		if(Server_TReadDataSync( state->session, sizeof(DBID), (Ptr)&state->sendQData.items[i].theID ) != noErr)
			return(kServerFuncAbort);
		if(Server_TReadDataSync( state->session, sizeof(long), (Ptr)&state->sendQData.items[i].size ) != noErr)
			return(kServerFuncAbort);
		state->sendQData.items[i].data = (unsigned char *)malloc(state->sendQData.items[i].size);
		if(!state->sendQData.items[i].data){
			// majorly fucked
			Logmsg("Out of memory reading a SendQ item\n");
			return(kServerFuncAbort);
		}
		
		if(Server_TReadDataSync(state->session, 
						state->sendQData.items[i].size, 
						(Ptr)state->sendQData.items[i].data ) != noErr)
			return(kServerFuncAbort);

		Logmsg("Read SendQ item of dbid %ld, size %ld\n", (long)state->sendQData.items[i].theID, state->sendQData.items[i].size);

	}

	Logmsg("Server_ReceiveSendQ done\n");

	return(kServerFuncOK);
}


//
// 8/19/94.
// This new version supports update addr book only for current user.
//
int Server_ReceiveAddressBookValidationQueries2(ServerState *state)
{
unsigned char 	opCode, length;
short 	i;

	Logmsg("Server_ReceiveAddressBookValidationQueries\n");

	if(Server_TReadDataSync( state->session, 1, (Ptr)&opCode ) != noErr)
		return(kServerFuncAbort);

	if(opCode != msSendAddressesToVerify){
		// fucked
		return(kServerFuncAbort);
	}

	if(Server_TReadDataSync( state->session, sizeof(short), (Ptr)&state->addrValidationData.count ) != noErr)
		return(kServerFuncAbort);

	state->validFlags |= kServerValidFlag_AddressBook;

	Logmsg("Receiving %ld items\n", (long)state->addrValidationData.count);

	if(state->addrValidationData.count < 1)
	{
		state->addrValidationData.items = NULL;
		return(kServerFuncOK);
	}

	state->addrValidationData.items = (AddrValidItem *)malloc(state->addrValidationData.count * sizeof(AddrValidItem));
	if(!state->addrValidationData.items){
		// majorly fucked... out of memory
		Logmsg("Out of memory.. fucked!\n");
		return(kServerFuncAbort);
	}

	for(i = 0; i < state->addrValidationData.count; i++)
	{
		state->addrValidationData.items[i].ownerUserID = state->loginData.userID.userID;

		if(Server_TReadDataSync( state->session, sizeof(unsigned char), (Ptr)&state->addrValidationData.items[i].serverUniqueID ) != noErr)
			return(kServerFuncAbort);

		if (state->addrValidationData.items[i].serverUniqueID == kUncorrelatedEntry)
		{
			if(Server_TReadDataSync( state->session, sizeof(BoxSerialNumber), (Ptr)&state->addrValidationData.items[i].userIdent.box ) != noErr)
				return(kServerFuncAbort);
			if(Server_TReadDataSync( state->session, sizeof(unsigned char), (Ptr)&state->addrValidationData.items[i].userIdent.userID ) != noErr)
				return(kServerFuncAbort);

			if(state->addrValidationData.items[i].userIdent.box.box == -1)
			{
				if(Server_TReadDataSync( state->session, sizeof(char), (Ptr)&length ) != noErr)
					return(kServerFuncAbort);
				if(Server_TReadDataSync( state->session, (long)length, (Ptr)state->addrValidationData.items[i].userIdent.userName ) != noErr)
					return(kServerFuncAbort);
	
				ASSERT_MESG(length <= kUserNameSize, "why is length bigger than the name field?  memory trasher!");

				if(length >= kUserNameSize)	// NULL terminate cuz box doesn't send the NULL
					length = kUserNameSize - 1;

				state->addrValidationData.items[i].userIdent.userName[length] = 0;
			}
		}
	}

	Logmsg("Server_ReceiveAddressBookValidationQueries done.  Read %ld items\n", (long)state->addrValidationData.count);

	return(kServerFuncOK);
}



int Server_ReceiveAddressBookValidationQueries(ServerState *state)
{
unsigned char 	opCode, length, ownerUserID;
short 	i, j;
short	numForThisUser;

	Logmsg("Server_ReceiveAddressBookValidationQueries\n");

	if(Server_TReadDataSync( state->session, 1, (Ptr)&opCode ) != noErr)
		return(kServerFuncAbort);

	if(opCode != msSendAddressesToVerify){
		// fucked
		return(kServerFuncAbort);
	}

	if(Server_TReadDataSync( state->session, sizeof(short), (Ptr)&state->addrValidationData.count ) != noErr)
		return(kServerFuncAbort);

	state->validFlags |= kServerValidFlag_AddressBook;

	Logmsg("Receiving %ld items\n", (long)state->addrValidationData.count);

	if(state->addrValidationData.count < 1)
	{
		state->addrValidationData.items = NULL;
		return(kServerFuncOK);
	}

	state->addrValidationData.items = (AddrValidItem *)malloc(state->addrValidationData.count * sizeof(AddrValidItem));
	if(!state->addrValidationData.items){
		// majorly fucked... out of memory
		Logmsg("Out of memory.. fucked!\n");
		return(kServerFuncAbort);
	}

	for(i = 0; i < state->addrValidationData.count; )
	{
		if(Server_TReadDataSync( state->session, sizeof(char), (Ptr)&ownerUserID ) != noErr)
			return(kServerFuncAbort);
		if(Server_TReadDataSync( state->session, sizeof(short), (Ptr)&numForThisUser ) != noErr)
			return(kServerFuncAbort);

		for(j = 0; j < numForThisUser; j++, i++)
		{
			state->addrValidationData.items[i].ownerUserID = ownerUserID;


			if(Server_TReadDataSync( state->session, sizeof(unsigned char), (Ptr)&state->addrValidationData.items[i].serverUniqueID ) != noErr)
				return(kServerFuncAbort);

			if (state->addrValidationData.items[i].serverUniqueID == kUncorrelatedEntry)
			{
				if(Server_TReadDataSync( state->session, sizeof(BoxSerialNumber), (Ptr)&state->addrValidationData.items[i].userIdent.box ) != noErr)
					return(kServerFuncAbort);
				if(Server_TReadDataSync( state->session, sizeof(unsigned char), (Ptr)&state->addrValidationData.items[i].userIdent.userID ) != noErr)
					return(kServerFuncAbort);

				if(state->addrValidationData.items[i].userIdent.box.box == -1)
				{
					if(Server_TReadDataSync( state->session, sizeof(char), (Ptr)&length ) != noErr)
						return(kServerFuncAbort);
					if(Server_TReadDataSync( state->session, (long)length, (Ptr)state->addrValidationData.items[i].userIdent.userName ) != noErr)
						return(kServerFuncAbort);
		
					ASSERT_MESG(length <= kUserNameSize, "why is length bigger than the name field?  memory trasher!");

					if(length >= kUserNameSize)	// NULL terminate cuz box doesn't send the NULL
						length = kUserNameSize - 1;

					state->addrValidationData.items[i].userIdent.userName[length] = 0;
				}
			}
		}
	}

	Logmsg("Server_ReceiveAddressBookValidationQueries done.  Read %ld items\n", (long)state->addrValidationData.count);

	return(kServerFuncOK);
}


// Receive mail from seg9 or earlier.
//
int Server_SEG9_ReceiveMail(ServerState *state)
{
unsigned char 	opCode;
short			i, toSize, titleSize, mailSize;
Mail			m;
char			tmpBuf[128];		// self-defense measure

	Logmsg("Server_ReceiveMail\n");

	if(Server_TReadDataSync( state->session, 1, (Ptr)&opCode ) != noErr)
		return(kServerFuncAbort);

	if(opCode != msSendOutgoingMail){
		// fucked
		return(kServerFuncAbort);
	}

	if(Server_TReadDataSync( state->session, sizeof(short), (Ptr)&state->incomingMail.count ) != noErr)
		return(kServerFuncAbort);

	state->validFlags |= kServerValidFlag_IncomingMail;

	if(state->incomingMail.count < 1)
	{
		state->incomingMail.mailItems = NULL;
		return(kServerFuncOK);
	}

	state->incomingMail.mailItems = (MailItem *)malloc(state->incomingMail.count * sizeof(MailItem));
	if(!state->incomingMail.mailItems)
	{
		Logmsg("out of mems in reading mail\n");
		return(kServerFuncAbort);
	}

	for(i = 0; i < state->incomingMail.count; i++)
		state->incomingMail.mailItems[i].mail = NULL;


	for(i = 0; i < state->incomingMail.count; i++)
	{
		m.from.box.box = -1;
		m.from.box.region = -1;
		m.from.userID = state->loginData.userID.userID;		// wrong, but fixed
		if(Server_TReadDataSync( state->session,
									sizeof(BoxSerialNumber),
									(Ptr)&m.to.box) != noErr)
			return(kServerFuncAbort);
		Logmsg("NEW THING: (old) from boxid = %d\n", m.from.userID);	// remove me

		if(Server_TReadDataSync( state->session,
									sizeof(unsigned char),
									(Ptr)&m.to.userID) != noErr)
			return(kServerFuncAbort);

		if(Server_TReadDataSync( state->session,
									sizeof(short),
									(Ptr)&toSize) != noErr)
			return(kServerFuncAbort);

		if (toSize > 128) {
			Logmsg("MAIL DEBUG: ERROR: toSize = %ld\n", (long)toSize);
			return (kServerFuncAbort);
		}
		if(Server_TReadDataSync( state->session,
									(long)toSize,
									(Ptr)tmpBuf /*(Ptr)&m.to.userName*/) != noErr)
			return(kServerFuncAbort);
		strncpy(m.to.userName, tmpBuf, kUserNameSize);
		m.to.userName[kUserNameSize-1] = '\0';

		if(Server_TReadDataSync( state->session,
									sizeof(short),
									(Ptr)&titleSize) != noErr)
			return(kServerFuncAbort);

		if (titleSize > 128) {
			Logmsg("MAIL DEBUG: ERROR: titleSize = %ld\n", (long)titleSize);
			return (kServerFuncAbort);
		}
		if(Server_TReadDataSync( state->session,
									(long)titleSize,
									(Ptr)tmpBuf /*&m.title*/) != noErr)
			return(kServerFuncAbort);
		strncpy(m.title, tmpBuf, kTitleSize);
		m.title[kTitleSize-1] = '\0';

		if(Server_TReadDataSync( state->session,
									sizeof(short),
									(Ptr)&mailSize) != noErr)
			return(kServerFuncAbort);

//		m.from = state->loginData.userID;	// create the from addr
											// actually, this is done in Server_ProcessIncomingMail
											// cuz ValidateLogin may have changed the account username
											// due to uniquification of the userName.

		state->incomingMail.mailItems[i].size = sizeof(Mail) + mailSize -1;

		state->incomingMail.mailItems[i].mail = (Mail *)malloc((long)state->incomingMail.mailItems[i].size);
		if(!state->incomingMail.mailItems[i].mail)
		{
			Logmsg("out of mems reading mail\n");
			return(kServerFuncAbort);
		}

		*state->incom