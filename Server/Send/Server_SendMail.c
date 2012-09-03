
/*
	File:		Server_SendMail.c

	Contains:	Server send mail function

	Written by:	Dave Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<23>	  9/2/94	DJ		tracking mail serial numbers in playerAccount->mailInBox[]
		<22>	 8/29/94	ATM		Fixed one of them thar plural dialogs.
		<21>	 8/28/94	ATM		Make tired company president happy by handling mono/plural stuff
									in mail dialogs.
		<20>	 8/22/94	DJ		groomed dialog text
		<19>	 8/17/94	DJ		send "no new  mail" only for a mail only connect
		<18>	 8/15/94	DJ		sticky dialogs for mail only connect
		<17>	 8/12/94	DJ		updated to new userIdentification
		<16>	 8/12/94	ATM		Converted to Logmsg.
		<15>	  8/8/94	DJ		SendDialog takes boolean whether to stick or disappear in 3 sec.
		<14>	  8/3/94	DJ		jizzle jozzle
		<13>	  8/3/94	DJ		new mail sending (cuz Kon's new mail shit wasn't compatible with
									x-platform)
		<12>	  8/2/94	DJ		update to latest mail sending scheme (added SendMailBody
		<11>	 7/20/94	DJ		added Server_Comm stuff
		<10>	 7/17/94	BET		Make printf's go to gLogFile instead of stdout, which is the
									modem.
		 <9>	 7/15/94	DJ		send dialogs telling box how much mail was downloaded, how much
									left on server, etc
		 <8>	 7/12/94	DJ		using Server_TCheckError instead of TCheckError
		 <7>	  7/1/94	DJ		making server handle errors from the comm layer
		 <6>	 6/30/94	DJ		added broadcast mail
		 <5>	  6/9/94	BET		Fix hairy compiles based on TransportSlayer.h
		 <4>	  6/5/94	DJ		check numMailsInBox and don't send box more than will fit in his
									inbox (kMaxNumMails)
		 <3>	  6/5/94	DJ		making everything take a ServerState instead of SessionRec and
									added Server_DebugService
		 <2>	 5/31/94	DJ		send mail to box
		 <1>	 5/25/94	DJ		first checked in
	To Do:
*/

#include "ServerCore.h"
#include "Server.h"
#include "ServerDataBase.h"
#include "Server_Comm.h"

#include "Messages.h"
#include "Mail.h"
#include "Challnge.h"

#include <stdio.h>
#include <string.h>


static int Server_SendMailBody(ServerState *state, Mail *mail);

