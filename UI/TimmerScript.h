/*
	File:		TimmerScript.h

	Contains:	Header for TimmerScript

	Written by:	Timmer, Brian Topping

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <1>	 6/30/94	BET		first checked in

	To Do:
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define		kMaxBuffLen		1024


typedef struct ScriptCmd ScriptCmd;

typedef ScriptCmd * (*ScriptProcPtr)( ScriptCmd * scriptCmd );

struct ScriptCmd
	{
	ScriptCmd *			scriptNext;
	ScriptProcPtr		scriptProc;
	char				scriptData[1000];
	};

typedef struct SParse SParse;
struct SParse
	{
	char *				scriptToken;
	ScriptProcPtr		scriptProc;
	};


void	DoScript( StandardFileReply *reply );


static	FILE *			OpenScript( StandardFileReply *reply );
static	ScriptCmd *		ParseScript( FILE * );
static	void			ExecuteScript( ScriptCmd * );
static	void			DisposeScript( ScriptCmd * );
static	void			strupr( char * );
static	char *			GetQuotedStr( void );

static	ScriptCmd *		ScriptPInit( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptPOpen( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptPListen( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptPClose( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptPNetIdle( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptPWritePacketSync( ScriptCmd * scriptCmd );	//
static	ScriptCmd *		ScriptPWritePacketASync( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptTInit( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptTOpen( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptTOpenAsync( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptTListen( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptTListenAsync( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptTClose( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptTCloseAsync( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptTNetIdle( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptTReadDataSync( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptTReadDataASync( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptTWriteDataSync( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptTWriteDataAsync( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptTReadData( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptTWriteData( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptTReadAByte( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptTWriteAByte( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptTQueueAByte( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptTReadBytesReady( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptTNetError( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptTDataReady( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptTDataReadySess( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptLogCommands( ScriptCmd * scriptCmd );			//
static	ScriptCmd *		ScriptDefine( ScriptCmd * scriptCmd );				//
static	ScriptCmd *		ScriptSetRandom( ScriptCmd * scriptCmd );			//
static	ScriptCmd *		ScriptWait( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptBeep( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptEndLoop( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptIf( ScriptCmd * scriptCmd );					//
static	ScriptCmd *		ScriptIfRandom( ScriptCmd * scriptCmd );
static	ScriptCmd *		ScriptElse( ScriptCmd * scriptCmd );				//
static	ScriptCmd *		ScriptSetRandSeed( ScriptCmd * scriptCmd );			//
static	ScriptCmd *		ScriptTrashStartFrame( ScriptCmd * scriptCmd );		//
static	ScriptCmd *		ScriptTrashCRC( ScriptCmd * scriptCmd );			//
static	ScriptCmd *		ScriptTrashEndFrame( ScriptCmd * scriptCmd );		//
static	ScriptCmd *		ScriptTrashHeader( ScriptCmd * scriptCmd );			//
static	ScriptCmd *		ScriptTrashData( ScriptCmd * scriptCmd );			//
static	ScriptCmd *		ScriptInsertNoise( ScriptCmd * scriptCmd );			//

static	void			FillWriteBuffer( unsigned char *, long );
static	void			CheckReadBuffer( unsigned char *, long );


char					*GetErrStr(OSErr theErr);
