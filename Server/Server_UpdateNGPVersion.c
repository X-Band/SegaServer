/*
	File:		Server_UpdateNGPVersion.c

	Contains:	xxx put contents here xxx

	Written by:	David Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <2>	  7/1/94	DJ		making server handle errors in Comm layer
		 <1>	 5/27/94	DJ		first checked in

	To Do:
*/

#include "ServerCore.h"
#include "Server.h"
#include "ServerState.h"
#include "ServerDataBase.h"

#include <stdio.h>

int Server_UpdateNGPVersion(ServerState *state)
{
long				latestNGPVersion;
PreformedMessage	*mesg;

	latestNGPVersion = DataBase_GetLatestNGPVersion();
	
	if(latestNGPVersion > state->NGPVersion){
		// send NGP version and data.
		mesg = DataBase_GetLatestNGP();
		return(Server_SendPreformedMessage(state, mesg));
	}
	return(kServerFuncOK);
}


/*
MessErr DoNewNGPListOpCode( void )
{
MessErr			result;
NGPData			myData;
NGPData			*NGPDataPtr;
gameName		*theNames;
long			nameSize;
//
// Server sends this message anytime a new version of the Network Game Patch list is available
//
	GetDataSync( 6, (Ptr)&myData.version );	// Get version and count
	NGPDataPtr = (NGPData*) NewMemory( kTemp,  myData.count * 4 + 6 );
	ASSERT(NGPDataPtr);

	if( NGPDataPtr )
	{
		NGPDataPtr->version = myData.version;
		NGPDataPtr->count = myData.count;
		GetDataSync( 4*myData.count, (Ptr)&(NGPDataPtr->gameID[0]) );
	}
	
	nameSize = myData.count * sizeof( gameName );
	theNames = (gameName*) NewMemory( kTemp,  nameSize );
	ASSERT(theNames);

	if( theNames )
	{
		GetDataSync( nameSize, (Ptr)theNames );
	}
	
	result = 0;
	if( GetDataError() )
	{
		result = kFatalStreamError;
	}
	else
	{
	short iii;

		SetDBTypeUnstable( kGameNameType, true );
		SetDBTypeUnstable( kNGPType, true );
		UpdateNGPList( NGPDataPtr );
//
// Remove all the old game names
//
		ClearGameNames();

		for( iii = 0; iii < myData.count; iii++ )
		{
			SetGameName( NGPDataPtr->gameID[iii], &theNames[iii] );
		}
		
		SetDBTypeUnstable( kNGPType, false );
		SetDBTypeUnstable( kGameNameType, false );
	}

	DisposeMemory( NGPDataPtr );
	DisposeMemory( theNames );
	return result;
}
*/
