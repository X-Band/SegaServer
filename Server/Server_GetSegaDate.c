/*
	File:		Server_GetSegaDate.c

	Contains:	unix breakout crud

	Written by:	Fucker Eat Shit
	
	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <7>	 8/18/94	DJ		compile on Mac
		 <6>	 8/16/94	DJ		senddateandtime
		 <5>	 8/11/94	ATM		Converted to Logmsg.
		 <4>	 7/17/94	BET		Make printf's go to gLogFile instead of stdout, which is the
									modem.
		 <3>	 7/15/94	DJ		minor tweak
		 <2>	 7/14/94	DJ		added #include studio.h
		 <1>	 7/13/94	dwh		first checked in

	To Do:
*/


#ifdef __MWERKS__
#include <UNIXtime.h>	// cuz KON has a time.h for sega.  shit.
#else
#include <time.h>
#endif

#include "ServerCore.h"
#include "Server.h"
#include "Messages.h"
#include "ServerDataBase.h"
#include "Dates.h"
#include "TransportLayer.h"
#include "PhysicalLayer.h"
#include "NetMisc.h"
#include <stdio.h>
#include "Server_Comm.h"


long
Server_GetSegaDate(void)
{
static long segaDate;
long year, month, day;
struct tm *datep;
time_t now;

	time(&now);
	datep 		= localtime(&now);
	year 		= datep->tm_year + 1900;
	month 		= datep->tm_mon;
	day 		= datep->tm_mday;
	segaDate 	= Date(year, month, day);
	Logmsg("year = %ld, month = %ld, day = %ld\n", year, month, day);

	return(segaDate);
}

int Server_SendDateAndTime(ServerState *state)
{
messOut				opCode;
long				segaDate, segaTime;

	Logmsg("Server_SendDateAndTime\n");

	opCode = msSetDateAndTime;
	Server_TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);

	segaTime = 0;	// Sega has no notion of time yet.
	segaDate = Server_GetSegaDate();

	Server_TWriteDataSync(state->session, sizeof(long), (Ptr)&segaDate);
	Server_TWriteDataSync(state->session, sizeof(long), (Ptr)&segaTime);

	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	Logmsg("Server_SendDateAndTime done\n");

	return(kServerFuncOK);
}
