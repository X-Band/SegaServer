/*
	File:		DataBase_GamePatch.c

	Contains:	xxx put contents here xxx

	Written by:	David Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<12>	 9/12/94	ATM		Changed DataBase_*GameInfo to Common_.
		<11>	  9/3/94	ATM		Now use GameInfo struct.
		<10>	 8/25/94	ATM		Fixed reload stuff to not wipe queues.
		 <9>	 8/24/94	ATM		Added reload stuff.
		 <8>	 8/22/94	ATM		Fixed sizeof(PatchDescriptor) problem.
		 <7>	 8/19/94	DJ		make compile on Mac
		 <6>	 8/19/94	ATM		Moved LoadGamePatches in (was Server_SetupGamePatches).
		 <5>	 8/16/94	ATM		Updated FindGame calls.
		 <4>	 7/19/94	DJ		gameName is now just a char*
		 <3>	 7/13/94	DJ		added DataBase_GetGameName
		 <2>	 6/11/94	DJ		looking up in game patch db for latest version
		 <1>	 5/27/94	DJ		first checked in

	To Do:
*/

#include "Server.h"	// this is only for PreformedMessage type
#include "ServerDataBase.h"
#include "ServerDataBase_priv.h"
#include "Errors.h"

#include "PatchDB.h"
#include "Messages.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//
// Update Game Patch
//
long DataBase_GetLatestGameVersion(long gameID)
{
SDBGameNode	*game;

	game = Database_FindGame(gameID, true);
	if(game)
		return(game->version);

	return(-1);	// no such game
}

PreformedMessage *DataBase_GetGamePatch(long gameID)
{
SDBGameNode	*game;

	game = Database_FindGame(gameID, true);
	if(game)
	{
		return(game->patch);
	}
	
	return(NULL);
}
char *DataBase_GetGameName(long gameID, char *gameName)
{
SDBGameNode	*game;

	game = Database_FindGame(gameID, true);
	if(game)
	{
		strncpy(gameName, game->gameName, kGameNameSize);
		return(gameName);
	}
	
	return(NULL);
}


//
// Formerly in Server_LoadDataBase.c
//

//
// Load game patches
//
void DataBase_LoadGamePatches(void)
{
	PreformedMessage	msg;
	PatchDescriptor		descr;
	GameInfo		*gameInfo;
	SDBGameNode		*game;
	unsigned char	*start, *end;
	FILE		*fp = NULL;
	messOut		opCode;
	char		buf[32], *cp;
	long 		hdrLength, fullLength;
	long		numGames;
	int			i;

	if ((fp = fopen(kSDB_GamePatchFile, "rb")) == NULL) {
		Logmsg("Unable to open game patch file '%s'\n", kSDB_GamePatchFile);
		goto nofile;
	}

	if ((gameInfo = Common_GetGameInfo(&numGames)) == NULL) {
		// this is Very Bad
		Logmsg("Unable to get GameInfo\n");
		return;
	}

	// don't use sizeof(PatchDescriptor); has dangler on end
	start = (unsigned char *) &descr;
	end = (unsigned char *) &descr.data;

	while (1) {
		// This ought to be done in PreformedMessage.c, somehow.
		//
		if (fread(&opCode, sizeof(opCode), 1, fp) != 1)
			break;
		if (feof(fp))
			break;
		if (opCode != msGamePatch) {
			Logmsg("Wrong opCode in game patch file!  Wanted %ld, got %ld\n",
				(long)msGamePatch, (long)opCode);
			goto nofile;
		}
		if (fread(&descr, end-start, 1, fp) != 1) {
			Logmsg("Unexpected EOF in game patch read\n");
			goto nofile;
		}

		// find the game's name
		cp = NULL;
		for (i = 0; i < numGames; i++) {
			if (descr.gameID == gameInfo[i].gameID) {
				cp = gameInfo[i].gameName;
				break;
			}
		}
		if (i < numGames) {
			gameInfo[i].patchVersion = descr.patchVersion;
			if (!gameInfo[i].patchVersion)
				Logmsg("WHOA: found a game patch with version 0, ID = 0x%.8lx\n",
					descr.gameID);
		} else {
				sprintf(buf, "Unknown 0x%.8lx", descr.gameID);
				cp = buf;
		}

		hdrLength = sizeof(messOut) + end-start;
		fullLength = descr.codeSize + hdrLength;
		msg.message = (Ptr)malloc((size_t)fullLength);
		ASSERT(msg.message);

		// copy the header bits over
		memcpy(msg.message, &opCode, sizeof(messOut));
		memcpy(msg.message + sizeof(messOut), &descr, end-start);

		// read what's left; set length to whole thing
		msg.length = fullLength;
		if (fread(msg.message + hdrLength, descr.codeSize, 1, fp) != 1) {
			Logmsg("Unexpected EOF in game Patch read\n");
			goto nofile;
		}

		if ((game = Database_FindGame(descr.gameID, true)) == NULL) {
			// haven't seen this before
			DataBase_NewGame(descr.gameID, descr.patchVersion, cp, &msg);
			Logmsg("Added patch for 0x%.8lx (%s) version %ld (length = %ld)\n",
				descr.gameID, cp, (long)descr.patchVersion,
				(long)descr.codeSize);
		} else {
			// been there, done that, just replace the patch if it needs it
			if (descr.patchVersion > game->version) {
				DataBase_AddGamePatch(descr.gameID, descr.patchVersion, &msg);
				Logmsg("Replaced patch for 0x%.8lx (%s) version %ld (length = %ld)\n",
					descr.gameID, cp, (long)descr.patchVersion,
					(long)descr.codeSize);
			} else {
				Logmsg("Patch for 0x%.8lx (%s) version %ld is current\n",
					descr.gameID, cp, (long)descr.patchVersion);
			}
		}

		free(msg.message);
	}


nofile:		// get here if no file to read, or file was hosed

	if (fp != NULL)
		fclose(fp);

	// Now we need to call DataBase_NewGame on all the games that *don't*
	// have patches and therefore weren't initialized above.
	//
	for (i = 0; i < numGames; i++) {
		if (!gameInfo[i].patchVersion) {
			if ((game = Database_FindGame(gameInfo[i].gameID, true)) == NULL) {
				DataBase_NewGame(gameInfo[i].gameID, 1, gameInfo[i].gameName, NULL);
				Logmsg("No patch for 0x%.8lx (%s), assigned version %ld\n",
					gameInfo[i].gameID, gameInfo[i].gameName, 1);
			} else {
				Logmsg("No patch for 0x%.8lx (%s) version %ld\n",
					gameInfo[i].gameID, gameInfo[i].gameName, game->version);
			}
		}
	}

	Common_FreeGameInfo(gameInfo);
}


void DataBase_ReloadGamePatches(void)
{
	Logmsg("Reloading game patches\n");
	DataBase_LoadGamePatches();
}

