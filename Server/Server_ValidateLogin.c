/*
	File:		Server_ValidateLogin.c

	Contains:	xxx put contents here xxx

	Written by:	David Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<84>	 9/16/94	DJ		xband free credits now == 32 not 35
		<83>	 9/16/94	DJ		tweak to longdistance checker (note: it checks every time you
									log in.... so we can change yer status at any time)
		<82>	 9/16/94	DJ		adding londistance checker
		<81>	 9/16/94	ATM		Need to clear "dial 9" flag on an ANI connect.
		<80>	 9/16/94	ATM		Added NETERR Logmsg so the errors can be tagged & bagged.  Made
									non-ANI calls from a simulator work better.
		<79>	 9/16/94	ATM		boxFlags->boxModified, playerFlags->playerModified,
									acceptChallenges->playerFlags.
		<78>	 9/14/94	ATM		Differentiate "!= HAPPY_REGION" from a new box.
		<77>	 9/13/94	ATM		Added a dialog for lookup failure.  Removed cruft.
		<76>	 9/12/94	DJ		changed LataLookup to POPLookup (looks up Compuserve POPs)
		<75>	 9/12/94	ATM		Sanity-check on ANI result should be 10 chars, not 11.
		<74>	  9/7/94	ATM		Roto-rooter job on ValidateLogin().
		<73>	  9/6/94	DJ		removed some DEBUG dialogs by #ifdef GRATUITOUSing them
		<72>	  9/6/94	ATM		Slap some more parens around that acceptChallenges compare.
		<71>	  9/6/94	ATM		Inverted the logic on acceptChallenges flag.
		<70>	  9/4/94	ATM		Re-send the box's hometown whenever the number changes.
		<69>	  9/4/94	ATM		Leave the ranking test stuff in.
		<68>	 8/30/94	DJ		returns endcommunication if bad 800 number
		<67>	 8/30/94	ATM		Recognize 1234567 as a bogus phone number.
		<66>	 8/30/94	ATM		Converted accountChangedMask references to Account flags.
		<65>	 8/29/94	ATM		Add new problem token constants.
		<64>	 8/28/94	ATM		Changed an error message to make it useful.
		<63>	 8/27/94	ATM		De-cheesed.
		<62>	 8/26/94	DJ		stub for Server_RestoreBox
		<61>	 8/26/94	ATM		Overhaul of FindAccount and CreateAccount.
		<60>	 8/24/94	ATM		Hack a logmsg.
		<59>	 8/23/94	DJ		added state->disallowGameConnect
		<58>	 8/22/94	DJ		groomed dialog text
		<57>	 8/22/94	DJ		nicer printfs
		<56>	 8/22/94	DJ		added some DEBUG dialogs about credit/debit/token stuff for box
									debugging/testing
		<55>	 8/21/94	ATM		Set accountChangedMask for acceptChallenges.
		<54>	 8/21/94	ATM		Tweaked a logmsg.
		<53>	 8/21/94	ATM		altPopPhone, acceptChallenges.
		<52>	 8/21/94	DJ		setlocalaccessphonenumber takes 2 phone nums (2nd is fallback
									number)
		<51>	 8/20/94	ATM		Foo.
		<50>	 8/20/94	ATM		Don't stuff junk into boxAccount.popPhone when they have an
									unknown boxID.
		<49>	 8/20/94	BET		Get rid of a few of the gratuitous dialogs.  The can be turned
									on again with the compile flag GRATUITOUS.
		<48>	 8/19/94	DJ		state->gameResult is now a ptr to a GameResult
		<47>	 8/19/94	DJ		printing out problem token details
		<46>	 8/19/94	ATM		De-RPC UpdateRanking.
		<45>	 8/19/94	DJ		smartcard stuff from box is broken, so i reenable the box on the
									next connect
		<44>	 8/19/94	ATM		Added an UpdateRankings call to the end of ValidateLogin.
		<43>	 8/18/94	DJ		added comments
		<42>	 8/18/94	DJ		added 35 credits per account.
		<41>	 8/18/94	DJ		clearing out the box disable password.
		<40>	 8/18/94	ATM		Added sanity check on ANI result.
		<39>	 8/17/94	ATM		Updated calls to CreateAccount and UpdateUserHandle.
		<38>	 8/17/94	ATM		Re-check newAccessPhoneNumber whenever 1-800 called.  Correctly
									handle calls to X.25 with boxPhone = blank.
		<37>	 8/16/94	DJ		new smartcard live debit testing
		<36>	 8/16/94	ATM		Added ANI support.
		<35>	 8/15/94	DJ		handling new user on existing box
		<34>	 8/15/94	ATM		Bumped up HAPPY_REGION to 1.
		<33>	 8/15/94	ATM		Added HAPPY_REGION hack.
		<32>	 8/13/94	DJ		Sever_ValidateLogin returns int instead of err
		<31>	 8/12/94	ATM		Added a Statusmsg for newly created players.
		<30>	 8/11/94	ATM		Switched to new LATA call.
		<29>	 8/11/94	ATM		Changed Pretty code to do dashes only.  Converted to Logmsg.
		<27>	 8/11/94	DJ		turned off account weirdness and added a hack to see if handle
									has changed and uniquify it.
		<26>	 8/10/94	BET		Added scriptID brokering vi phoneNumber structure.
		<25>	 8/10/94	ATM		Mac-ified.
		<24>	 8/10/94	ATM		Added phoneNumber prettification and updated the area code
									munging calls.
		<23>	  8/9/94	ATM		Now cope with area codes.
		<22>	  8/8/94	DJ		SendDialog takes boolean whether to stick or disappear in 3 sec.
		<21>	  8/6/94	ATM		Removed abort() to make Mac happy.
		<20>	  8/6/94	ATM		Updated uniquify stuff.
		<19>	  8/6/94	DJ		bug
		<18>	  8/6/94	DJ		x25 redial
		<17>	  8/5/94	DJ		playeraccount stuff
		<16>	  8/4/94	ATM		Improved unique handle code.
		<15>	  8/3/94	BET		Add faked-up X.25 brokering until the real thing is there.
		<14>	  8/3/94	ATM		Improved uniquification of usernames.
		<13>	  8/2/94	DJ		sendcredittoken doesn't send cardserialnum anymore
		<12>	 7/31/94	DJ		clearing out magic token if not sending one
		<11>	 7/29/94	DJ		added Server_FakeCreditStuff to test smartcard stuff
		<10>	 7/25/94	DJ		disabling box if user is shannon
		 <9>	 7/20/94	DJ		added Server_Comm stuff
		 <8>	 7/19/94	DJ		comments
		 <7>	 7/12/94	DJ		updated to new boxserialnum format
		 <6>	  7/7/94	DJ		user account is updated with phone number in case it changes
									(this is a hack until ANI)
		 <5>	  7/1/94	DJ		making server handle errors in Comm layer
		 <4>	 6/29/94	BET		(Really DJ) Clean up after KON.
		 <3>	 6/11/94	DJ		added kServerValidFlag_ExtractedPhoneNumber
		 <2>	 5/31/94	DJ		sends new boxSerialNumber if first login
		 <1>	 5/27/94	DJ		first checked in

	To Do:
*/

