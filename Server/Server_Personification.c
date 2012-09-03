/*
	File:		Server_Personification.c

	Contains:	Pretty pink flowers.  If you don't like pink then they are blue.  

	Stolen by:	Brian Topping

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<21>	 9/16/94	ATM		boxFlags->boxModified, playerFlags->playerModified,
									acceptChallenges->playerFlags.
		<20>	 8/30/94	ATM		Converted accountChangedMask references to Account flags.
		<19>	 8/28/94	ATM		Put quotes around some logmsg strings to see trailing spaces
									(esp password).
		<18>	 8/25/94	ATM		Had a weird problem.  Added an ASSERT.
		<17>	 8/21/94	DJ		bug
		<16>	 8/21/94	DJ		added kPersonificationRAMIcon
		<15>	 8/20/94	DJ		dont' receive RAM icon, don't send handle
		<14>	 8/20/94	DJ		added more logmsgs
		<13>	 8/20/94	ATM		Added some ASSERTs around the malloc/free stuff.
		<12>	 8/18/94	DJ		testing custom icons
		<11>	 8/17/94	ATM		Changed UpdateUserHandle call.
		<10>	 8/16/94	DJ		fixed double uniquification
		 <9>	 8/16/94	DJ		glogs
		 <8>	 8/15/94	DJ		new sendpersonification
		 <7>	 8/12/94	DJ		update to handle new ramicon
		 <6>	 8/12/94	DJ		wont change your name to itself
		 <5>	 8/12/94	DJ		fixing barf
		 <4>	 8/12/94	DJ		fixing barf
		 <3>	 8/11/94	DJ		new personifuckation
		 <2>	 8/10/94	DJ		process peer, receive peer.  fix wire routines.
		 <1>	  8/4/94	BET		first checked in

	To Do:
*/

#include "Server_Personification.h"
#include "Server.h"
#include "Server_Comm.h"
#include "ServerDataBase.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


//
// FILTERFIELDS changes the fields of the personification.
// TEST_CUSTOM_ICON will download the sheep icon (works if FILTERFIELDS  is on).
//
#define FILTERFIELDS
#define TEST_CUSTOM_ICON
//

