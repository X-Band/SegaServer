/*
	File:		DataBase_NGPVersion.c

	Contains:	xxx put contents here xxx

	Written by:	David Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <2>	 7/16/94	dwh		#ifndef unix to the Debugger in the apparently unused routine
									below.  (This whole file looks unused, why is it even here?)
		 <1>	 5/27/94	DJ		first checked in

	To Do:
*/

#include "Server.h"	// this is only for PreformedMessage type
#include "ServerDataBase.h"

#include <stdio.h>


//
// Update the NGPVersion information if not up to date.
//
long DataBase_GetLatestNGPVersion(void)
{
	return(1);
}

PreformedMessage *DataBase_GetLatestNGP(void)
{
#ifndef	unix
	Debugger();
#endif
	return(NULL);
}

