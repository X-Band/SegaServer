#define PARANOID_MODE		// remove me soon
/*
	File:		DataBase_UserAccount.c

	Contains:	xxx put contents here xxx

	Written by:	David Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<39>	 9/18/94	ATM		Added numRankingInfo field.
		<38>	 9/17/94	ATM		Made CompareStrings run faster.
		<37>	 9/16/94	ATM		boxFlags->boxModified, playerFlags->playerModified,
									acceptChallenges->playerFlags.  Added boxFlags.
		<36>	  9/7/94	ATM		phoneNumber result buf no longer passed to FindAccount,
									CreateAccount, and CreateBoxAccount.
		<35>	  9/2/94	DJ		setting the addrbook.serverUniqueID entries to
									kUncorrelatedEntry when creating a playerAccount
		<34>	 8/30/94	ATM		Made UpdateUserPhoneNumber use Account instead of
									userIdentification.
		<33>	 8/30/94	ATM		Now update BoxAccount's password.
		<32>	 8/28/94	ATM		Player account's password wasn't getting updated.
		<31>	 8/26/94	ATM		Overhaul of FindAccount and CreateAccount.
		<30>	 8/25/94	ATM		Init the boxModified field.
		<29>	 8/25/94	ATM		Allow you to uniquify onto your own name (e.g. "kodiak" -->
									"Kodiak", not "Kodiak1").
		<28>	 8/22/94	ATM		Add in netErrorTotals stuff.
		<27>	 8/21/94	ATM		Bunch o' changes to GetAccount and UpdateAccount.
		<26>	 8/19/94	ATM		Update PlayerAccount when ranking info changes.  [still need to
									handle connect records in BoxAccount.]
		<25>	 8/18/94	ATM		Handle update flag for credits.
		<24>	 8/17/94	ATM		Changed CreateAccount and UpdateUserHandle.
		<23>	 8/15/94	ATM		Changed SDBBox to hold full BoxSerialNumber.
		<22>	 8/12/94	DJ		made it work with personifuckations
		<21>	 8/11/94	DJ		0 0 is now the win/loss record for addr entries
		<20>	 8/10/94	ATM		Fixed bug in UpdateUserHandle.
		<19>	 8/10/94	ATM		Added homeTown.
		<18>	  8/6/94	ATM		Did a roto-rooter job on the uniquify stuff.  Now checks the
									pixel width of the handle as well.
		<17>	  8/5/94	ATM		Added "const"s to make the Mac -super_anal compiler happy.
		<16>	  8/5/94	ATM		PlayerAccount --> Account.
		<15>	  8/5/94	ATM		Added PlayerAccount stuff.
		<14>	 7/25/94	DJ		added Database_UpdateUserHandle
		<13>	 7/19/94	DJ		added mail serial number in SDBUser
		<12>	 7/19/94	DJ		dbid removed from playerinfo (is now in useridentification)
		<11>	 7/19/94	DJ		set dateLastPlayed to kNeverDate
		<10>	 7/18/94	DJ		changed Database_FindPlayerInfoByHandle to
									Database_FindPlayerInfo
		 <9>	  7/8/94	DJ		Database_FindPlayerInfoByHandle makes funny info fields now
		 <8>	  7/7/94	DJ		added updatephonenumber
		 <7>	  7/6/94	DJ		address book validation
		 <6>	  7/3/94	DJ		added Database_FindUserByHandle
		 <5>	 6/30/94	DJ		added lastBroadcastMailSent to user account
		 <4>	 6/29/94	BET		(Really DJ) Clean up after KON.
		 <3>	 6/11/94	DJ		cleaned up account creation and finding (used to be
									CheckAccount())
		 <2>	 5/31/94	DJ		added all the real user account handling
		 <1>	 5/27/94	DJ		first checked in

	To Do:
*/

#include "Server.h"
#include "ServerDataBase.h"
#include "ServerDataBase_priv.h"
#include "Errors.h"
#include "Utils.h"
#include "StringDB.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// == BRAIN DAMAGE ==
// receive login gets the boxid and phone number and then userIdentification.
// only userIdentification is needed (cuz boxid is redundant and there is
// no need to send phone number across, and that may even pose a security risk).
//
// For username (the handle) KON should send server a message when it changes....
// otherwise I would have to compare it to what the DB has to see if it changed.
//
// 5/30/94  dj
//

Err DataBase_FindUserIdentification(const userIdentification *userID, userIdentification *updatedUserID)
{
SDBUser *user;

	user = Database_FindUser(userID);
	if(user)
	{
		*updatedUserID = user->userID;
		return(kNoError);
	}
	return(kNoSuchUserAccount);
}