int Server_ProcessPersonifications(ServerState *state)
{
PlayerAccount			*player;
PersonificationSetupRec	*pers;
userIdentification		*uid, oldID;
long					result;
unsigned char 			opCode;
unsigned char			flags;
Boolean					handleHasBeenUniquified = false;

	ASSERT(state->validFlags & kServerValidFlag_Personifications);
	ASSERT(state->validFlags & kServerValidFlag_Account);

	if (state->account == NULL) {
		Logmsg("Why is this happening?\n");
		abort();
	}

	uid = &state->loginData.userID;
	player = &state->account->playerAccount;
	pers = &state->userPersonification;


	if(state->personificationFlags)
	{

		if(state->personificationFlags & kPersonificationPassword )
		{
			strncpy(player->password, pers->pwData, kMaxPWLength);
			state->account->playerModified |= kPA_password;

			Logmsg("	Personification password changed.\n");
		}

		if( state->personificationFlags & kPersonificationTauntText )
		{
			ASSERT(player->openTaunt);
			free(player->openTaunt);
			player->openTaunt = (char *)malloc(strlen(pers->TauntTextData)+1);
			ASSERT(player->openTaunt);
			strcpy(player->openTaunt, pers->TauntTextData);
			state->account->playerModified |= kPA_openTaunt;

			Logmsg("	Personification taunt = %s\n",  player->openTaunt);
		}

		if( state->personificationFlags & kPersonificationAboutText )
		{
			ASSERT(player->personInfo);
			free(player->personInfo);
			player->personInfo = (char *)malloc(strlen(pers->InfoTextData)+1);
			ASSERT(player->personInfo);
			strcpy(player->personInfo, pers->InfoTextData);
			state->account->playerModified |= kPA_personInfo;


			Logmsg("	Personification info = %s\n",  player->personInfo);
		}

		if(state->personificationFlags & kPersonificationROMIconID )
		{
			player->iconID = (short)pers->userIDData->ROMIconID;
			state->account->playerModified |= kPA_iconID;
			
			Logmsg("	Personification ROMIconID = %ld\n", (long)player->iconID);
		}

		if(state->personificationFlags & kPersonificationROMClutID )
		{
			player->colorTableID = (short)pers->userIDData->colorTableID;
			state->account->playerModified |= kPA_colorTableID;
			
			Logmsg("	Personification colorTableID = %ld\n", (long)player->colorTableID);
		}

#ifdef FILTERFIELDS

		// Mess around with Ted's personification and icon.
		//
		if(!strncmp( state->loginData.userID.userName,  "ted", strlen("ted")))
		{
		register long a;
		unsigned char *bugger = "censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult \
		censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult \
		censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult \
		censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult censored by catapult ";
		
			for(a = 0; a < strlen(player->personInfo); a++)
				player->personInfo[a] = bugger[a];
			state->personificationFlagsEditedByServer |= kPersonificationAboutText;
			state->account->playerModified |= kPA_personInfo;

			for(a = 0; a < strlen(player->openTaunt); a++)
				player->openTaunt[a] = bugger[a];
			state->personificationFlagsEditedByServer |= kPersonificationTauntText;
			state->account->playerModified |= kPA_openTaunt;


#ifdef TEST_CUSTOM_ICON	// download a custom icon for you!

			{
			RAMIconBitMapPtr	ram;
			PreformedMessage	*pre;			

				pre = PreformedMessage_ReadFromFile("SheepIcon.datafork");
				if(pre)
				{
					Logmsg("Sending SheepIcon as custom icon\n.");
					ram = (RAMIconBitMapPtr)pre->message;
					ram->box = state->loginData.userID.box;			// <- Probably cleaner/more consistent to get from playerAccount.
					ram->userID = state->loginData.userID.userID;
	
					if(player->customIcon)
						free ( player->customIcon );
					player->customIcon = malloc(pre->length);
					ASSERT_MESG(player->customIcon, "Out of mems reading in custom icon.");
					if(!player->customIcon)
						goto OutOfMemory;

					player->customIconSize = pre->length;
					bcopy(pre->message, player->customIcon, player->customIconSize);
					
					state->personificationFlagsEditedByServer |= kPersonificationRAMIcon;
					state->account->playerModified |= kPA_customIcon;

OutOfMemory:
					PreformedMessage_Dispose(pre);
				}
			}
#endif

		}
#endif

	}


	flags = state->personificationFlags | state->personificationFlagsEditedByServer;
	if(flags)
		Server_SendPersonification(state, flags);


	return(kServerFuncOK);
}


int Server_ReceivePersonification(ServerState *state)
{
unsigned char 			opCode;
PersonificationSetupRec	*pers;
int						retVal;

	Logmsg("Server_ReceivePersonification\n");

	if(Server_TReadDataSync( state->session, 1, (Ptr)&opCode ) != noErr)
		return(kServerFuncAbort);

	if(opCode != msSendInvalidPers){
		// fucked
		return(kServerFuncAbort);
	}

	state->personificationFlags = 0;

	if(Server_TReadDataSync( state->session, sizeof(unsigned char), (Ptr)&state->personificationFlags ) != noErr)
		return(kServerFuncAbort);

	retVal = Server_GetPersonificationFromWire(state, &state->userPersonification, state->personificationFlags);

	if(retVal != kServerFuncOK)
		return(retVal);

	pers = &state->userPersonification;
	Logmsg("Personification received:\n");
	if(state->personificationFlags & kPersonificationPassword )
		Logmsg("    password: '%s'\n", pers->pwData);
	if( state->personificationFlags & kPersonificationTauntText )
		Logmsg("    taunt: '%s'\n", pers->TauntTextData);
	if( state->personificationFlags & kPersonificationAboutText )
		Logmsg("    info: '%s'\n", pers->InfoTextData);
	if(state->personificationFlags & kPersonificationROMIconID )
		Logmsg("    ROMIconID: %ld\n", (long)pers->userIDData->ROMIconID);
	if(state->personificationFlags  & kPersonificationROMClutID )
		Logmsg("	clutID: %ld\n", (long)pers->userIDData->colorTableID);

	state->validFlags |= kServerValidFlag_Personifications;

	Logmsg("Server_ReceivePersonification done\n");

	return(kServerFuncOK);
}

void Server_DeletePersonification( PersonificationSetupRec *OPers )
{
	ASSERT(OPers);

	if ( OPers->userIDData )
		free( (Ptr)OPers->userIDData );
	if ( OPers->TauntTextData )
		free( (Ptr)OPers->TauntTextData );
	if ( OPers->InfoTextData )
		free( (Ptr)OPers->InfoTextData );
	if ( OPers->RAICData )
		free( (Ptr)OPers->RAICData );
}


// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// Server <-> Box Personification Xfer

// The info exchanged looks like this:
//
//				         	  		           			  Taunt	  	  Info	    Custom Icon
//	opcode	[userID][ROMIconID][Town][Handle][Password][size:Text][size:Text][size:ISEG][IDSC]
//			\_______________________________________________________________/\_______________/
//					  	          		Mandatory				        	  	  Optional
//
// If the sender doesn't have a Custom Icon, he sends 0 for the ISEG size, then no more.
//
// The IDSC is just the framerate (a short) for an animated icon.  If the sender's custom
//  icon is static, 0 is sent.
//
// The receiver must always accept all 6 (4 with no Custom Icon) resources.
// He doesn't necessarily have to keep the Icon.
//
// string sizes are unsigned chars, icon size is a short.

int Server_GetPersonificationFromWire( ServerState *state, PersonificationSetupRec *OPers, unsigned char flags )
{
int					result;
short				theSize;


	result = kServerFuncOK;

	OPers->userIDData = 0;
	OPers->TauntTextData = 0;
	OPers->InfoTextData = 0;
	OPers->RAICData = 0;
	OPers->RAICDataSize = 0;

	if(flags & kPersonificationROMIconID )
		OPers->userIDData = (userIdentification *)malloc( sizeof(userIdentification) );

// get [Password]

	if(flags & kPersonificationPassword )
	{
		Server_TReadDataSync( state->session, sizeof(Password), (Ptr)&(OPers->pwData) );
		if(Server_TCheckError() != noErr)
			goto abort;
	}

// get [size:Taunt]

	if( flags & kPersonificationTauntText )
	{
		Server_TReadDataSync( state->session, sizeof(short), (Ptr)&theSize );
		if(Server_TCheckError() != noErr)
			goto abort;

		OPers->TauntTextData = malloc( theSize );
		Server_TReadDataSync( state->session, theSize, OPers->TauntTextData );
		if(Server_TCheckError() != noErr)
			goto abort;
	}

// get [size:Info]

	if( flags & kPersonificationAboutText )
	{
		Server_TReadDataSync( state->session, sizeof(short), (Ptr)&theSize );
		if(Server_TCheckError() != noErr)
			goto abort;

		OPers->InfoTextData = malloc( theSize );
		Server_TReadDataSync( state->session, theSize, OPers->InfoTextData );
		if(Server_TCheckError() != noErr)
			goto abort;
	}

	if(flags & kPersonificationROMIconID )
	{
	// get [ROMIconID]

		Server_TReadDataSync( state->session, sizeof(DBID), (Ptr)&(OPers->userIDData->ROMIconID) );
		if(Server_TCheckError() != noErr)
			goto abort;
	
		OPers->RAICDataSize = 0;	// box never sends custom icon back up to the server, silly!
		(OPers->IDSCData).frameDelay = 0;
		OPers->RAICData = 0L;
	}
	
	if(flags & kPersonificationROMClutID )
	{
	// get [kPersonificationROMClutID]

		Server_TReadDataSync( state->session, sizeof(DBID), (Ptr)&(OPers->userIDData->colorTableID) );
		if(Server_TCheckError() != noErr)
			goto abort;
	}
	
	// box doesn't send its custom icon (kPersonificationRAMIcon), cuz they always originate on the server.
	
	
	return result;

abort:
	result = kServerFuncAbort;

	Server_DeletePersonification(OPers);

	return result;
}


