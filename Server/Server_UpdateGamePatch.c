/*
	File:		Server_UpdateGamePatch.c

	Contains:	xxx put contents here xxx

	Written by:	David Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<16>	 9/16/94	ATM		Improved signal to noise ratio of log messages.
		<15>	 9/15/94	DJ		oops. forgot to include Challnge.h
		<14>	 9/15/94	DJ		don't send game patch on mail-only connect
		<13>	  9/9/94	DJ		added alarm to drop the connection during download of game patch
									(commented out for production, of course)
		<12>	 8/23/94	DJ		added state->disallowGameConnect
		<11>	 8/16/94	ATM		Allow boxes with huge patch numbers.
		<10>	 8/13/94	DJ		ForceEnd
		 <9>	 8/13/94	ATM		Removed reference to gLogFile.
		 <8>	 8/11/94	ATM		Try that again.
		 <7>	 8/11/94	ATM		Converted to Logmsg.
		 <6>	 8/10/94	DJ		added more printfs so you can tell which version of patch we
									downloaded, if any
		 <5>	 7/17/94	BET		Make printf's go to gLogFile instead of stdout, which is the
									modem.
		 <4>	  7/1/94	DJ		making server handle errors in Comm layer
		 <3>	 6/11/94	DJ		major making real
		 <2>	 5/31/94	DJ		renamed DisposePreformedMsg.. to ReleasePreformedMsg...
		 <1>	 5/27/94	DJ		first checked in

	To Do:
*/

#include "ServerCore.h"
#include "Server.h"
#include "ServerState.h"
#include "ServerDataBase.h"

#include "Challnge.h"

#include <stdio.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <signal.h>

void alarm_handler(void);


int Server_UpdateGamePatch(ServerState *state)
{
long 				latestGameVersion;
PreformedMessage	*mesg;
OSErr				err;

	Logmsg("Server_UpdateGamePatch\n");

	if(state->challengeData.userID.box.box == kDownloadOnlyMailSerialNumber)
	{
		Logmsg("UpdateGamePatch: no patch downloaded for mail only connection\n");
		//
		// Don't set this flag, or Server_StartGamePlay won't print the
		// last Game Results.
		//
		//		state->disallowGameConnect = true;
		return(kServerFuncOK);
	}

//
// This is to test the network when we drop a connection during download of a game patch.
//
//		signal(SIGALRM, alarm_handler);
//		alarm(10);
//		Logmsg("Alarm set for %d seconds\n", 15);


	if(state->gameIDData.version == -1){
		Logmsg("UpdateGamePatch: box has no patch for this game\n");
		//
		// Box doesn't have *any* version of this game's patch.
		//
		if ((latestGameVersion =
			DataBase_GetLatestGameVersion(state->gameIDData.gameID)) == -1)
		{
			Logmsg("UpdateGamePatch: unsupported game ID 0x%.8l\n",
				state->gameIDData.gameID);
			if(Server_SendUnsupportedGame(state) != kServerFuncOK)
				return(kServerFuncAbort);
			
			state->disallowGameConnect = true;
			return(kServerFuncOK);
		}

		mesg = DataBase_GetGamePatch(state->gameIDData.gameID);
		if(!mesg){
			Logmsg("UpdateGamePatch: game ID 0x%.8lx has no patch\n",
				state->gameIDData.gameID);
			return(kServerFuncOK);
		}

		Logmsg("UpdateGamePatch: sending version %ld for game 0x%.8lx\n",
			latestGameVersion, state->gameIDData.gameID);
		err = Server_SendPreformedMessage(state, mesg);
		DataBase_ReleasePreformedMessage(mesg);
		Logmsg("Server_UpdateGamePatch done\n");
		return(err);
	}

	latestGameVersion = DataBase_GetLatestGameVersion(state->gameIDData.gameID);
	if(latestGameVersion < 0){
		Logmsg("UpdateGamePatch: unsupported game ID 0x%.8lx\n",
			state->gameIDData.gameID);
		if(Server_SendUnsupportedGame(state) != kServerFuncOK)
				return(kServerFuncAbort);
		
		Logmsg("WARNING: UpdateGamePatch: box has version %ld for UNKNOWN game 0x%.8lx\n",
			state->gameIDData.version, state->gameIDData.gameID);

		state->disallowGameConnect = true;
		return(kServerFuncOK);
	}

	if(latestGameVersion > state->gameIDData.version){
		//
		// Box needs latest version of this game's patch.
		//
		mesg = DataBase_GetGamePatch(state->gameIDData.gameID);
		if(!mesg){
			Logmsg("Server_UpdateGamePatch: game ID = %ld has a NULL patch\n", state->gameIDData.gameID);
			return(kServerFuncOK);
		}
		Logmsg("UpdateGamePatch: box has version %ld for game 0x%.8lx, sending version %ld\n",
			state->gameIDData.version, state->gameIDData.gameID,
			latestGameVersion);
		err = Server_SendPreformedMessage(state, mesg);
		DataBase_ReleasePreformedMessage(mesg);
		Logmsg("Server_UpdateGamePatch done\n");
		return(err);
	}

	if(state->gameIDData.version > latestGameVersion){
		Logmsg("WARNING: UpdateGamePatch: box has version %ld for game 0x%.8lx, current version is OLDER %ld\n",
			state->gameIDData.version, state->gameIDData.gameID,
			latestGameVersion);

		// ==BRAIN DAMAGE==   should we try to notify the box that it is fucked?
		// or should we go into a send stream that blows the boxes O/S away
		// and resets it automagically?
		//
		
		//return(kServerFuncAbort);
		Logmsg("UpdateGamePatch: box had a patch version > mine (okay for now)\n");
		return(kServerFuncOK);
	}

	Logmsg("UpdateGamePatch: box has current version %ld for 0x%.8lx\n",
		latestGameVersion, state->gameIDData.gameID);

	return(kServerFuncOK);	// box has current version of game patch.
}

