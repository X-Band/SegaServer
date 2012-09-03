/*
	File:		TimmerScript.c

	Contains:	Test script interpreter for ServerTalk

	Written by:	Timmer, Brian Topping

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <6>	 7/28/94	BET		Add gLogFile changes.
		 <5>	 7/25/94	DJ		fixed to long flags
		 <4>	  7/4/94	DJ		update GetDataBytesReady
		 <3>	  7/3/94	DJ		update to PListen change
		 <2>	  7/2/94	BET		Add POpen changes
		 <1>	 6/30/94	BET		first checked in

	To Do:
*/

/*
**	Script.c
**
**	This file opens, tokenizes, and executes a text script file.
**	It can call the important interface routines.
**	In addition it has rudimentary flow control.
**	Here are some notes on the implemented commands:
**		WAIT time				waits for time ticks - calls NetIdle if PInit has been called
**		LOOP count				executes the commands up until the following ENDLOOP count times
**		ENDLOOP
**		IFRANDOM thresh max		executes the commands up until the following ENDIF if a rondomly
**		ENDIF					generated number between 0 and max-1 is less than thresh
**
**	Commands could be added rather easily.
**	Under Other Commands below you'll see some that I might have implemented if I had 
**	not come down with that stupid cold.
**
*/


/*
	We assume that the WDS passed to PWritePacket never has more than 2 elements
	
	Physical Script Commands
		PInit
		POpen
		PListen
		PClose
		PNetIdle
		PWritePacketSync length1 length2
		PWritePacketASync length1 length2
	
	Transport Script Commands
		TInit
		TOpen
		TOpenAsync
		TListen timeout
		TListenAsync timeout
		TClose
		TCloseAsync
		TNetIdle
		TReadDataSync length
		TReadDataASync length
		TWriteDataSync length
		TWriteDataAsync length
		TReadData length syncFlag
		TWriteData length syncFlag
		TReadAByte
		TWriteAByte byte
		TQueueAByte byte
		TReadBytesReady
		TNetError
		TDataReady
		TDataReadySess
	
	Other Commands
		LogCommands flag			(nyi)
		Define name value			(nyi)
		SetRandom name min max		(nyi)
		Wait ticks
		Loop count
		EndLoop
		If flag						(nyi)
		IfRandom min max
		Else						(nyi)
		EndIf
		SetRandSeed value			(nyi)
		TrashStartFrame				(special)
		TrashEndFrame
		TrashData
		TrashHeader
		InsertNoise					(nyi)
*/


/*
	Errors reported in output file:
		Bad Data		our private checksum does not match
		Message Lost	we expect to get all messages that were sent.
						messages are assigned an increasing transaction id.
						we detect an error when one is skipped.
*/


// undef this to get log flowing to pretty scrolling window (that sometimes crashes)
//#define SCRIPT 1

#include "PhysicalLayer.h"
#include "TransportLayer.h"
#include "TimmerScript.h"
#include "printf.h"
#include "NetErrors.h"
#include "CSegaServerApp.h"



static	SessionRec			gSession;
static	NetParamBlock		gNetBlock;
static	short				gSendTransID;
static	short				gRecvTransID;
		Boolean				bTrashStartFrame;
		Boolean				bTrashCRC;
		Boolean				bTrashEndFrame;
		Boolean				bTrashData;
		Boolean				bTrashHeader;
static	Boolean				bCanNetIdle;
extern	FILE				*gLogFile;

void
DoScript(StandardFileReply *reply)
{
	short			result;
	FILE *			stream;
	ScriptCmd *		scriptCmds;
	
	result     = noErr;
	stream     = nil;
	scriptCmds = nil;
	
	//if ( result == noErr )
		{
		stream = OpenScript( reply );
		if ( stream == nil )
			{
			result = -1;
			}
		}
	if ( result == noErr )
		{
		scriptCmds = ParseScript( stream );
		if ( scriptCmds == nil )
			{
			result = -1;
			}
		}
	if ( result == noErr )
		{
		ExecuteScript( scriptCmds );
		}
	
	if ( scriptCmds )
		{
		DisposeScript( scriptCmds );
		}
	if ( stream )
		{
		fclose( stream );
		}
}