#include "ServerCore.h"
#include "Server.h"
#include "ServerState.h"
#include "Messages.h"
#include "UsrConfg.h"
#include "ServerDataBase.h"
#include "Server_Comm.h"

#include "SegaIn.h"
#include "DBConstants.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//
// 8/22/94
// BRAIN DAMAGE
// To do: these dialogs and the logic to drive them need to be implemented sometime.
//
// You do not have XBAND access privileges at this time.  Check the Account Info in the Options
// area to see your account restrictions.  Press any button to continue.
//
// Sorry - no local phone numbers are available to connect to XBAND.  A long distance call
// will be made to connect you.  Press any button to continue.
//
// We don't have enough XBAND players registered in your area yet.  We will sometimes
// need to make a long distance call to find you an opponent.
//


/*
	lookup box serial number in the database
	
	if ok
		return(ok)
	else
		why isn't this valid?  how did he call in without it being valid?
		is his O/S fucked?  reset his O/S?
		or is it a hacker?
		

	login should have
		boxSerialNumber
		userNumber (0-3)
	
	billing information
		smart card... we do nothing but log the card for counterfeit checking later
		credit card... how many credits are left?  is he running low?

	we should also check lockouts
		- N games per day
		- between certain hours of the day
		- etc. (?)
*/



int Server_ValidateCreditStuff(ServerState *state);
int Server_DisableBox(ServerState *state, char *reason);
Boolean Server_BoxCanDialLongDistance(phoneNumber *gamePhone);



