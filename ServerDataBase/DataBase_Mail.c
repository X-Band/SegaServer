/*
	File:		DataBase_Mail.c

	Contains:	xxx put contents here xxx

	Written by:	David Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<25>	 9/16/94	ATM		Added a sanity check where things are breaking strangely.
		<24>	 8/28/94	ATM		Changed something, I think.
		<23>	 8/24/94	ATM		Added reload stuff.
		<22>	 8/17/94	DJ		mail from xband has an icon now
		<21>	 8/13/94	ATM		Updated static userIdentification struct... def changed.
		<20>	 8/11/94	ATM		Improved error reporting.
		<19>	 8/11/94	ATM		Converted to Logmsg.
		<18>	 8/10/94	ATM		Added a warning message if BroadcastMail can't be found.
		<17>	  8/9/94	DJ		sends all bcst mail at once ifndef ONEATATIME
		<16>	  8/9/94	ATM		Added town field to broadcast mail.
		<15>	  8/8/94	ATM		Minor changes.
		<14>	  8/4/94	DJ		fixed scanning of longs & setting of timestamp
		<13>	  8/4/94	ATM		Added SegaDate stuff to broadcast mail.
		<12>	  8/3/94	DJ		fixed bug in broadcast mail loading (if no file to load) on mac
		<11>	  8/4/94	ATM		Broadcast mail loading.
		<10>	 7/19/94	DJ		set mail serialNumber
		 <9>	 7/17/94	BET		Make printf's go to gLogFile instead of stdout, which is the
									modem.
		 <8>	 7/15/94	DJ		something trivial
		 <7>	  7/8/94	DJ		broadcast mail sends a list, one at a time
		 <6>	  7/3/94	DJ		made it resend broadcast mail each time for testing purposes
		 <5>	 6/30/94	DJ		added broadcast mail
		 <4>	 6/11/94	DJ		some minor tweak or another
		 <3>	  6/5/94	DJ		added removal of sent mail and don't send more than kMaxInBox
		 <2>	 5/31/94	DJ		added all sorts of wonderful mail handling
		 <1>	 5/29/94	DJ		first checked in

	To Do:
*/


#include "Server.h"
#include "ServerDataBase.h"
#include "ServerDataBase_priv.h"
#include "Mail.h"
#include "Dates.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


Mail *Database_DuplicateMail(const Mail *mail)
{
Mail 	*copy;
long	mailSize;

	mailSize = sizeof(Mail) + strlen(mail->message);	// +1 taken care of by message[1]
	
	copy = (Mail *)malloc(mailSize);
	ASSERT_MESG(copy, "out of mems");
	
	if(copy)
	{
		*copy = *mail;
		strcpy(copy->message, mail->message);
	}
	
	return(copy);
}

long DataBase_SizeofMail(const Mail *mail)
{
	// +1 taken care of by message[1]
	return(sizeof(Mail) + strlen(mail->message));
}

//
// This routine doesn't use the userID yet, but it eventually will in order to
// filter and send targetted mail.
//
// a more sophisticated thing would be to go thru the list and count the number
// of mails who's sort id (which is a timestamp) is greater than some timestamp
// stored in the user account.
//
long DataBase_GetNumBroadcastMail(const userIdentification *userID)
{
long		count;
ListNode	*node;
SDBUser	*recvr;

	ASSERT(gSDB);
	ASSERT(gSDB->broadcastMail);

	recvr = Database_FindUser( userID );
	if(!recvr)
	{
		ERROR_MESG("Impl error: there should be a user account.. DataBase_CheckAccount should have been called");
		return(0);
	}

	node = GetFirstListNode(gSDB->broadcastMail);
	for(count = 0; node; node = GetNextListNode(node))
	{		
		if(GetSortedListNodeKey(node) > recvr->lastBroadcastMailSent)
			count++;
	}

#ifdef ONEATATIME
// BRAIN DAMAGE... this little hack sends 1 broadcast mail per connect.  See also DataBase_GetBroadcastMail
//
	return(count ? 1 : 0);	// hack
#else
	return(count);
#endif
}

