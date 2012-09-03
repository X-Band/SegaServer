/*
	File:		Server_SendText.c

	Contains:	xxx put contents here xxx

	Written by:	David Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<10>	 7/20/94	DJ		added Server_Comm stuff
		 <9>	 7/17/94	BET		Make printf's go to gLogFile instead of stdout, which is the
									modem.
		 <8>	 7/12/94	DJ		using Server_TCheckError instead of TCheckError
		 <7>	  7/1/94	DJ		making server handle errors from the comm layer
		 <6>	  6/9/94	BET		Fix hairy compiles based on TransportSlayer.h
		 <5>	  6/5/94	DJ		making everything take a ServerState instead of SessionRec and
									added Server_DebugService
		 <4>	 5/29/94	DJ		sync writing instead of async
		 <3>	 5/25/94	DJ		fixed SetDevice (wasn't sending one of the fields, thus client
									hung).
		 <2>	 5/25/94	DJ		first checked in
		 <1>	 5/25/94	DJ		first checked in

	To Do:
*/

#include "ServerCore.h"
#include "Server.h"
#include "Messages.h"
#include "UsrConfg.h"
#include "SegaGraphics.h"
#include "Server_Comm.h"

#include <stdio.h>
#include <string.h>

extern	FILE				*gLogFile;

int Server_SendText(ServerState *state, char *text, short xPos, short yPos)
{
messOut	opCode;
short	textLength;
short	messLength;


printf("Server_SendText\n");

	opCode = msFormatText;
	Server_TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);

	textLength = strlen(text);	// don't send the NULL character.
	messLength = textLength + sizeof(xPos) + sizeof(yPos);
	Server_TWriteDataSync(state->session, sizeof(messLength), (Ptr)&messLength);
	Server_TWriteDataSync(state->session, sizeof(xPos), (Ptr)&xPos);
	Server_TWriteDataSync(state->session, sizeof(yPos), (Ptr)&yPos);
	Server_TWriteDataSync(state->session, textLength, (Ptr)text);

	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

printf("Server_SendText done\n");


	return(kServerFuncOK);
}

/*

MessErr DoFormatTextMessageOpCode( void )
{
short	length;
long	textLength;
Ptr		theText;
MessErr	result;
short	xpos;
short	ypos;

	//get the size and alloc the memory
		
	GetDataSync( 2, (Ptr)&length );
	GetDataSync( 2, (Ptr)&xpos );
	GetDataSync( 2, (Ptr)&ypos );
	textLength = length-sizeof(xpos)-sizeof(ypos);
	theText = NewMemory( kTemp,  textLength+1 );
	
//fill 'er up
	
	GetDataSync( textLength, theText );
//
// Downloads a game patch and adds it to the game patch database
//
	result = 0;
	if( GetDataError() )
	{
		DisposeMemory( theText );
		result = kFatalStreamError;
	}
	else
	{
		theText[textLength] = 0;	//make it a C-string
		DrawSegaString( &xpos, &ypos, theText );
		VDPIdle();
	}
	DisposeMemory( theText );
	return result;
}

*/

int Server_SendSetupTextDevice(ServerState *state, short left, short top, short right, short bottom)
{
messOut	opCode;
segaCharRect rect;
unsigned char device;

	fprintf(gLogFile, "Server_SendSetupTextDevice\n");
	if(Server_DebugService(state) != kServerFuncOK)
		return(kServerFuncAbort);

	opCode = msCreateTextDevice;
	Server_TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);

	device = 1;
	Server_TWriteDataSync(state->session, 1, (Ptr)&device);

	rect.left = left;
	rect.top = top;
	rect.right = right;
	rect.bottom = bottom;
	Server_TWriteDataSync(state->session, sizeof(segaCharRect), (Ptr)&rect);

	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	fprintf(gLogFile, "Server_SendSetupTextDevice done\n");

	return(kServerFuncOK);
}


/*
MessErr DoSetupTextGDeviceMessageOpCode( void )
{
segaCharRect 	myRect;
unsigned char	newDevice;
long			patternStart;
MessErr			result;

	GetDataSync( 1, (Ptr)&newDevice );
	GetDataSync( 8, (Ptr)&myRect );

	result = 0;
	if( GetDataError() )
	{
		result = kFatalStreamError;
	}
	else
	{
		SetCurrentDevice( newDevice );
		patternStart = LinearizeScreenArea( &myRect, 0 );
		SetupTextGDevice( &myRect, patternStart );
	}
	return result;
}
*/

int Server_SendTextColor(ServerState *state, unsigned char color0, unsigned char color1, unsigned char color2, unsigned char color3)
{
messOut	opCode;

	fprintf(gLogFile, "Server_SendTextColor\n");
	if(Server_DebugService(state) != kServerFuncOK)
		return(kServerFuncAbort);

	opCode = msSetTextColor;
	Server_TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);
	Server_TWriteDataSync(state->session, 1, (Ptr)&color0);
	Server_TWriteDataSync(state->session, 1, (Ptr)&color1);
	Server_TWriteDataSync(state->session, 1, (Ptr)&color2);
	Server_TWriteDataSync(state->session, 1, (Ptr)&color3);

	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	fprintf(gLogFile, "Server_SendTextColor done\n");

	return(kServerFuncOK);
}




/*
MessErr DoSetTextColorMessageOpCode( void )
{
unsigned char	color0, color1, color2, color3;
MessErr			result;

	GetDataSync( 1, (Ptr)&color0 );
	GetDataSync( 1, (Ptr)&color1 );
	GetDataSync( 1, (Ptr)&color2 );
	GetDataSync( 1, (Ptr)&color3 );

	result = 0;
	if( GetDataError() )
	{
		result = kFatalStreamError;
	}
	else
	{
		SetFontColors( color0, color1, color2, color3 );
	}
	return result;
}
*/