static FILE *
OpenScript(StandardFileReply *reply)
{
	FILE *					stream;
	OSType					fileType = 'TEXT';
	Str255					buff;
	short					vRefNum;
	long					dirID;
		
	stream = nil;
	HGetVol( buff, &vRefNum, &dirID );
	HSetVol( nil, reply->sfFile.vRefNum, reply->sfFile.parID );
	
	p2cstr( (StringPtr) &reply->sfFile.name );
	stream = fopen( (char *) &reply->sfFile.name, "r" );
	fprintf(gLogFile, "\n%s\n",(char *) &reply->sfFile.name);
	if ( stream == nil )
		{
		DebugStr( "\punable to open script file." );
		}
	
	HSetVol( buff, vRefNum, dirID );

	return stream;
}


static ScriptCmd *
ParseScript( FILE * stream )
{
	Boolean				bReturnFlag;
	Boolean				bDisposeError;
	long				size;
	long				count;
	char *				token;
	ScriptCmd *			rootCmd = nil;
	ScriptCmd *			newCmd;
	ScriptCmd *			nextCmd;
	ScriptCmd *			firstCmd;
	ScriptCmd **		backLink;
	char				buff[256];
	
	backLink = &rootCmd;
	
	bReturnFlag   = false;
	bDisposeError = false;
	
	while ( bReturnFlag   == false
	   &&   bDisposeError == false
	   &&   fgets( &buff[0], sizeof(buff), stream ) != nil )
		{
		// get the first token
		token = strtok( &buff[0], " \t\n" );
		
		// check for blank line or comment
		if ( token == nil )
			{
			continue;
			}
		if ( *token == '#' )
			{
			continue;
			}
		
		// allocate a new script command
		size = sizeof(ScriptCmd);
		newCmd = (ScriptCmd *) NewPtrClear( size );
		if ( newCmd == nil )
			{
			DebugStr( "\punable to allocate memory for script command." );
			bDisposeError = true;
			break;
			}
		size -= sizeof(newCmd->scriptData);
		
		// find the type
		strupr( token );
		if ( strcmp( token, "WAIT" ) == 0 )
			{
			newCmd->scriptProc = ScriptWait;
			token = strtok( nil, " \t\n" );
			if ( token == nil )
				{
				DebugStr( "\pexpected number after Wait command." );
				bDisposeError = true;
				break;
				}
			size += sizeof(long);
			* (long *) &newCmd->scriptData[0] = atol( token );
			}
		else
		if ( strcmp( token, "BEEP" ) == 0 )
			{
			newCmd->scriptProc = ScriptBeep;
			}
		else
		if ( strcmp( token, "LOOP" ) == 0 )
			{
			token = strtok( nil, " \t\n" );
			if ( token == nil )
				{
				DebugStr( "\pexpected number after Loop command." );
				bDisposeError = true;
				break;
				}
			count = atol( token );
			DisposePtr( (Ptr) newCmd );
			firstCmd = ParseScript( stream );
			if ( firstCmd == nil )
				{
				DebugStr( "\pno body for Loop command." );
				bDisposeError = true;
				break;
				}
			newCmd = firstCmd;
			nextCmd = newCmd->scriptNext;
			while ( nextCmd )
				{
				newCmd = nextCmd;
				nextCmd = newCmd->scriptNext;
				}
			* (long *) &newCmd->scriptData[0] = count;
			*backLink = firstCmd;
			backLink = &newCmd->scriptNext;
			newCmd = nil;
			}
		else
		if ( strcmp( token, "ENDLOOP" ) == 0 )
			{
			newCmd->scriptProc = ScriptEndLoop;
			size += sizeof(long) + sizeof(long) + sizeof(ScriptCmd*);
			* (long *)       &newCmd->scriptData[4] = 1;
			* (ScriptCmd **) &newCmd->scriptData[8] = rootCmd;
			bReturnFlag = true;
			}
		else
		if ( strcmp( token, "IFRANDOM" ) == 0 )
			{
			newCmd->scriptProc = ScriptIfRandom;
			size += 2 * sizeof(long) + sizeof(ScriptCmd*);
			token = strtok( nil, " \t\n" );
			if ( token )
				{
				* (long *) &newCmd->scriptData[0] = atol( token );
				token = strtok( nil, " \t\n" );
				}
			if ( token == nil )
				{
				DebugStr( "\pexpected thresh and max after IfRandom command." );
				bDisposeError = true;
				break;
				}
			* (long *) &newCmd->scriptData[4] = atol( token );
			SetPtrSize( (Ptr) newCmd, size );
			firstCmd = ParseScript( stream );
			if ( firstCmd == nil )
				{
				DebugStr( "\pno body for IfRandom command." );
				bDisposeError = true;
				}
			newCmd->scriptNext = firstCmd;
			nextCmd = firstCmd->scriptNext;
			while ( nextCmd )
				{
				firstCmd = nextCmd;
				nextCmd = firstCmd->scriptNext;
				}
			*backLink = newCmd;
			backLink = &firstCmd->scriptNext;
			* (ScriptCmd **) &newCmd->scriptData[8] = firstCmd;
			newCmd = nil;
			}
		else
		if ( strcmp( token, "ENDIF" ) == 0 )
			{
			DisposePtr( (Ptr) newCmd );
			newCmd = nil;
			bReturnFlag = true;
			}
		else
		if ( strcmp( token, "TRASHSTARTFRAME" ) == 0 )
			{
			newCmd->scriptProc = ScriptTrashStartFrame;
			}
		else
		if ( strcmp( token, "TRASHCRC" ) == 0 )
			{
			newCmd->scriptProc = ScriptTrashCRC;
			}
		else
		if ( strcmp( token, "TRASHENDFRAME" ) == 0 )
			{
			newCmd->scriptProc = ScriptTrashEndFrame;
			}
		else
		if ( strcmp( token, "TRASHHEADER" ) == 0 )
			{
			newCmd->scriptProc = ScriptTrashHeader;
			}
		else
		if ( strcmp( token, "TRASHDATA" ) == 0 )
			{
			newCmd->scriptProc = ScriptTrashData;
			}
		else
		if ( strcmp( token, "PINIT" ) == 0 )
			{
			newCmd->scriptProc = ScriptPInit;
			}
		else
		if ( strcmp( token, "POPEN" ) == 0 )
			{
			newCmd->scriptProc = ScriptPOpen;
			token = strtok( nil, " \t\n" );
			if ( token == nil )
				{
				DebugStr( "\pexpected number after POpen command." );
				bDisposeError = true;
				break;
				}
			size += sizeof(long);
			* (long *) &newCmd->scriptData[0] = atol( token );
			}
		else
		if ( strcmp( token, "PLISTEN" ) == 0 )
			{
			newCmd->scriptProc = ScriptPListen;
			token = strtok( nil, " \t\n" );
			if ( token == nil )
				{
				DebugStr( "\pexpected number after POpen command." );
				bDisposeError = true;
				break;
				}
			size += sizeof(long);
			* (long *) &newCmd->scriptData[0] = atol( token );
			}
		else
		if ( strcmp( token, "PCLOSE" ) == 0 )
			{
			newCmd->scriptProc = ScriptPClose;
			}
		else
		if ( strcmp( token, "PNETIDLE" ) == 0 )
			{
			newCmd->scriptProc = ScriptPNetIdle;
			}
		else
		if ( strcmp( token, "PWRITEPACKETSYNC" ) == 0 )
			{
			newCmd->scriptProc = ScriptPWritePacketSync;
			size += 2 * sizeof(long);
			token = strtok( nil, " \t\n" );
			if ( token )
				{
				* (long *) &newCmd->scriptData[0] = atol( token );
				token = strtok( nil, " \t\n" );
				}
			if ( token == nil )
				{
				DebugStr( "\pexpected two lengths after PWritePacketSync command." );
				bDisposeError = true;
				break;
				}
			* (long *) &newCmd->scriptData[4] = atol( token );
			}
		else
		if ( strcmp( token, "PWRITEPACKETASYNC" ) == 0 )
			{
			newCmd->scriptProc = ScriptPWritePacketASync;
			size += 2 * sizeof(long);
			token = strtok( nil, " \t\n" );
			if ( token )
				{
				* (long *) &newCmd->scriptData[0] = atol( token );
				token = strtok( nil, " \t\n" );
				}
			if ( token == nil )
				{
				DebugStr( "\pexpected two lengths after PWritePacketASync command." );
				bDisposeError = true;
				break;
				}
			* (long *) &newCmd->scriptData[4] = atol( token );
			}
		else
		if ( strcmp( token, "TINIT" ) == 0 )
			{
			newCmd->scriptProc = ScriptTInit;
			}
		else
		if ( strcmp( token, "TOPEN" ) == 0 )
			{
			newCmd->scriptProc = ScriptTOpen;
			}
		else
		if ( strcmp( token, "TOPENASYNC" ) == 0 )
			{
			newCmd->scriptProc = ScriptTOpenAsync;
			}
		else
		if ( strcmp( token, "TLISTEN" ) == 0 )
			{
			newCmd->scriptProc = ScriptTListen;
			token = strtok( nil, " \t\n" );
			if ( token == nil )
				{
				DebugStr( "\pexpected number after TListen command." );
				bDisposeError = true;
				break;
				}
			size += sizeof(long);
			* (long *) &newCmd->scriptData[0] = atol( token );
			}
		else
		if ( strcmp( token, "TLISTENASYNC" ) == 0 )
			{
			newCmd->scriptProc = ScriptTListenAsync;
			token = strtok( nil, " \t\n" );
			if ( token == nil )
				{
				DebugStr( "\pexpected number after TListenAsync command." );
				bDisposeError = true;
				break;
				}
			size += sizeof(long);
			* (long *) &newCmd->scriptData[0] = atol( token );
			}
		else
		if ( strcmp( token, "TCLOSE" ) == 0 )
			{
			newCmd->scriptProc = ScriptTClose;
			}
		else
		if ( strcmp( token, "TCLOSEASYNC" ) == 0 )
			{
			newCmd->scriptProc = ScriptTCloseAsync;
			}
		else
		if ( strcmp( token, "TNETIDLE" ) == 0 )
			{
			newCmd->scriptProc = ScriptTNetIdle;
			}
		else
		if ( strcmp( token, "TREADDATASYNC" ) == 0 )
			{
			newCmd->scriptProc = ScriptTReadDataSync;
			token = strtok( nil, " \t\n" );
			if ( token == nil )
				{
				DebugStr( "\pexpected number after TReadDataSync command." );
				bDisposeError = true;
				break;
				}
			size += sizeof(long);
			* (long *) &newCmd->scriptData[0] = atol( token );
			}
		else
		if ( strcmp( token, "TREADDATAASYNC" ) == 0 )
			{
			newCmd->scriptProc = ScriptTReadDataASync;
			token = strtok( nil, " \t\n" );
			if ( token == nil )
				{
				DebugStr( "\pexpected number after TReadDataASync command." );
				bDisposeError = true;
				break;
				}
			size += sizeof(long);
			* (long *) &newCmd->scriptData[0] = atol( token );
			}
		else
		if ( strcmp( token, "TWRITEDATASYNC" ) == 0 )
			{
			newCmd->scriptProc = Sc