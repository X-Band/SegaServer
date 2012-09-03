/*
	File:		DataBase_Core.c

	Contains:	xxx put contents here xxx

	Written by:	David Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<34>	 9/18/94	ATM		Added numRankingInfo field.
		<33>	 9/16/94	ATM		boxFlags->boxModified, playerFlags->playerModified,
									acceptChallenges->playerFlags.  Added boxFlags.
		<32>	 9/12/94	DJ		changed LataLookup to POPLookup (looks up Compuserve POPs)
		<31>	 9/11/94	ATM		Changed how gRestore works (no more getenv).
		<30>	  9/3/94	ATM		Update load/save for new mailInBox stuff.
		<29>	 8/25/94	ATM		Add BoxAccount password to load/save.
		<28>	 8/22/94	ATM		Add in the netErrorTotals thang.
		<27>	 8/22/94	ATM		Fixed restore (added colorTableID read).
		<26>	 8/21/94	ATM		Updated save/restore with major BoxAccount and PlayerAccount
									changes.
		<25>	 8/19/94	ATM		Updated save/restore with ranking and other goodies.  Changed
									Server_SetupGamePatches to DataBase_LoadGamePatches.
		<24>	 8/15/94	ATM		Changed SDBBox to hold full BoxSerialNumber.
		<23>	 8/13/94	ATM		Another Statusmsg.
		<22>	 8/13/94	ATM		Statusmsg for shutdown.
		<21>	 8/12/94	ATM		Added hook for LoadFakeLata.
		<20>	 8/12/94	DJ		upgrade to latest error messages
		<19>	 8/12/94	ATM		Converted to Logmsg.
		<18>	 8/10/94	ATM		Added homeTown.
		<17>	  8/9/94	ATM		D-Bug.
		<16>	  8/8/94	ATM		Bug --> squash.
		<15>	  8/8/94	ATM		Added hook for loading News.
		<14>	  8/6/94	ATM		Added BoxAccount and PlayerAccount stuff to save/restore
									routines.
		<13>	  8/5/94	DJ		read/write on mac
		<12>	  8/4/94	ATM		Added Server_SetupGamePatches (moved from client side).
		<11>	  8/4/94	DJ		nuthing i think
		<10>	  8/4/94	ATM		Add call to load broadcast mail.
		 <9>	  8/3/94	DJ		fixed for mac
		 <8>	  8/3/94	ATM		Added gloppy database save & restore.
		 <7>	 7/20/94	DJ		no ServerState passed to dbase routines
		 <6>	 7/15/94	DJ		dick
		 <5>	  7/3/94	DJ		noting\
		 <4>	 6/30/94	DJ		added broadcast mail
		 <3>	 6/30/94	DJ		LoadSystemPatches (and no save/restore of dbase)
		 <2>	 6/11/94	DJ		ServerDataBase_SaveToDisk
		 <1>	 5/31/94	DJ		startup & shutdown the database

	To Do:
*/


#include <string.h>
#include <stdlib.h>

#define __DataBase_Core__

#include "ServerDataBase_priv.h"
#include "Errors.h"

SDB *gSDB;

// This gets set by premain().  Eventually this should get passed in
// as an argument.  Of course, eventually the CheeseDB should be
// gone entirely...
char *gRestoreFile = NULL;

Err ServerDataBase_Initialize(void)
{
FILE	*fp;
Err		result;

	if(!SDB_New())
		return(kOutOfMemoryErr);

	SDBSystem_LoadSystemPatches();
	
	// restore database from disk.
	//
	result = kNoError-1;		// just make it != kNoError
	if (gRestoreFile == NULL)
		fp = fopen(kSDBFILE, "rb");
	else
		fp = fopen(gRestoreFile, "rb");

	Logmsg("Restore file is '%s' (open %s)\n",
		(gRestoreFile == NULL) ? kSDBFILE : gRestoreFile,
		(fp == NULL) ? "FAILED" : "succeeded");

	if(fp)
	{
		result = SDB_Restore(fp);
		fclose(fp);
	}
	if (result == kNoError) {
		Logmsg("DB was restored\n");
		Statusmsg("segad  : RESTART (DB restored)\n");
	} else {
		Logmsg("DB was NOT restored\n");
		Statusmsg("segad  : RESTART (DB cleared)\n");
	}
	
	// load game patches
	DataBase_LoadGamePatches();

	// read broadcast mail in
	if ((result = DataBase_LoadBroadcastMail()) != kNoError)
		;		// should return error later

	// read news in
	if ((result = DataBase_LoadNewsPages()) != kNoError)
		;		// should return error later

	// read the fake LATA database
	if ((result = DataBase_LoadFakeLata()) != kNoError)
		;		// should return error later

	// read the Compuserver POP database
	if ((result = DataBase_LoadCompuservePOPs()) != kNoError)
		;		// should return error later


	return(kNoError);
}