long DataBase_GetNumIncomingMail(const userIdentification *userID)
{
SDBUser	*recvr;
long	numMails;

	recvr = Database_FindUser( userID );
	if(!recvr)
	{
		ERROR_MESG("Impl error: there should be a user account.. DataBase_CheckAccount should have been called");
		return(0);
	}
	
	ASSERT(recvr->incomingMail);
	numMails = NumListNodesInList(recvr->incomingMail);
	
	return(numMails);
}

Mail *DataBase_GetIncomingMail(const userIdentification *userID, long mailIndex)
{
SDBUser		*recvr;
Mail		*mail;
ListNode	*node;
long		i, numMails;
SDBMail		*header;


	recvr = Database_FindUser( userID );
	if(!recvr)
	{
		ERROR_MESG("Impl error: there should be a user account.. DataBase_CheckAccount should have been called");
		return(0);
	}
	
	numMails = NumListNodesInList(recvr->incomingMail);

	ASSERT(numMails > 0);
	
	if(mailIndex < 0 || mailIndex >= numMails)
	{
		ERROR_MESG("mailIndex not in range");
		return(NULL);
	}
	
	node = GetFirstListNode(recvr->incomingMail);
	for(i = 0; i < mailIndex; i++)
	{
		node = GetNextListNode(node);
		if(!node)
		{
			ERROR_MESG("Impl error: not as many mails in incoming as NumListNodesInList reported!?!");
			return(NULL);
		}
	}
	
	ASSERT(node);
	header = (SDBMail *)GetListNodeData(node);
	ASSERT(header);
	mail = header->mail;
	ASSERT(mail);

	return(mail);
}

//
// Will send a broadcast mail to a given user only once.  Uses mail timestamp and the
// lastBroadcastMailSent stamp in each user account to do this.
//
Mail *DataBase_GetBroadcastMail(const userIdentification *userID, long mailIndex)
{
SDBUser		*recvr;
Mail		*mail;
ListNode	*node, *node2;
long		i, numMails;
SDBMail		*header;


	recvr = Database_FindUser( userID );
	if(!recvr)
	{
		ERROR_MESG("Impl error: there should be a user account.. DataBase_CheckAccount should have been called");
		return(NULL);
	}

	numMails = NumListNodesInList(gSDB->broadcastMail);

	ASSERT(numMails > 0);
	
	if(mailIndex < 0 || mailIndex >= numMails)
	{
		ERROR_MESG("mailIndex not in range");
		return(NULL);
	}
	

#ifndef ONEATATIME

	for( node = GetFirstListNode(gSDB->broadcastMail); node; node = GetNextListNode(node))
	{
		if(GetSortedListNodeKey(node) > recvr->lastBroadcastMailSent)
			break;
	}

	if (node == NULL) {
		Logmsg("GLITCH: somebody called DataBase_GetBroadcastMail twice??\n");
		return (NULL);
	}
	
	ASSERT(node);
	header = (SDBMail *)GetListNodeData(node);
	ASSERT(header);
	mail = header->mail;
	ASSERT(mail);

	recvr->lastBroadcastMailSent = GetSortedListNodeKey(node);

	return(mail);

#else

	node = node2 = GetFirstListNode(gSDB->broadcastMail);
	for(i = -1; i < mailIndex; node2 = GetNextListNode(node))
	{
		node = node2;
		if(!node)
		{
			ERROR_MESG("Impl error: not as many mails in incoming as NumListNodesInList reported!?!");
			return(NULL);
		}
		
		if(GetSortedListNodeKey(node) > recvr->lastBroadcastMailSent)
			i++;
	}
	
	ASSERT(node);
	header = (SDBMail *)GetListNodeData(node);
	ASSERT(header);
	mail = header->mail;
	ASSERT(mail);

//
// BRAIN DAMAGE.  This hack sends the list of broadcast mails, one per connect, then starts
//					at the front of the list again!  Ha ha ha!
//

	node2 = node;
	if(node2)
		for(node2 = GetNextListNode(node2); node2; node2 = GetNextListNode(node2))
			i++;

	if(i == 0)
		recvr->lastBroadcastMailSent = 0;
	else
		recvr->lastBroadcastMailSent = GetSortedListNodeKey(node);

	return(mail);

#endif ONEATATIME
}