//
// This is used by mail to find a user account.
// If the box serial number is -1 then we should search by handle.  sigh.
// ATM: should we search by phone number instead of handle...?
// 
SDBUser *Database_FindUser(const userIdentification *userID )
{
SDBUsers	*users;
ListNode	*node;
SDBBox		*box;
SDBUser		*user;

	if(userID->box.box == -1)
		return(Database_FindUserByHandle(userID->userName));

	ASSERT_MESG((userID->userID > -1) && (userID->userID < 4),
				"userID is not between 0 and 3");
	if(userID->userID < 0 || userID->userID > 3)
		return(NULL);

	users = SDB_GetUsers(gSDB);
	ASSERT(users);

	node = SearchList(users->list, userID->box.box);
	if(node)
	{
		box = (SDBBox *)GetListNodeData(node);
		if(!box)
			return(NULL);

		user = box->users[userID->userID];
		if(!user)
			return(NULL);
		
		return(user);	// found the account.
	}

	return(NULL);		// no such account.
}

//
// Look up a user by handle.
//
SDBUser *Database_FindUserByHandle(const char *userName )
{
SDBUsers	*users;
ListNode	*node;
SDBBox		*box;
SDBUser		*user;
short		u;

	users = SDB_GetUsers(gSDB);
	ASSERT(users);

	for(node = GetFirstListNode(users->list);
		node;
		node = GetNextListNode(node))
	{
		box = (SDBBox *)GetListNodeData(node);
		ASSERT_MESG(box, "Impl error.  ListNode data should not be NULL... should be a SDBBox*");
		if(!box)
			return(NULL);			// impl error

		for(u = 0; u < 4; u++)
		{
			user = box->users[u];
			if(user)
				if(!Database_CompareStrings(userName, user->userID.userName))
					return(user);	// found the account.
		}
	}
	
	return(NULL);		// no such account.
}


Boolean Database_CompareUserIdentifications(userIdentification *challengeUserID,
											userIdentification *userID)
{
	if(Database_CompareBoxSerialNumbers(&challengeUserID->box, &userID->box))
		if(challengeUserID->userID == userID->userID)
			return(true);
	return(false);
}


// In proper fucked-up C style, this returns true if they match, false if different.
//
Boolean Database_CompareBoxSerialNumbers(BoxSerialNumber *box1, BoxSerialNumber *box2)
{
	if(box1->box == box2->box && box1->region == box2->region)
		return(true);
	else
		return(false);
}

#ifdef PARANOID_MODE
short Original_CompareStrings( const char *cString1, const char *cString2 );
short Original_CompareStrings( const char *cString1, const char *cString2 )
{
char	byte1, byte2;

	do
	{
		byte1 = *cString1++;
		byte2 = *cString2++;
		
		//
		// strip out spaces
		//
		while( byte1 == ' ')
			byte1 = *cString1++;
		while( byte2 == ' ')
			byte2 = *cString2++;

		//
		// convert to lower case
		//
		if( (byte1 <= 'Z') && (byte1 >= 'A') )
			byte1 += ('a'-'A');
	
		if( (byte2 <= 'Z') && (byte2 >= 'A') )
			byte2 += ('a'-'A');
		
		//
		// compare
		//
		if( byte1 > byte2 )
			return 1;			//string 1 is after string 2 alphabetically
		if( byte1 < byte2 )
			return -1;			//string 1 is before string 2 alphabetically
	}
	while( byte1 );
	
	return 0;	//they're equal
}
#endif /*PARANOID_MODE*/

// Assume most chars will be lower case.
#define MY_TOLOWER(x) ( ((x) >= 'A' && (x) <= 'Z') ? (x)+0x20 : (x) )

//
// This should be faster than the original (without using -O, the code
// for this routine is twice as fast using gcc).
//
// Note that this returns, 0, positive, or negative, NOT 0,1,-1.
//
int Database_CompareStrings(register const char *str1, register const char *str2)
{
	register int c1, c2;
#ifdef PARANOID_MODE
	int retval;
	char *orig_str1 = str1;
	char *orig_str2 = str2;
#endif

	str1--, str2--;		// you didn't see that
	do {
		// Assume runs of spaces will be short.
		//
		while ((c1 = *++str1) == ' ')
			;
		while ((c2 = *++str2) == ' ')
			;

		if (MY_TOLOWER(c1) != MY_TOLOWER(c2))
			break;
	} while (c1);

#ifndef PARANOID_MODE
	return (MY_TOLOWER(c1) - MY_TOLOWER(c2));
#else
	retval = MY_TOLOWER(c1) - MY_TOLOWER(c2);
	if (retval > 0) retval = 1;
	if (retval < 0) retval = -1;
	if (retval != Original_CompareStrings(orig_str1, orig_str2)) {
		Logmsg("CompareStrings FAILED\n");
		abort();
	}
	return (c1 - c2);
#endif /*PARANOID_MODE*/
}