Err ServerDataBase_Shutdown(void)
{
Err	err;

	ASSERT(gSDB);

	ServerDataBase_SaveToDisk();

	err = SDB_Dispose(gSDB);
	if(err != kNoError)
		return(err);

	gSDB = NULL;
		
	Statusmsg("segad  : SHUTDOWN (DB saved to disk)\n");

	//return(kNoError);
	exit(0);
}

void ServerDataBase_SaveToDisk(void)
{
FILE	*fp;
char	saveName[32];

	ASSERT(gSDB);

	// save database to disk.
	//
	//fp = fopen(kSDBFILE, "wb");

#ifdef unix
	sprintf(saveName, "ServerDB.%.8lx", time(0));
#else
	sprintf(saveName, kSDBFILE);
#endif

	fp = fopen(gRestoreFile, "wb");
	if(fp)
	{
		SDB_Save(fp);
		fclose(fp);
	}
}

SDB *SDB_New(void)
{
	gSDB = (SDB *)malloc(sizeof(SDB));
	
	if(!gSDB){
		ERROR_MESG("out of mems allocating database\n");
		return(NULL);
	}
	
	gSDB->system = SDBSystem_New();
	gSDB->ngp = SDBNGP_New();
	gSDB->games = SDBGames_New();
	gSDB->users = SDBUsers_New();
	gSDB->news = SDBNews_New();

	gSDB->broadcastMail = NewSortedList();

	return(gSDB);
}

Err SDB_Dispose(SDB *sdb)
{
	// Delete everything....
	// BRAIN DAMAGE.. haven't written dispose yet....

	return(kNoError);
}

SDBSystem *SDB_GetSystem(SDB *sdb)
{
	ASSERT(sdb);
	return(sdb->system);
}

SDBUsers *SDB_GetUsers(SDB *sdb)
{
	ASSERT(sdb);
	return(sdb->users);
}

SDBNGP *SDB_GetNGP(SDB *sdb)
{
	ASSERT(sdb);
	return(sdb->ngp);
}

SDBGames *SDB_GetGames(SDB *sdb)
{
	ASSERT(sdb);
	return(sdb->games);
}

SDBNews *SDB_GetNews(SDB *sdb)
{
	ASSERT(sdb);
	return(sdb->news);
}


// fwrite() a simple type with size=1 and nitems=sizeof(item) to "fp",
// then verify that the bytes were written.
#define ASSERT_FWRITE_ELEM(item) { \
		long len = fwrite(&(item), 1, sizeof(item), fp); \
		ASSERT(len == sizeof(item)); \
	}