//
// Validate the incoming user.
//
int Server_ValidateLogin(ServerState *state)
{
userIdentification 	*userID, oldID;
Err					err, findErr;
Hometown			town;
phoneNumber 		newBoxPhoneNumber, newAccessPhoneNumber, altAccessPhoneNumber;
int					newPhoneFlag, playerCreated, accountCreated;
char				*cp, buf[256];
long				result;

	Logmsg("Server_ValidateLogin\n");
	ASSERT(state->validFlags & kServerValidFlag_BoxPhoneNumber);
	ASSERT(state->validFlags & kServerValidFlag_Login);

	userID = &state->loginData.userID;
	Logmsg("Connection from box # (%ld,%ld)[%ld]\n", userID->box.box,
		userID->box.region, (long)userID->userID);

	newPhoneFlag = false;
	accountCreated = false;
	playerCreated = false;

	// See if we have ANI via 1-800.  Sometimes we get bogus data, so
	// do a sanity-check on it to be sure.
	//
	if ((cp = getenv("PORTAL_ANI")) != NULL) {
		if (strncmp(cp, "00", 2) != 0 || strlen(cp) < 11) {
			Logmsg("WHOA: bad ANI '%s', ignoring\n", cp);
			cp = NULL;
		} else {
			// ANI is valid, pull the data out
			//
			Logmsg("ANI says caller is '%s'\n", cp);
			newPhoneFlag = true;
			newBoxPhoneNumber.scriptID = 0;
			newBoxPhoneNumber.pad[0] = 0;
			strcpy(newBoxPhoneNumber.phoneNumber, cp+2);
			if (strlen(newBoxPhoneNumber.phoneNumber) != 10) {
				Logmsg("GLITCH: value from ANI (%s) is hosed\n",
					newBoxPhoneNumber.phoneNumber);
			}
		}
	}

	// At this point we could see if the box's idea of what its phone number
	// is got damaged (which we used to do).  However, we no longer care what
	// the box thinks.

	// See if the box lost its mind.
	//
	err = Server_RestoreBox(state, cp ? true : false);	// true if 800 call
	if (err != kServerFuncOK)
		return(err);


	// Print the DNIS number, just for fun.
	//
	if ((cp = getenv("PORTAL_DNIS")) != NULL)
		Logmsg("DNIS says line is '%s'\n", cp);


	// Quick hack to allow us to invalidate all existing users without
	// having to reset all of their boxes manually.
	//
	if (userID->box.region != -1 && userID->box.region != HAPPY_REGION) {
		userID->box.box = -1;
		userID->box.region = HAPPY_REGION;
		Logmsg("Region wasn't happy, resetting ID to -1\n");
	}


	//
	// Try to find the player's account.
	//
	findErr = Database_FindAccount(userID, &state->account);

	if (findErr == kNoError) {
		// We found the box and the player.
		//
		state->validFlags |= kServerValidFlag_Account;

		// See if he updated his handle.
		//
		if (strcmp(userID->userName, state->account->playerAccount.userName) != 0)
		{
			oldID = *userID;
			if (Database_UpdateUserHandle(userID, &result) != kNoError) {
				Logmsg("ERROR: Found account, but unable to UpdateUserHandle\n");
				return(kServerFuncAbort);
			}
			// ATM: fix this!!  Make UUH take an account ptr, de-RPC it

			// If UpdateUserHandle uniqified the userName field, notify the box.
			//
			if (result & kUserNameUniquified) {
				Server_SendNewCurrentUserName(state, userID->userName);
				sprintf(buf, "Your handle '%s' is not unique.  It has been changed to '%s'.",
					oldID.userName, userID->userName);
				Server_SendDialog(state, buf, false);
				
				Logmsg("UpdateUserHandle changed handle from '%s' to '%s'.\n",
					oldID.userName, userID->userName);
			}
		}

	} else if (findErr == kNoSuchUserAccount) {
		//
		// New player on a box that has already logged in.
		//
		oldID = *userID;
		if (Database_CreateAccount(userID, &result, &state->account) !=kNoError)
		{
			Logmsg("ERROR: Server_ValidateLogin couldn't find existing box!\n");
			return(kServerFuncAbort);
		}
		state->validFlags |= kServerValidFlag_Account;

		//
		// If CreateAccount uniqified the userName field, notify the box.
		//
		if (result & kUserNameUniquified) {
			Server_SendNewCurrentUserName(state, userID->userName);
			sprintf(buf, "Your handle '%s' is not unique.  It has been changed to '%s'.",
				oldID.userName, userID->userName);
			Server_SendDialog(state, buf, false);

			Logmsg("CreateAccount changed handle from '%s' to '%s'.\n",
				oldID.userName, userID->userName);
		}

		playerCreated = true;

	} else {
		//
		// Brand new box.
		//
		if(!(state->validFlags & kServerValidFlag_BoxPhoneNumber))
		{
			// ATM: what does this do?  There's an ASSERT at the top of
			// this routine that makes sure this is set.  It's a bit late
			// to be worrying about it now...
			ERROR_MESG("Implementation Error: there is no user account, yet this isn't a phone number change or ANI");
			ERROR_MESG("Database may be corrupt?");
			return(kServerFuncAbort);
		}

		if (userID->box.box != -1) {
			Logmsg("WARNING: boxSerialNum is (%ld,%ld), but account not found.\n",
				userID->box.box, userID->box.region);
		}

		// Generate a serial number, and set it in the box.
		//
		// Failing now could be a bad thing, because the box would know
		// who it was, but we wouldn't.  However, so long as we don't
		// reuse the generated ID, we should be able to figure it out.
		//
		Database_GenerateUniqueBoxSerialNumber(&userID->box);
		if (Server_SendNewBoxSerialNumber(state, &userID->box) != kNoError) {
			return(kServerFuncAbort);
		}

		// Create the account in the database.
		//
		oldID = *userID;
		if (Database_CreateAccount(userID, &result, &state->account) !=kNoError)
		{
			Logmsg("ERROR: CreateAccount failed\n");
			return(kServerFuncAbort);
		}
		state->validFlags |= kServerValidFlag_Account;

		// If CreateAccount uniqified the userName field, notify the box.
		//
		if (result & kUserNameUniquified) {
			Server_SendNewCurrentUserName(state, userID->userName);
			sprintf(buf, "Your handle '%s' is not unique.  It has been changed to '%s'.",
				oldID.userName, userID->userName);
			Server_SendDialog(state, buf, false);

			Logmsg("CreateAccount changed handle from '%s' to '%s'.\n",
				oldID.userName, userID->userName);
		}

#ifdef kFreeXBandCredits
		// Give them 35 free credits.
		//
		state->account->boxAccount.freeCredits = kFreeXBandCredits;
		state->account->boxModified |= kBA_credits;
		sprintf(buf, "You have been given %ld free XBand credits for Alpha testing.",
			(long)kFreeXBandCredits);
		Server_SendDialog(state, buf, true);
		Logmsg("%s\n", buf);
#endif

		accountCreated = true;
	}

set_new_phone:
	//
	// If this was an ANI call, set their box phone number, POP phone number,
	// and home town.
	//
	// We can't just set the box phone number if it's changed, since we now
	// strip the area code off and can't do a clean comparison.  These aren't
	// exactly huge bandwidth wasters anyway.
	//
	if (newPhoneFlag) {
		// Put parens or dashes in to make it look nice.  We MUST do this
		// or Server_SendSetBoxPhoneNumber won't strip the area code off.
		//
		Server_MakePhoneNumberPretty(&newBoxPhoneNumber);

		// Do a POP lookup to get the access phone numbers and city/state.
		//
//		if (DataBase_LataLookup(&newBoxPhoneNumber,
//			&newAccessPhoneNumber, &altAccessPhoneNumber, town) != kNoError)
		if (DataBase_POPLookup(&newBoxPhoneNumber,
			&newAccessPhoneNumber, &altAccessPhoneNumber, town) != kNoError)
		{
			// This shouldn't happen.
			Logmsg("POP lookup failed for '%s'\n",
				newBoxPhoneNumber.phoneNumber);
			sprintf(buf,
			  "We can't seem to find a local number for you to call.  Bummer.");
			Server_SendDialog(state, buf, true);
			return (kServerFuncAbort);
		}

		// Set the boxPhoneNumber.
		//
		Server_SendSetBoxPhoneNumber(state, &newBoxPhoneNumber);
		strcpy(state->boxPhoneNumber.phoneNumber, newBoxPhoneNumber.phoneNumber);
		(void)Database_UpdateUserPhoneNumber(state->account, &newBoxPhoneNumber);
		//state->account->boxAccount.gamePhone = newBoxPhoneNumber;
		//state->account->boxModified |= kBA_gamePhone;

		// Set the POP phone numbers.  We can have this trigger an
		// immediate redial, by passing TRUE as the last argument and
		// having the function return kServerFuncEnd.
		// (Do we need to set "state->disallowGameConnect = true" as well?)
		//
		Server_SendSetLocalAccessPhoneNumber(state, &newAccessPhoneNumber,
			&altAccessPhoneNumber, false);	// don't force a redial.
		state->account->boxAccount.popPhone = newAccessPhoneNumber;
		state->account->boxAccount.altPopPhone = altAccessPhoneNumber;
		state->account->boxModified |= kBA_popPhone;

		// Set the home town.
		//
		Server_SendNewBoxHometown(state, town);
		strcpy(state->account->boxAccount.homeTown, town);
		state->account->boxModified |= kBA_homeTown;
		strncpy(userID->userTown, town, kUserTownSize);		// not necessary

		// Clear the "9," flag... they got through to 1-800.
		//
		if (state->account->boxAccount.debug1 & 1) {
			state->account->boxAccount.debug1 &= ~1;
			state->account->boxModified |= kBA_debug;
			Logmsg("Cleared the dial-9 flag\n");
		}

		Logmsg("New phone info: box='%s', pop='%s'[%d], altPop='%s'[%d], town='%s'\n",
			newBoxPhoneNumber.phoneNumber,
			newAccessPhoneNumber.phoneNumber, newAccessPhoneNumber.scriptID,
			altAccessPhoneNumber.phoneNumber, altAccessPhoneNumber.scriptID,
			town);

	} else {
		// If this was a brand-new account, and they didn't come in on 1-800,
		// it's either a confused box or a hacker.  Do something?
		//

		// If we just created an account, but they didn't come in over ANI