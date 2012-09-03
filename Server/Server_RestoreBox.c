/*
	File:		Server_RestoreBox.c

	Contains:	xxx put contents here xxx

	Written by:	Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <4>	 9/16/94	ATM		boxFlags->boxModified, playerFlags->playerModified,
									acceptChallenges->playerFlags.
		 <3>	  9/9/94	DJ		added comments and pseudocode about what this routine should do
		 <2>	 8/27/94	ATM		De-cheesed.
		 <1>	 8/21/94	DJ		first checked in

	To Do:
*/


#include "Server.h"
#include "Server_Comm.h"
#include "ServerDataBase.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


int Server_RestoreBox(ServerState *state, Boolean callingFrom800)
{
unsigned char 			opCode;
PersonificationSetupRec	*pers;
int						retVal;

	Logmsg("Server_RestoreBox\n");

	Logmsg("Server_RestoreBox done\n");
	return(kServerFuncOK);
}



//////////////////////////////////////////////////////////////////////////////////////

#ifdef DAVES_NOTES



//
//
// The first thing we need to do is establish if the box has crashed.
// If so, we see if the boxid & region survived.
//	if so, lookup the PlayerAccount
	findErr = Database_FindAccount(userID, &state->account);
	if (findErr == kNoError) {
		// We found the box and the player.


// we should really figure out which heaps are trashed.  do we need to blow anything
// away?  I hope the box blows away any heaps it thinks might be trashed... we don't
// want fucked-up patches and the like hanging around.

// if we couldn't retrieve the boxid and region, then we check if dialed-in on 1-800
// if so, we use the ANI number and lookup the box account that way
	Database_FindAccountByBoxPhoneNumberAndPlayerNumber(newBoxPhoneNumber, state->loginData.userID.userID)

// if not, we tell the box to redial us on 1-800 and we disconnect.

	// so... if we couldn't find an account corresponding to the 1-800 number... is it a new box?
	// if not, then the guy's box crashed when he moved phone numbers (or his battery is dead, and
	// he sold the unit in a garage sale).  WHAT do we do here?  All we can do is create a new
	// account.  perhaps we should mark the box as fucked and store the date of fuckage.
	//.  then, whenever he redials in on 1-800, we check to see if the new number matches
	// an old account that never called in after the box was fucked.  if so, we *may* have
	// found the straggler.  We can't really restore at that point, cuz the box is in all
	// new state.  About all we can do is mark the old account as dead, with some special
	// indicator to say that it now references a new account.
	
	// The problem is that when boxes lose their battery, every time they move, we will
	// create a new account for that box.  Even worse, is if there was a box at that new
	// number and we erroneously restore the box to someone else's box.  I can imagine all
	// kinds of nightmares for people who live in apartments or frat houses.
	


/////////////////

		if (Server_SendNewBoxSerialNumber(state, &userID->box) != kNoError) {
			Server_SendNewCurrentUserName(state, userID->userName);
		Server_SendSetBoxPhoneNumber(state, &newBoxPhoneNumber);
		Server_SendSetLocalAccessPhoneNumber(state, &newAccessPhoneNumber,
			&altAccessPhoneNumber, false);	// don't force a redial.
		Server_SendNewBoxHometown(state, town);




		//
		// Turn off the box password.
		//
		state->account->boxAccount.debug0 = 0;
		state->account->boxModified |= kBA_debug;

		ids[0] = kConnectPasswordHiConst;
		ids[1] = kConnectPasswordLoConst;
		consts[0] = consts[1] = 0;
		Server_SendDBConstants(state, 2, ids, consts);



//
// Send system patches in ValidateSystem
//

//
// Update game patch in UpdateGamePatch
//


/////////////////////// mail //////////////
//
// Send back the mail that was on the box from last time
//
	for(inBox = 0; inBox < state->loginData.numMailsInBox; inBox++)
		state->account->playerAccount.mailInBox[inBox] = state->loginData.mailSerialNumbers[inBox];

//
// Shit! here is a bug.  we must store the number of mails on the box

// Also be sure then to set state->loginData.numMailsInBox to the number
// of mails on the box.

		opCode = msReceiveMail;
		Server_TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);

		Server_TWriteDataSync(state->session, sizeof(short), (Ptr)&numTotalMails);

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





//
////////
// Now restore the address book.  be careful here, because they may have had a full
// addr book, then added some new entries after the crash.... so what you will end
// up with, if not careful, is an overflowed addr book!  yikes!
//
//
	for(i = 0; i < kMaxAddressBookEntries; i++)
	{
		if( state->account->playerAccount.addressBook[i].serverUniqueID != kUncorrelatedEntry)

// use this info
					state->account->playerAccount.addressBook[serverUniqueID].serverUniqueID = serverUniqueID;
					state->account->playerAccount.addressBook[serverUniqueID].box = playerInfo->userId.box;
					state->account->playerAccount.addressBook[serverUniqueID].playerNumber = playerInfo->userId.userID;
					state->account->playerAccount.addressBook[serverUniqueID].lastPlayed = playerInfo->dateLastPlayed;
					state->account->playerAccount.addressBook[serverUniqueID].wins = playerInfo->wins;
					state->account->playerAccount.addressBook[serverUniqueID].losses = playerInfo->losses;

// and this call (?)
				playerInfo = Database_FindPlayerInfo(&updatedUserIdent, &state->loginData.userID);
				if(!playerInfo)
					// ooops.  that player no longer there.
				else
					playerInfo->serverUniqueID = serverUniqueID;
					retVal = Server_SendPlayerInfo(state, playerInfo, state->addrValidationData.items[i].ownerUserID);


// *** somehow, someway, must check that any new addr book validation queries
// don't overflow things.  shit.  how do we do that?




///////////////////////
// Accountinfo will be resent by SendAccountInfo (calls SendRestrictions and SendCreditInfo)
//
//
//  sendranking will send rankings for all of the users games, so ok there.
//
// Server_SendDateAndTime gets called ok too.
//
//
// Now we must send the rest of their personification
//	- we've sent handle already
//

	flags = kPersonificationPassword | kPersonificationTauntText | kPersonificationAboutText | kPersonificationROMIconID | kPersonificationRAMIcon | kPersonificationROMClutID;
	Server_SendPersonification(state, flags);

// Hmmmm.  could we have some interesting side-effects with Server_ProcessPersonifications ???
//		I think we're ok...  since it doesn't deal with handles anymore, we are fine.
//





#endif DAVES_NOTES