//
// 7/1/94 A prob with Server_SendMail is that if mail fails while sending the broadcast mail,
// the dbase has already marked it as sent (the user account keeps a timestamp of the
// last broadcast mail sent).  Oh well.  Bummer.
//
int Server_SendMail(ServerState *state)
{
short	numMails, numBroadcastMails, numTotalMails, i, numLeftOnServer, inBox;
messOut	opCode;
Mail	*mail;
char	msg[200];
Boolean	stickyDialog;


	// Make dialog stick on screen if mail only connect.
	//
	if(state->challengeData.userID.box.box == kDownloadOnlyMailSerialNumber)
		stickyDialog = true;
	else
		stickyDialog = false;



	Logmsg("Server_SendMail\n");
	if(Server_DebugService(state) != kServerFuncOK)
		return(kServerFuncAbort);


	//
	// Update the player account with serial num of mails on the box.
	//
	for(inBox = 0; inBox < kMaxInBoxEntries; inBox++)
		state->account->playerAccount.mailInBox[inBox] = 0;
	for(inBox = 0; inBox < state->loginData.numMailsInBox; inBox++)
		state->account->playerAccount.mailInBox[inBox] = state->loginData.mailSerialNumbers[inBox];






	// check if there is generic mail to send to everyone.
	//
	numBroadcastMails = DataBase_GetNumBroadcastMail(&state->loginData.userID);
	//
	// adjust numMails so that there are never more than kMaxInBoxEntries mails in box's inbox.
	//
	numLeftOnServer = 0;
	i = state->loginData.numMailsInBox + numBroadcastMails;
	if(i > kMaxInBoxEntries)
	{
		i -= kMaxInBoxEntries;
		numBroadcastMails -= i;
		numLeftOnServer += i;
	}
	
	// check if there is mail in this user's input queue.
	//
	numMails = DataBase_GetNumIncomingMail(&state->loginData.userID);
	//
	// adjust numMails so that there are never more than kMaxInBoxEntries mails in box's inbox.
	//
	i = state->loginData.numMailsInBox + numBroadcastMails + numMails;
	if(i > kMaxInBoxEntries)
	{
		i -= kMaxInBoxEntries;
		numMails -= i;
		numLeftOnServer += i;
	}
	
	numTotalMails = numMails + numBroadcastMails;
	if(numTotalMails > 0)
	{
		Server_SetTransportHold(true);

		opCode = msReceiveMail;
		Server_TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);

		Server_TWriteDataSync(state->session, sizeof(short), (Ptr)&numTotalMails);

		for(i = 0; i < numBroadcastMails; i++)
		{
			mail = DataBase_GetBroadcastMail(&state->loginData.userID, i);
			Server_SendMailBody(state, mail);
			if(Server_TCheckError() != noErr)
				return(kServerFuncAbort);

			// add a record of it into the playerAccount.
			// Do we need some kind of differentiator for broadcast mail serial numbers?
			state->account->playerAccount.mailInBox[inBox++] = mail->serialNumber;
		}

		for(i = 0; i < numMails; i++)
		{
			mail = DataBase_GetIncomingMail(&state->loginData.userID, i);
			Server_SendMailBody(state, mail);
			if(Server_TCheckError() != noErr)
				return(kServerFuncAbort);

			// add a record of it into the playerAccount.
			state->account->playerAccount.mailInBox[inBox++] = mail->serialNumber;

			DataBase_MarkMailAsSent(&state->loginData.userID, i);
		}

		if(numLeftOnServer)
		{
			if (numTotalMails == 1)
				strcpy(msg, "One new mail message has been added to your mailbox.");
			else
				sprintf(msg, "%ld new mail messages have been added to your mailbox.",
					(long)numTotalMails);

			if (numLeftOnServer == 1)
				strcat(msg, "Your mailbox is full, and there is one message waiting to be sent to you the next time you connect to XBAND.");
			else
				sprintf(msg+strlen(msg), "Your mailbox is full, and there are %ld messages waiting to be sent to you next time you connect to XBAND.",
				(long)numLeftOnServer);
			Server_SendLargeDialog(state, msg, stickyDialog);
		}
		else
		{
			if (numTotalMails == 1)
				strcpy(msg, "One new mail message has been added to your mailbox.");
			else
				sprintf(msg, "%ld new mail messages have been added to your mailbox.",
					(long)numTotalMails);
			Server_SendDialog(state, msg, stickyDialog);
		}
		
		Server_SetTransportHold(false);

	} else
	{
		if(numLeftOnServer)
		{
			if (numLeftOnServer == 1)
				strcat(msg, "Your mailbox is full, and there is one message waiting to be sent to you the next time you connect to XBAND.");
			else
				sprintf(msg, "Your mailbox is full, and there are %ld messages waiting to be sent to you next time you connect to XBAND.",
				(long)numLeftOnServer);
			Server_SendLargeDialog(state, msg, stickyDialog);
		}
		else
		{
			if(state->challengeData.userID.box.box == kDownloadOnlyMailSerialNumber)
				Server_SendDialog(state, "You have no new mail.", stickyDialog);
		}
	}
	
	DataBase_RemoveSentMail(&state->loginData.userID);

	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	return(kServerFuncOK);
}

static int Server_SendMailBody(ServerState *state, Mail *mail)
{
short			mailSize;
unsigned char	length;
short			messageSize;

	mailSize = (short)DataBase_SizeofMail(mail);

	Server_TWriteDataSync(state->session, sizeof(short), (Ptr)&mailSize);

	Server_SendUserIdentification(state, &mail->from);

	// don't have to send the 'to' userIdentification
	
	Server_TWriteDataSync(state->session, sizeof(short), (Ptr)&mail->serialNumber);
	Server_TWriteDataSync(state->session, sizeof(long), (Ptr)&mail->date);

	length = strlen(mail->title) + 1;
	Server_TWriteDataSync(state->session, sizeof(unsigned char), (Ptr)&length);
	Server_TWriteDataSync(state->session, (long)length, (Ptr)&mail->title);

	messageSize = strlen(mail->message) + 1;
	Server_TWriteDataSync(state->session, sizeof(short), (Ptr)&messageSize);
	Server_TWriteDataSync(state->session, (long)messageSize, (Ptr)mail->message);

	return(kServerFuncOK);
}

int Server_SendUserIdentification(ServerState *state, userIdentification *userID)
{
unsigned char length;

	Server_TWriteDataSync(state->session, sizeof(BoxSerialNumber), (Ptr)&userID->box);
	Server_TWriteDataSync(state->session, sizeof(unsigned char), (Ptr)&userID->userID);
	Server_TWriteDataSync(state->session, sizeof(DBID), (Ptr)&userID->ROMIconID);

	length = strlen(userID->userTown) + 1;
	Server_TWriteDataSync(state->session, sizeof(unsigned char), (Ptr)&length);
	Server_TWriteDataSync(state->session, (long)length, (Ptr)userID->userTown);

	length = strlen(userID->userName) + 1;
	Server_TWriteDataSync(state->session, sizeof(unsigned char), (Ptr)&length);
	Server_TWriteDataSync(state->session, (long)length, (Ptr)userID->userName);

	return(kServerFuncOK);
}
