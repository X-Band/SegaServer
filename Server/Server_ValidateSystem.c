/*
	File:		Server_ValidateSystem.c

	Contains:	xxx put contents here xxx

	Written by:	David Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <5>	  9/2/94	DJ		more printf cheese
		 <4>	 8/30/94	DJ		Server_ValidateSystem now just looks on the filesystem for the
									latest patch file instead of looking in the database.  So you
									can put a SystemPatch.1 where SunSega runs and it will get send
									to patchversion 0 boxes without having to bounce the DB.
		 <3>	  7/1/94	DJ		making server handle errors from the comm layer
		 <2>	 5/31/94	DJ		using ReleasePreformedMessage
		 <1>	 5/27/94	DJ		first checked in

	To Do:
*/

#include "ServerCore.h"
#include "Server.h"
#include "ServerState.h"
#include "Messages.h"
#include "UsrConfg.h"
#include "ServerDataBase.h"

#include <stdio.h>

/*
	get the latest system from the database
	compare to:
		state->systemVersionData.length (?)
		state->systemVersionData.version
	
	if his version doesn't match, update his system
	we will incrementally update him.  eg.
		sys0
		sys1 - patch 3
		sys2 - sys1 and patch 5
		
	if he is system 0, then we send him system 1 patches and then
	system 2 patches.  So we iterate through the system
	

	Actually, it's more subtle...
		there can be keyframes and deltas.
		find the latest keyframe.. send it and then the deltas.
	
	Also, each system could have multiple patches
	
	
*/

#ifdef OLD_AND_USES_THE_DATABASE

int Server_ValidateSystem(ServerState *state)
{
long i, latestVersion, latestKeyframeVersion, startVersion;
	
	latestVersion = DataBase_GetLatestSystemVersion();
	latestKeyframeVersion = DataBase_GetLatestSystemKeyframeVersion();
	if(state->systemVersionData.version >= latestKeyframeVersion)
		startVersion = state->systemVersionData.version;
	else
		startVersion = latestKeyframeVersion;

	for(i = startVersion; i <= latestVersion; i++)
		if(Server_SendSystemPatches(state, i) != kServerFuncOK)
			return(kServerFuncAbort);

	return(kServerFuncOK);
}

/*
	look up the patches in the database	
	there are multiple patches per system version....
*/
int Server_SendSystemPatches(ServerState *state, long version)
{
PreformedMessage	*patch;
long 				numPatches, i;
OSErr				err;

	numPatches = DataBase_GetSystemNumPatches(version);
	for(i = 0; i < numPatches; i++)
	{
		patch = DataBase_GetSystemPatch(version, i);
		err = Server_SendPreformedMessage(state, patch);
		DataBase_ReleasePreformedMessage(patch);
		if(err != kServerFuncOK)
			return(err);
	}

	return(kServerFuncOK);
}

#else


//
// Now system patches are read in by the SunSega processes, so we don't have to
// bounce the database when we add a new patch.
//
#define kSystemPatchFileName	"SystemPatch"
#define kSystemPatchFileNameLength	500
#define kBiggestSystemVersion	100L


int Server_ValidateSystem(ServerState *state)
{
long				i;
char				fname[kSystemPatchFileNameLength];
PreformedMessage	*patch = NULL;
int					err;

	Logmsg("Server_ValidateSystem\n");

	for(i = kBiggestSystemVersion; i; i--)
	{
		sprintf(fname, "%s.%ld", kSystemPatchFileName, i);
		patch = PreformedMessage_ReadFromFile(fname);
		if(patch)
			break;
	}
	
	err = kServerFuncOK;

	if(i > state->systemVersionData.version)
	{
		err = Server_SendPreformedMessage(state, patch);
		Logmsg(" Box was patch version %ld, I downloaded patch version %ld (length = %ld).\n",
					state->systemVersionData.version, i, patch->length);
	}
	else
		Logmsg(" No system patch was downloaded because box had patch version %ld and the latest system patch version is %ld.\n",
					state->systemVersionData.version, i);

	if(patch)
		PreformedMessage_Dispose(patch);

	Logmsg("Server_ValidateSystem done\n");
	return(err);
}

int Server_SendSystemPatches(ServerState *state, long version)
{
	return(kServerFuncOK);
}

#endif