//
// look up the boxid.
// if none, create an account and create this user.
//
// find this user.
//
// BRAIN DAMAGE: this needs to check for region as well
//

SDBBox *Database_FindBoxAccount(SDBUsers *users, const BoxSerialNumber *boxSerialNumber)
{
SDBBox		*box;
ListNode	*node;

	box = NULL;

	node = SearchList(users->list, boxSerialNumber->box);
	if(node)
		box = (SDBBox *)GetListNodeData(node);

	return(box);
}

SDBBox *Database_CreateBoxAccount(SDBUsers *users, const userIdentification *userID)
{
SDBBox		*box;
ListNode	*node;

	// create a new account for this virgin box. yum!  ;-)
	box = (SDBBox *)malloc(sizeof(SDBBox));
	ASSERT(box);
	if(!box)
	{
		ERROR_MESG("out of mems");
		return(NULL);
	}
	
	box->boxSerialNumber = userID->box;		// move into BoxAccount??
	//box->boxPhoneNumber = *boxPhoneNumber;		// move this into BoxAccount
	box->users[0] = box->users[1] = box->users[2] = box->users[3] = NULL;

	DataBase_InitBoxAccount(box);

	node = NewSortedListNode((Ptr)box, box->boxSerialNumber.box);
	AddListNodeToSortedList(users->list, node);

	return(box);
}

	
// Create a new account for this virgin box. yum!  ;-)
//
SDBUser *Database_CreateBoxUserAccount(SDBBox *box, const userIdentification *userID)
{
SDBUser		*user;

	ASSERT(box);
	
	// create the user
	user = (SDBUser *)malloc(sizeof(SDBUser));
	ASSERT(user);
	if(!user)
	{
		ERROR_MESG("out of mems");
		return(NULL);
	}

	user->userID = *userID;

	user->incomingMail = NewSortedList();
	user->mailSerialNumber = 0;
	user->lastBroadcastMailSent = 0;

	DataBase_InitPlayerAccount(user);		// set defaults

	return(user);
}


// Updates the userID in place.
//
Err Database_CreateAccount(userIdentification *userID, long *result, Account **account)
{
SDBUsers	*users;
SDBBox		*box;
SDBUser		*user;

	*result = 0;
	*account = NULL;

	Logmsg("CREATING: (%ld,%ld)[%ld]\n", userID->box.box, userID->box.region,
		userID->userID);

	ASSERT_MESG((userID->userID > -1) && (userID->userID < 4),
				"userID is not between 0 and 3");
	if(userID->userID < 0 || userID->userID > 3)
		return(kInvalidUserID);


	users = SDB_GetUsers(gSDB);
	ASSERT(users);

	// copy the userID, and uniquify the userName
	if (DataBase_UniquifyHandle(userID))
		*result |= kUserNameUniquified;

	box = Database_FindBoxAccount(users, &userID->box);
	if(!box)
		box = Database_CreateBoxAccount(users, userID);
	
	user = box->users[userID->userID];

	if(!user)
	{
		user = Database_CreateBoxUserAccount(box, userID);
		box->users[userID->userID] = user;
	} else {
		ERROR_MESG("You cannot recreate an existing account!");
		return(kAccountAlreadyExists);
	}
	
	// store a log of when logged in perhaps?

	// also, if the box did send phone number all the time,
	// then we could ensure that it was the same and if not
	// we could reprogram ourselves...  we should also reprogram
	// the box with its new local POP if that's the case.
	//
	// Right now we ignore the phone number  5/31/94

	*account = DataBase_GetAccount(userID);
	if (*account == NULL) {
		Logmsg("CreateAccount: found box, but not Account?!?\n");
		return(kNoSuchUserAccount);
	}

	return(kNoError);
}

// Updates the userID in place.
//
Err Database_UpdateUserHandle(userIdentification *userID, long *result)
{
SDBUsers	*users;
SDBBox		*box;
SDBUser		*user;

	*result = 0;

	ASSERT_MESG((userID->userID > -1) && (userID->userID < 4),
				"userID is not between 0 and 3");
	if(userID->userID < 0 || userID->userID > 3)
		return(kInvalidUserID);

	// copy the userID, and uniquify the userName
	if (DataBase_UniquifyHandle(userID))
		*result |= kUserNameUniquified;

	users = SDB_GetUsers(gSDB);
	ASSERT(users);

	box = Database_FindBoxAccount(users, &userID->box);
	if(!box)
		return(kNoSuchBoxAccount);

	user = box->users[userID->userID];
	if(!user)
		return(kNoSuchUserAccount);

	// check that handles match and replace if not.
	//
	if(Database_CompareStrings(user->userID.userName, userID->userName))
		ByteCopy((Ptr)user->userID.userName, (Ptr)userID->userName, kUserNameSize);

	return(kNoError);
}


