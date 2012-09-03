/*
	File:		Server_SendBitmap.c

	Contains:	Send bitmaps to the wee box.

	Written by:	Dave Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <5>	 7/20/94	DJ		added Server_Comm stuff
		 <4>	 7/17/94	BET		Make printf's go to gLogFile instead of stdout, which is the
									modem.
		 <3>	  6/5/94	DJ		making everything take a ServerState instead of SessionRec and
									added Server_DebugService
		 <2>	 5/29/94	DJ		sync writing instead of async
		 <1>	 5/25/94	DJ		first checked in
	To Do:
*/

#include "ServerCore.h"
#include "Server.h"
#include "Messages.h"
#include "UsrConfg.h"
#include "SegaGraphics.h"

#include <stdio.h>
#include <string.h>

extern	FILE				*gLogFile;


int Server_SendBitmap(ServerState *state, SegaBitMap *bitMap, short xPos, short yPos)
{
/*
messOut	opCode;
short	bitmapLength;
short	messLength;


printf("Server_SendBitmap\n");

	opCode = msDrawBitMap;
	TWriteDataASync(session, sizeof(messOut), (Ptr)&opCode);

	bitmapLength = sizeof(SegaBitMap) + ???;	// how do we compute this?
	messLength = bitmapLength + sizeof(xPos) + sizeof(yPos);
	Server_TWriteDataSync(session, sizeof(messLength), (Ptr)&messLength);
	Server_TWriteDataSync(session, sizeof(xPos), (Ptr)&xPos);
	Server_TWriteDataSync(session, sizeof(yPos), (Ptr)&yPos);
	Server_TWriteDataSync(session, bitmapLength, (Ptr)bitMap);


printf("Server_SendBitmap done\n");
*/
	fprintf(gLogFile, "Server_SendBitmap is unimplemented\n");
	Server_DebugService(state);

	return(kServerFuncOK);
}

/*
MessErr DoDrawBitMapMessageOpCode( void )
{
long			length;
long			bitMapLength;
MessErr			result;
SegaBitMapPtr	myGraphic;
short			xpos;
short			ypos;

	GetDataSync( 4, (Ptr)&length );
	GetDataSync( 2, (Ptr)&xpos );
	GetDataSync( 2, (Ptr)&ypos );
	bitMapLength = length - sizeof(xpos) - sizeof(ypos);
	myGraphic = (SegaBitMapPtr)NewMemory( kTemp,  bitMapLength );
	GetDataSync( bitMapLength, (Ptr)myGraphic );

	result = 0;
	if( GetDataError() )
	{
		result = kFatalStreamError;
	}
	else
	{
		DrawGraphic( myGraphic, xpos, ypos, 0 );
		VDPIdle();
	}
	DisposeMemory( myGraphic );
	return result;
}
*/