Err SDB_Save(FILE *fp)
{
SDBCode	code;
ListNode	*lnode;
long	len;
long	count;

	//  write a header.
	code = kSDBCode_Server;
	len = fwrite(&code, 1, sizeof(SDBCode), fp);
	ASSERT(len == sizeof(SDBCode));

	//
	// Dump the contents of the server DB to a file.
	//

	// skip the system patches
	//SDBSystem_Save(fp);

	// skip the NGP stuff, the game info, and the broadcast mail

	// dump the SDBUsers stuff out to a file
	ASSERT_FWRITE_ELEM(gSDB->users->timeStamp);
	ASSERT_FWRITE_ELEM(gSDB->users->uniqueBoxSerialNumber);
	ASSERT_FWRITE_ELEM(gSDB->users->numUsers);

	// count up and write out the list of SDBBox structs
	count = NumListNodesInList(gSDB->users->list);
	ASSERT_FWRITE_ELEM(count);
	Logmsg("Dumping %ld box entries\n", count);

	lnode = GetFirstListNode(gSDB->users->list);
	while (count--) {
		SDBBox *box;
		BoxAccount *boxacc;
		int i, j;

		box = (SDBBox *)GetListNodeData(lnode);
		ASSERT_FWRITE_ELEM(box->boxSerialNumber);
		ASSERT_FWRITE_ELEM(box->boxPhoneNumber);

		// write BoxAccount struct
		boxacc = &(box->boxAccount);
		ASSERT_FWRITE_ELEM(boxacc->box);
		ASSERT_FWRITE_ELEM(boxacc->password);
		ASSERT_FWRITE_ELEM(boxacc->homeTown);
		ASSERT_FWRITE_ELEM(boxacc->gamePhone);
		ASSERT_FWRITE_ELEM(boxacc->popPhone);
		ASSERT_FWRITE_ELEM(boxacc->altPopPhone);
		ASSERT_FWRITE_ELEM(boxacc->gamePlatform);
		ASSERT_FWRITE_ELEM(boxacc->authCode);
		ASSERT_FWRITE_ELEM(boxacc->userCredits);
		ASSERT_FWRITE_ELEM(boxacc->freeCredits);
		for (i = 0; i < 7; i++)
			ASSERT_FWRITE_ELEM(boxacc->restrictInfo[i]);
		ASSERT_FWRITE_ELEM(boxacc->osLength);
		ASSERT_FWRITE_ELEM(boxacc->boxFlags);
		ASSERT_FWRITE_ELEM(boxacc->osVersion);
		ASSERT_FWRITE_ELEM(boxacc->ngpVersion);
		ASSERT_FWRITE_ELEM(boxacc->connections[0]);		// should be list
		ASSERT_FWRITE_ELEM(boxacc->netErrorTotals);
		ASSERT_FWRITE_ELEM(boxacc->debug0);
		ASSERT_FWRITE_ELEM(boxacc->debug1);
		ASSERT_FWRITE_ELEM(boxacc->debug2);
		ASSERT_FWRITE_ELEM(boxacc->debug3);

		// dump all four SDBUser structs
		for (i = 0; i < 4; i++) {
			SDBUser *user;
			PlayerAccount *placc;
			long mailcount;
			ListNode *mailnode;
			char user_exists;
			long slen, zero=0;

			user = box->users[i];

			user_exists = (user != NULL);
			ASSERT_FWRITE_ELEM(user_exists);
			if (!user_exists) continue;

			ASSERT_FWRITE_ELEM(user->userID);
			ASSERT_FWRITE_ELEM(user->mailSerialNumber);
			ASSERT_FWRITE_ELEM(user->lastBroadcastMailSent);

			// write PlayerAccount struct
			placc = &(user->playerAccount);
			ASSERT_FWRITE_ELEM(placc->magicID);
			ASSERT_FWRITE_ELEM(placc->userName);
			ASSERT_FWRITE_ELEM(placc->iconID);
			ASSERT_FWRITE_ELEM(placc->colorTableID);
			if (placc->openTaunt) {
				slen = strlen(placc->openTaunt);		// write len for strings
				ASSERT_FWRITE_ELEM(slen);
				if (slen) {
					len = fwrite(placc->openTaunt, 1, slen, fp);
					ASSERT(len == slen);
				}
			} else {
				ASSERT_FWRITE_ELEM(zero);
			}
/* -----
			if (placc->closeTaunt) {
				slen = strlen(placc->closeTaunt);		// write len for strings
				ASSERT_FWRITE_ELEM(slen);
				if (slen) {
					len = fwrite(placc->closeTaunt, 1, slen, fp);
					ASSERT(len == slen);
				}
			} else {
				ASSERT_FWRITE_ELEM(zero);
			}
----- */
			if (placc->personInfo) {
				slen = strlen(placc->personInfo);		// write len for strings
				ASSERT_FWRITE_ELEM(slen);
				if (slen) {
					len = fwrite(placc->personInfo, 1, slen, fp);
					ASSERT(len == slen);
				}
			} else {
				ASSERT_FWRITE_ELEM(zero);
			}
			ASSERT_FWRITE_ELEM(placc->password);
			ASSERT_FWRITE_ELEM(placc->birthday);
			ASSERT_FWRITE_ELEM(placc->playerFlags);
			ASSERT_FWRITE_ELEM(placc->customIconSize);
			if (placc->customIconSize) {
				len = fwrite(placc->customIcon, 1, placc->customIconSize, fp);
				ASSERT(len == placc->customIconSize);
			}
			// msgState, msgChannels
			ASSERT_FWRITE_ELEM(placc->autoMatchConnects);
			ASSERT_FWRITE_ELEM(placc->challengeConnects);
			ASSERT_FWRITE_ELEM(placc->msgCheckConnects);
			ASSERT_FWRITE_ELEM(placc->failedServerConnects);
			ASSERT_FWRITE_ELEM(placc->failedClientConnects);
			ASSERT_FWRITE_ELEM(placc->numRankingInfo);
			ASSERT_FWRITE_ELEM(placc->rankingInfo[0]);	// ouch!
			ASSERT_FWRITE_ELEM(placc->rankingInfo[1]);	// ick!
			for (j = 0; j < kMaxAddressBookEntries; j++)
				ASSERT_FWRITE_ELEM(placc->addressBook[j]);
			ASSERT_FWRITE_ELEM(placc->debug0);
			ASSERT_FWRITE_ELEM(placc->debug1);
			ASSERT_FWRITE_ELEM(placc->debug2);
			ASSERT_FWRITE_ELEM(placc->debug3);
			for (j = 0; j < kMaxInBoxEntries; j++)
				ASSERT_FWRITE_ELEM(placc->mailInBox[j]);

			// dump the user's mail
			mailcount = NumListNodesInList(user->incomingMail);
			ASSERT_FWRITE_ELEM(mailcount);

			mailnode = GetFirstListNode(user->incomingMail);
			while (mailcount--) {
				SDBMail *sdbmail;
				Mail *mail;
				long mlen, key;

				// Mail is sorted based on the server "TimeStamp", but the
				// value isn't explicitly included in the mail struct.
				key = GetSortedListNodeKey(mailnode);
				ASSERT_FWRITE_ELEM(key);

				sdbmail = (SDBMail *)GetListNodeData(mailnode);
				ASSERT_FWRITE_ELEM(sdbmail->sentToBox);

				mail = sdbmail->mail;
				ASSERT(mail);

				// For the Mail struct, we want to dump the whole thing in
				// one piece, because there aren't any substructures, and the
				// userIdentification field is nontrivial (but flat!).  Trick
				// is to deal with the string hanging off the end; to make
				// reads easier we output its length first.
				mlen = strlen(mail->message);
				ASSERT_FWRITE_ELEM(mlen);

				// Want sizeof(mail) + mlen, since sizeof is one too large
				// because of the dummy byte at the end, and mlen is one too
				// small because strlen() doesn't count the null byte.
				len = fwrite(mail, 1, sizeof(Mail) + mlen, fp);
				ASSERT(len == sizeof(Mail) + mlen);

				mailnode = GetNextListNode(mailnode);
			}
		}

		lnode = GetNextListNode(lnode);
	}

	fflush(fp);
	return(kNoError);
}