// NOT RPCed.
//
Err Database_UpdateUserPhoneNumber(Account *account, const phoneNumber *boxPhoneNumber)
{
	// check that phone numbers match and replace if not.
	//
	if (strncmp(account->boxAccount.gamePhone.phoneNumber, boxPhoneNumber->phoneNumber,
		kPhoneNumberSize) != 0)
	{
		account->boxAccount.gamePhone = *boxPhoneNumber;
		account->boxModified |= kBA_gamePhone;
	}

	return(kNoError);
}

Err Database_FindAccount(const userIdentification *userID, Account **account)
{
SDBUsers	*users;
SDBBox		*box;
SDBUser		*user;

	*account = NULL;

	ASSERT_MESG((userID->userID > -1) && (userID->userID < 4),
				"userID is not between 0 and 3");
	if(userID->userID < 0 || userID->userID > 3)
		return(kInvalidUserID);


	users = SDB_GetUsers(gSDB);
	ASSERT(users);

	box = Database_FindBoxAccount(users, &userID->box);
	if(!box)
		return(kNoSuchBoxAccount);

	user = box->users[userID->userID];

	if(!user)
		return(kNoSuchUserAccount);

	// store a log of when logged in perhaps?

	//*boxPhoneNumber = box->boxPhoneNumber;	// return the box's phone number.
	*account = DataBase_GetAccount(userID);
	if (*account == NULL) {
		Logmsg("FindAccount: found box, but not Account?!?\n");
		return(kNoSuchUserAccount);
	}
	return(kNoError);
}


//
// userID is the id of the caller.  this is used because you need to cross-index the last time you played
// against that user for the dateLastPlayed field.  Shit.
//
// This routine assumes that playerID is valid (ie. came from DataBase_FindUserIdentification)
//
PlayerInfo *Database_FindPlayerInfo(const userIdentification *playerID, const userIdentification *userID)
{
PlayerInfo	*playerInfo;
Account		*account;
PlayerAccount		*playerAccount;
long		length;

	ASSERT(playerID->box.box != -1);

	account = DataBase_GetAccount(playerID);
	playerAccount = &account->playerAccount;
	ASSERT_MESG(playerAccount, "We should never get this message, cuz we just called Database_FindUser and it returned OK.");
	if(!playerAccount)
		return(NULL);

	length = strlen(playerAccount->personInfo)+1;
	playerInfo = (PlayerInfo *)malloc(sizeof(PlayerInfo) + length);
	playerInfo->userId = *playerID;

	// for this info you want to scan through the address book of userID looking for playerID.  
	playerInfo->wins = 0;
	playerInfo->losses = 0;
	playerInfo->dateLastPlayed = 1; /* kNeverDate; */	// BRAIN DAMAGE.  should check if we matched them up and set date to then.

	strcpy(playerInfo->info, playerAccount->personInfo);

	DataBase_FreeAccount(account);		// ++ATM

	return(playerInfo);
}

void Database_ReleasePlayerInfo(PlayerInfo *playerInfo)
{
	ASSERT(playerInfo);
	free(playerInfo);
}


// ===========================================================================
//		BoxAccount routines
// ===========================================================================

//
// Set the values in a brand new SDBBox to defaults.
//
// Called from Database_CreateBoxAccount().
//
Err DataBase_InitBoxAccount(SDBBox *box)
{
	BoxAccount *newbox;

	ASSERT(box);
	newbox = &(box->boxAccount);

	// zap the whole thing
	memset(newbox, 0, sizeof(BoxAccount));

	newbox->box = box->boxSerialNumber;
	newbox->gamePhone = box->boxPhoneNumber;

	return (kNoError);
}


// ===========================================================================
//		PlayerAccount routines
// ===========================================================================

//
// Set the values in a brand new SDBUser to defaults.
//
// Called from Database_CreateBoxUserAccount().
//
Err DataBase_InitPlayerAccount(SDBUser *user)
{
	PlayerAccount *newpa;
	long			a;


	ASSERT(user);
	newpa = &(user->playerAccount);

	// zap the whole thing
	memset(newpa, 0, sizeof(PlayerAccount));

	newpa->numRankingInfo = 2;		// BRAIN DAMAGE, remove me soon

	strcpy(newpa->userName, user->userID.userName);

	newpa->openTaunt = (char *) malloc(1);	// if we just make the pointer NULL
	*(newpa->openTaunt) = '\0';				// then xdr_string() will crash
/* -----
	newpa->closeTaunt = (char *) malloc(1);
	*(newpa->closeTaunt) = '\0';
----- */
	newpa->personInfo = (char *) malloc(1);
	*(newpa->personInfo) = '\0';

	for(a = 0; a < kMaxAddressBookEntries; a++)
		newpa->addressBook[a].serverUniqueID = kUncorrelatedEntry;
	
	return (kNoError);
}


// ===========================================================================
//		Account routines
// =========================================================