void Server_SendPersonification(ServerState *state, unsigned char flags)
{
PlayerAccount	*player;
short			theSize;
DBID			iconID;
unsigned char	opCode;


	Logmsg("Server_SendPersonification\n");


	ASSERT(state->validFlags & kServerValidFlag_Personifications);
	player = &state->account->playerAccount;


	opCode = msReceiveValidPers;
	Server_TWriteDataSync( state->session, sizeof(unsigned char), (Ptr)&opCode );


	Server_TWriteDataSync( state->session, sizeof(unsigned char), (Ptr)&flags );

	if(flags & kPersonificationPassword )
	{
		Server_TWriteDataSync( state->session, sizeof(Password), (Ptr)&player->password );
	}

	if( flags & kPersonificationTauntText )
	{
		theSize = strlen( player->openTaunt ) + 1;
		Server_TWriteDataSync( state->session, sizeof(short), (Ptr)&theSize );
		Server_TWriteDataSync( state->session, theSize, player->openTaunt );
	}

	if( flags & kPersonificationAboutText )
	{
		theSize = strlen( player->personInfo ) + 1;
		Server_TWriteDataSync( state->session, sizeof(short), (Ptr)&theSize );
		Server_TWriteDataSync( state->session, theSize, player->personInfo );
	}

	if(flags & kPersonificationROMIconID )
	{
		iconID = (DBID)player->iconID;
		Server_TWriteDataSync( state->session, sizeof(DBID), (Ptr)&iconID );
	}
	
	if(flags & kPersonificationRAMIcon )
	{
		theSize = (short)player->customIconSize;
		Server_TWriteDataSync( state->session, sizeof(short), (Ptr)&theSize );

		if(theSize)
		{
		IconSetup			ramIconIDSC;

			Server_TWriteDataSync( state->session, (long)theSize, (Ptr)player->customIcon );
			ramIconIDSC.frameDelay = 0;
			Server_TWriteDataSync( state->session, sizeof(IconSetup), (Ptr)&ramIconIDSC );
		}
	}

	if(flags & kPersonificationROMClutID )
	{
		iconID = (DBID)player->colorTableID;
		Server_TWriteDataSync( state->session, sizeof(DBID), (Ptr)&iconID );
	}

}


// The info exchanged looks like this:
//
//				         	  		           			  Taunt	  	  Info	    Custom Icon
//	opcode	[userID][ROMIconID][Town][Handle][Password][size:Text][size:Text][size:ISEG][IDSC]
//			\_______________________________________________________________/\_______________/
//					  	          		Mandatory				        	  	  Optional
//
// string sizes are unsigned chars, icon size is a short.

void Server_PutPersonificationOnWire( ServerState *state, PersonificationSetupRec *PSR )
{
userIdentification	*theUserIdentification;
//IconRefStruct		*theIREF;
IconSetup			*theIDSC;
Ptr					shit;
short				theSize;
long				opponentVerificationTag;


#if 0

// send [userID]

	Server_TWriteDataSync( state->session, sizeof(unsigned char), (Ptr)&PSR->userIDData->userID );

// send [ROMIconID]

//	theIREF = (IconRefStruct *)PSR;
	
	Server_TWriteDataSync( state->session, sizeof(DBID), (Ptr)&PSR->ROMIconID );

// send [Town]

	Server_TWriteDataSync( state->session, sizeof(Hometown), PSR->userIDData->userTown );

// send [Handle]

	Server_TWriteDataSync( state->session, sizeof(UserName), PSR->userIDData->userName );

// send [Password]

	Server_TWriteDataSync( state->session, sizeof(Password), PSR->pwData );

// send [size:Taunt]

	shit = PSR->TauntTextData;
	theSize = strlen( shit ) + 1;
	Server_TWriteDataSync( state->session, sizeof(short), (Ptr)&theSize );
	Server_TWriteDataSync( state->session, theSize, shit );

// send [size:Info]

	shit = PSR->InfoTextData;
	theSize = strlen( shit ) + 1;
	Server_TWriteDataSync( state->session, sizeof(short), (Ptr)&theSize );
	Server_TWriteDataSync( state->session, theSize, shit );


/*
*	8/4/94 7:03:39 PM (BET): I am guessing we don't need this...
* // send [size:ISEG][IDSC] (or 0 if there's no custom icon)
*	if ( DBGetItem( kIconBitMapType, theIREF->RAMID ) )
*	{
*		theSize = DBGetItemSize( kIconBitMapType, theIREF->RAMID );
*		Server_TWriteDataSync( state->session, sizeof(short), (Ptr)&theSize );				// send size
*
*		shit = DBGetItem( kIconBitMapType, theIREF->RAMID );
*		Server_TWriteDataSync( state->session, theSize, shit );								// send ISEG
*		
*		theIDSC = DBGetItem( kIconDescType, theIREF->RAMID );			// is it animated?
*		if ( theIDSC )
*			Server_TWriteDataSync( state->session, sizeof(IconSetup), (Ptr)theIDSC );		// send IDSC
*			else
*				Server_TWriteDataSync( state->session, sizeof(IconSetup), (Ptr)&theIDSC );	// send 0
*	}
*	else
*/
	{
		theSize = 0;										
		Server_TWriteDataSync( state->session, sizeof(short), (Ptr)&theSize );
	}
	
#endif

}