void DataBase_MarkMailAsSent( const userIdentification *userID, long mailIndex )
{
SDBUser		*recvr;
ListNode	*node;
long		i, numMails;
SDBMail		*header;


	recvr = Database_FindUser( userID );
	if(!recvr)
	{
		ERROR_MESG("Impl error: there should be a user account.. DataBase_CheckAccount should have been called");
		return;
	}
	
	numMails = NumListNodesInList(recvr->incomingMail);

	ASSERT(numMails > 0);
	
	if(mailIndex < 0 || mailIndex >= numMails)
	{
		ERROR_MESG("mailIndex not in range");
		return;
	}
	
	node = GetFirstListNode(recvr->incomingMail);
	for(i = 0; i < mailIndex; i++, node = GetNextListNode(node))
	{
		if(!node)
		{
			ERROR_MESG("Impl error: not as many mails in incoming as NumListNodesInList reported!?!");
			return;
		}
	}
	
	ASSERT(node);
	header = (SDBMail *)GetListNodeData(node);
	ASSERT(header);
	header->sentToBox = true;
}


void DataBase_RemoveSentMail( const userIdentification *userID )
{

SDBUser		*recvr;
ListNode	*node, *node2;
SDBMail		*header;


	recvr = Database_FindUser( userID );
	if(!recvr)
	{
		ERROR_MESG("Impl error: there should be a user account.. DataBase_CheckAccount should have been called");
		return;
	}

	node = GetFirstListNode(recvr->incomingMail);
	while(node)
	{
		node2 = GetNextListNode(node);

		header = (SDBMail *)GetListNodeData(node);
		ASSERT(header);

		if(header->sentToBox)
		{
			RemoveListNodeFromList(node);
			ASSERT(header->mail);
			free(header->mail);
			free(header);
			DisposeListNode(node);
		}
		
		node = node2;
	}
}



//
// Add Mail to the database
//
// KON... You really don't have to have both fields on a mail.  A single 'address' field
// will do.  Why?  because server knows who it came from by the account login.
// Sending: address = 'to'.
// server receives it and puts into the incoming mail queue of 'to' and changes address to 'from'
// Receiving: address = 'from'.
//
// dj 5/31/94
//
// WRONG!  Need the player number, in case the player switched before the
// message was sent.
//

Err DataBase_AddMailToIncoming(const Mail *mail)
{
SDBUser 	*sender, *recvr;
Mail		*m;
ListNode	*node;
SDBMail		*header;

	// Database_CheckAccount is called by Server_ValidateLogin to validate/setup a user account.
	//
	sender = Database_FindUser(  &mail->from );
	ASSERT_MESG(sender, "If Database_CheckAccount was called before FindUser, this call CANNOT FAIL!?!?");
	
	recvr = Database_FindUser( &mail->to );
	if(!recvr)
		return(kNoSuchUserAccount);

	m = Database_DuplicateMail(mail);
	m->serialNumber = recvr->mailSerialNumber++;		// Set the mail serial number.

	ASSERT(m);
	header = (SDBMail *)malloc(sizeof(SDBMail));
	ASSERT(header);
	header->mail = (Mail *)m;	// cast the const to make metrowerks happy.
	header->sentToBox = false;	

	node = NewSortedListNode((Ptr)header, Database_GetTimeStamp());
	AddListNodeToSortedList(recvr->incomingMail, node);
	Database_IncrementTimeStamp();

	return(kNoError);
}