#undef ASSERT_FWRITE_ELEM

// just like ASSERT_FWRITE_ELEM, but read instead
#define ASSERT_FREAD_ELEM(item) { \
		long len = fread(&(item), 1, sizeof(item), fp); \
		ASSERT(len == sizeof(item)); \
	}

Err SDB_Restore(FILE *fp)
{
SDBCode		code;
ListNode	*lnode;
long		len;
long		count;

	// read a header
	len = fread(&code, 1, sizeof(SDBCode), fp);
	ASSERT(len == sizeof(SDBCode));
	if(len != sizeof(SDBCode))
		return(kUnexpectedEOF);

	ASSERT(code == kSDBCode_Server);
	if(code != kSDBCode_Server)
		return(kUnexpectedCode);

	// (don't) read the system.
	//if(SDBSystem_Restore(fp) != kNoError)
	//	return(kUnexpectedEOF);

	// SDBUsers_New() should already have been called
	ASSERT(gSDB);
	ASSERT(gSDB->users);
	
	// load the SDBUsers stuff
	ASSERT_FREAD_ELEM(gSDB->users->timeStamp);
	ASSERT_FREAD_ELEM(gSDB->users->uniqueBoxSerialNumber);
	ASSERT_FREAD_ELEM(gSDB->users->numUsers);

	// read the #of SDBBox structs
	ASSERT_FREAD_ELEM(count);
	Logmsg("Reading %ld box entries\n", count);

	// read the SDBBox junk, appending each new struct to gSDB->users->list
	while (count--) {
		SDBBox *box;
		BoxAccount *boxacc;
		int i, j;

		box = (SDBBox *)malloc(sizeof(SDBBox));
		ASSERT_MESG(box, "out of mems");

		ASSERT_FREAD_ELEM(box->boxSerialNumber);
		ASSERT_FREAD_ELEM(box->boxPhoneNumber);

		// read BoxAccount struct
		boxacc = &(box->boxAccount);
		boxacc->magicID = 0;		// no reason to save/restore this
		ASSERT_FREAD_ELEM(boxacc->box);
		ASSERT_FREAD_ELEM(boxacc->password);
		ASSERT_FREAD_ELEM(boxacc->homeTown);
		ASSERT_FREAD_ELEM(boxacc->gamePhone);
		ASSERT_FREAD_ELEM(boxacc->popPhone);
		ASSERT_FREAD_ELEM(boxacc->altPopPhone);
		ASSERT_FREAD_ELEM(boxacc->gamePlatform);
		ASSERT_FREAD_ELEM(boxacc->authCode);
		ASSERT_FREAD_ELEM(boxacc->userCredits);
		ASSERT_FREAD_ELEM(boxacc->freeCredits);
		for (i = 0; i < 7; i++)
			ASSERT_FREAD_ELEM(boxacc->restrictInfo[i]);
		ASSERT_FREAD_ELEM(boxacc->osLength);
		ASSERT_FREAD_ELEM(boxacc->boxFlags);
		ASSERT_FREAD_ELEM(boxacc->osVersion);
		ASSERT_FREAD_ELEM(boxacc->ngpVersion);
		ASSERT_FREAD_ELEM(boxacc->connections[0]);		// should be list
		ASSERT_FREAD_ELEM(boxacc->netErrorTotals);
		ASSERT_FREAD_ELEM(boxacc->debug0);
		ASSERT_FREAD_ELEM(boxacc->debug1);
		ASSERT_FREAD_ELEM(boxacc->debug2);
		ASSERT_FREAD_ELEM(boxacc->debug3);

		// read all four SDBUser structs
		for (i = 0; i < 4; i++) {
			SDBUser *user;
			PlayerAccount *placc;
			long mailcount;
			ListNode *mailnode;
			char user_exists;
			long slen;

			ASSERT_FREAD_ELEM(user_exists);
			if (!user_exists) {
				box->users[i] = NULL;
				continue;
			}

			user = (SDBUser *)malloc(sizeof(SDBUser));
			ASSERT_MESG(user, "out of mems");
			box->users[i] = user;

			ASSERT_FREAD_ELEM(user->userID);
			ASSERT_FREAD_ELEM(user->mailSerialNumber);
			ASSERT_FREAD_ELEM(user->lastBroadcastMailSent);

			// read the PlayerAccount
			placc = &(user->playerAccount);
			ASSERT_FREAD_ELEM(placc->magicID);
			ASSERT_FREAD_ELEM(placc->userName);
			ASSERT_FREAD_ELEM(placc->iconID);
			ASSERT_FREAD_ELEM(placc->colorTableID);

			ASSERT_FREAD_ELEM(slen);
			if (slen) {
				placc->openTaunt = (char *)malloc(slen+1);
				len = fread(placc->openTaunt, 1, slen, fp);
				ASSERT(len == slen);
			} else {
				placc->openTaunt = (char *)malloc(1);
				*(placc->openTaunt) = '\0';
			}
/* -----
			ASSERT_FREAD_ELEM(slen);
			if (slen) {
				placc->closeTaunt = (char *)malloc(slen+1);
				len = fread(placc->closeTaunt, 1, slen, fp);
				ASSERT(len == slen);
			} else {
				placc->closeTaunt = (char *)malloc(1);
				*(placc->closeTaunt) = '\0';
			}
----- */
			ASSERT_FREAD_ELEM(slen);
			if (slen) {
				placc->personInfo = (char *)malloc(slen+1);
				len = fread(placc->personInfo, 1, slen, fp);
				ASSERT(len == slen);
			} else {
				placc->personInfo = (char *)malloc(1);
				*(placc->personInfo) = '\0';
			}

			ASSERT_FREAD_ELEM(placc->password);
			ASSERT_FREAD_ELEM(placc->birthday);
			ASSERT_FREAD_ELEM(placc->playerFlags);
			ASSERT_FREAD_ELEM(placc->customIconSize);
			if (placc->customIconSize) {
				placc->customIcon = (unsigned char *) malloc(placc->customIconSize);
				ASSERT(placc->customIcon);
				len = fread(placc->customIcon, 1, placc->customIconSize, fp);
				ASSERT(len == placc->customIconSize);
			} else {
				placc->customIcon = NULL;
			}
			// msgState, msgChannels
			ASSERT_FREAD_ELEM(placc->autoMatchConnects);
			ASSERT_FREAD_ELEM(placc->challengeConnects);
			ASSERT_FREAD_ELEM(placc->msgCheckConnects);
			ASSERT_FREAD_ELEM(placc->failedServerConnects);
			ASSERT_FREAD_ELEM(placc->failedClientConnects);
			ASSERT_FREAD_ELEM(placc->numRankingInfo);
			ASSERT_FREAD_ELEM(placc->rankingInfo[0]);	// ouch!
			ASSERT_FREAD_ELEM(placc->rankingInfo[1]);	// ick!
			for (j = 0; j < kMaxAddressBookEntries; j++)
				ASSERT_FREAD_ELEM(placc->addressBook[j]);
			ASSERT_FREAD_ELEM(placc->debug0);
			ASSERT_FREAD_ELEM(placc->debug1);
			ASSERT_FREAD_ELEM(placc->debug2);
			ASSERT_FREAD_ELEM(placc->debug3);
			for (j = 0; j < kMaxInBoxEntries; j++)
				ASSERT_FREAD_ELEM(placc->mailInBox[j]);

			// load the user's mail into the incomingMail list
			user->incomingMail = NewSortedList();
			ASSERT_FREAD_ELEM(mailcount);
			if (mailcount > 1024) {
				Logmsg("ERROR: Bogus mailcount in restore, aborting\n");
				return (kUnexpectedEOF);
			}

			while (mailcount--) {
				SDBMail *sdbmail;
				Mail *mail;
				long mlen, key;

				// mail sort key (database "TimeStamp")
				ASSERT_FREAD_ELEM(key);

				sdbmail = (SDBMail *)malloc(sizeof(SDBMail));
				ASSERT_MESG(sdbmail, "out of mems");
				ASSERT_FREAD_ELEM(sdbmail->sentToBox);

				// get the mail message string length
				ASSERT_FREAD_ELEM(mlen);
				ASSERT(mlen < 1024);		// sanity check; screen out old save files
				if (mlen > 1024) {
					Logmsg("ERROR: Bogus mail in restore, aborting\n");
					return (kUnexpectedEOF);
				}

				// Size of mail message works out to be the size of the
				// struct plus the message string's length.
				sdbmail->mail = (Mail *)malloc(sizeof(Mail) + mlen);
				ASSERT_MESG(sdbmail->mail, "out of mems");

				len = fread(sdbmail->mail, 1, sizeof(Mail) + mlen, fp);
				ASSERT(len == sizeof(Mail) + mlen);

				mailnode = NewSortedListNode((Ptr)sdbmail, key);
				AddListNodeToSortedList(user->incomingMail, mailnode);
			}
		}

		lnode = NewSortedListNode((Ptr)box, box->boxSerialNumber.box);
		AddListNodeToSortedList(gSDB->users->list, lnode);
	}

	return(kNoError);
}

#undef ASSERT_FREAD_ELEM