// Non-API routine; does the dirty work.
Err DataBase_AddStampedMailToBroadcast(const Mail *mail, long timeStamp)
{
//SDBUser 	*sender;
Mail		*m;
ListNode	*node;
SDBMail		*header;

	// This mail is generally from Catapult...
	//
	//sender = Database_FindUser(  &mail->from );
	//ASSERT_MESG(sender, "If Database_CheckAccount was called before FindUser, this call CANNOT FAIL!?!?");

	m = Database_DuplicateMail(mail);
	m->serialNumber = 0;								// 0 serial number
	ASSERT(m);
	header = (SDBMail *)malloc(sizeof(SDBMail));
	ASSERT(header);
	header->mail = (Mail *)m;							// cast the const to make metrowerks happy.
	header->sentToBox = false;	
	
	// Timestamp the mail so we can avoid sending it twice to a given user.
	//
	//Database_IncrementTimeStamp();
	//node = NewSortedListNode((Ptr)header, Database_GetTimeStamp());
	node = NewSortedListNode((Ptr)header, timeStamp);
	AddListNodeToSortedList(gSDB->broadcastMail, node);

	//Logmsg("Added broadcast[%d] (%s: '%s')\n", timeStamp,
	//	mail->from.userName, mail->message);

	return(kNoError);
}

//
// We're not keeping track of the serial number for broadcast mail
//
Err DataBase_AddMailToBroadcast(const Mail *mail)
{
	Database_IncrementTimeStamp();
	DataBase_AddStampedMailToBroadcast(mail, Database_GetTimeStamp());
	return (kNoError);
}


//
// Load broadcast mail files into database.
//
// Syntax rules:
// - Blank lines and lines beginning with '#' are ignored.
// - Everything up to the first "%%" is ignored.
// - Every entry must be followed by "%%".  
// - First line is '!', eight 'x's, and '!'; this gets changed into a timestamp.
// - Second line is "from", third line is "town", fourth is "date" (year,
//   month, day), fifth line is "title", sixth is "message".  This
//   ordering is fixed; the two-character tags are there for sanity checking
//   and readability.
// - If garbage is detected, this routine scans ahead to the next %%.
//
// %%
// !xxxxxxxx!
// FR Catapult
// TN Happy Town
// DA 1994 8 3
// TI This is the title
// MS This is the message.  It must fit on a single line.
// %%
// !xxxxxxxx!
// FR Cataplot
// TN Another Town
// DA 1994 8 4
// TI This is the second title.
// MS This is yet another broadcast mail message.
// %%
//
#define MY_MAX_MAIL_MESG_SIZE 512

Err DataBase_LoadBroadcastMail()
{
enum { kLookForPercent, kTimeStamp, kFrom, kTown, kDate, kTitle, kMessage };
FILE	*fp;
char	linebuf[MY_MAX_MAIL_MESG_SIZE+4], stampbuf[9];
long	state, len, year, month, day;
Mail	*fakem;
long	cur_offset, stamp_offset;
long	timeStamp;
userIdentification	catapult = {{-1, -1}, 0, 0, kXBANDPlayerIcon, "Broadcast", "Catapult"};

	if ((fp = fopen(kSDB_BMail, "r+")) == NULL) {	// do NOT want "rb"
		Logmsg("NOTE: couldn't open '%s'\n", kSDB_BMail);
		return (kNoError);	// little white lie
	}
	
	fakem = (Mail *)malloc(sizeof(Mail) + MY_MAX_MAIL_MESG_SIZE);
	ASSERT(fakem);

	state = kLookForPercent;
	while (1) {
		fgets(linebuf, MY_MAX_MAIL_MESG_SIZE+4, fp);
		if (feof(fp) || ferror(fp)) break;

		if (linebuf[strlen(linebuf)-1] == '\n')
			linebuf[strlen(linebuf)-1] = '\0';
		if (linebuf[0] == '\0' || linebuf[0] == '#') continue;

		switch (state) {
		case kLookForPercent:
			if (strcmp(linebuf, "%%") == 0) {
				state = kTimeStamp;
				stamp_offset = ftell(fp);	// should be start of timestamp line
			}
			break;
		case kTimeStamp:
			if (linebuf[0] != '!' || linebuf[9] != '!') {
				Logmsg("Mail: expected timestamp, got '%s'\n", linebuf);
				state = kLookForPercent;
				break;
			}
			timeStamp = -1;
			if (strncmp("xxxxxxxx", linebuf+1, 8) != 0) {
				stamp_offset = 0;	// date already set, don't write to file
				(void) sscanf(linebuf+1, "%8x", &timeStamp);
			} else {
				timeStamp = -1;		// generate a new one
			}
			state = kFrom;
			break;
		case kFrom:
			if (strncmp(linebuf, "FR ", 3) != 0) {
				Logmsg("Mail: expected FR, got '%s'\n", linebuf);
				state = kLookForPercent;
				break;
			}
			// fake account with boxID(-1,-1) and userName from mail file
			strncpy(catapult.userName, linebuf+3, kUserNameSize-1);
			catapult.userName[kUserNameSize-1] = '\0';
			fakem->from = catapult;
			state = kTown;
			break;
		case kTown:
			if (strncmp(linebuf, "TN ", 3) != 0) {
				Logmsg("Mail: expected TN, got '%s'\n", linebuf);
				state = kLookForPercent;
				break;
			}
			strncpy(catapult.userTown, linebuf+3, kUserTownSize-1);
			catapult.userTown[kUserTownSize-1] = '\0';
			fakem->from = catapult;
			state = kDate;
			break;
		case kDate:
			if (strncmp(linebuf, "DA ", 3) != 0) {
				Logmsg("Mail: expected DA, got '%s'\n", linebuf);
				state = kLookForPercent;
				break;
			}
			if (sscanf(linebuf+3, "%ld %ld %ld", &year, &month, &day) != 3)
				state = kLookForPercent;
			month--;	// sega months start with 0.  sega days start with 1.
			fakem->date = Date(year, month, day);
			state = kTitle;
			break;
		case kTitle:
			if (strncmp(linebuf, "TI ", 3) != 0) {
				Logmsg("Mail: expected TI, got '%s'\n", linebuf);
				state = kLookForPercent;
				break;
			}
			strncpy(fakem->title, linebuf+3, kTitleSize-1);
			fakem->title[kTitleSize-1] = '\0';
			state = kMessage;
			break;
		case kMessage:
			if (strncmp(linebuf, "MS ", 3) != 0) {
				Logmsg("Mail: expected MS, got '%s'\n", linebuf);
				state = kLookForPercent;
				break;
			}
			strncpy(fakem->message, linebuf+3, MY_MAX_MAIL_MESG_SIZE-1);
			fakem->message[MY_MAX_MAIL_MESG_SIZE-1] = '\0';

			// we should have the full message now, so add it to the DB
			if (timeStamp == -1) {
				Database_IncrementTimeStamp();
				timeStamp = Database_GetTimeStamp();
			}
			DataBase_AddStampedMailToBroadcast(fakem, timeStamp);

			// if timestamp wasn't read in, back up and stuff it into the file
			if (stamp_offset) {
				cur_offset = ftell(fp);
				sprintf(stampbuf, "%.8lx", Database_GetTimeStamp());
				fseek(fp, stamp_offset+1, 0);	// math on offsets is evil
				len = fwrite(stampbuf, 1, 8, fp);
				fseek(fp, cur_offset, 0);
			}

			state = kLookForPercent;
			break;
		default:
			Logmsg("Bad state in BroadcastMail\n");
			goto error;
		}
	}

error:
	fclose(fp);
	free(fakem);
	return (kNoError);	// little white lie when exiting via error
}
#undef MY_MAX_MAIL_MESG_SIZE

void DataBase_PrintMail(const Mail *mail)
{
	Logmsg("Mail from: '%s' (%d,%d)\n", mail->from.userName,
		mail->from.box.box, mail->from.box.region);
	Logmsg("     to: '%s' (%d,%d)\n", mail->to.userName,
		mail->to.box.box, mail->to.box.region);
	Logmsg("     subject: '%s'\n", mail->title);
	Logmsg("     message: '%s'\n", mail->message);
}


void DataBase_ReloadBroadcastMail()
{
	gSDB->broadcastMail = NewSortedList();		// zot
	Logmsg("ERASED broadcast mail, reloading\n");
	(void) DataBase_LoadBroadcastMail();
}

