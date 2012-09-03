
/*
	File:		Server_DownloadKoolStuff.c

	Contains:	Server Download kool stuff function

	Written by:	Dave Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<35>	 8/31/94	ATM		Sanity-check numPages in dlKoolStuff.
		<34>	 8/24/94	DJ		added sends of DBConsts for ted.  These should be moved to
									Brian's tool.
		<33>	 8/20/94	DJ		added an abort for Brian to test Transport bug
		<32>	 8/15/94	DJ		turned on send of news validate if no news, cuz it is a kon bug.
		<31>	 8/15/94	DJ		added send of news validate (but commented out until we are
									smart enuf to know if we've ever sent news to it).
		<30>	 8/12/94	ATM		Converted to Logmsg.
		<29>	 8/10/94	DJ		countdown starts at 30 instead of 120 now.
		<28>	  8/8/94	DJ		sends news everytime now
		<27>	  8/2/94	DJ		including DBConstants.h
		<26>	  8/2/94	DJ		bracketing with Server_SetTransportHold
		<25>	 7/31/94	DJ		changed the times in countdown
		<24>	 7/29/94	DJ		setting the news countdown kNewsCountdownConst
		<23>	 7/27/94	DJ		sending it again
		<22>	 7/26/94	DJ		aaaaaah news!
		<21>	 7/25/94	DJ		commented out kTest dialog
		<20>	 7/20/94	DJ		no ServerState passed to dbase routines
		<19>	 7/18/94	DJ		send 1st kind of news only once
		<18>	 7/17/94	BET		Make printf's go to gLogFile instead of stdout, which is the
									modem.
		<17>	  7/8/94	DJ		send both types of news
		<16>	  7/7/94	DJ		removed unsavory dialog
		<15>	  7/2/94	DJ		rankings
		<14>	  7/1/94	DJ		making server handle errors from the comm layer
		<13>	 6/30/94	DJ		turned news on for teddis
		<12>	 6/30/94	DJ		better news
		<11>	  6/5/94	DJ		don't send a test image
		<10>	  6/5/94	DJ		making everything take a ServerState instead of SessionRec and
									added Server_DebugService
		 <9>	  6/3/94	DJ		more news goo
		 <8>	  6/2/94	BET		send news
		 <7>	  6/1/94	DJ		text color and sending news
		 <6>	  6/1/94	BET		text color
		 <5>	  6/1/94	DJ		poo
		 <4>	 5/31/94	DJ		don't send image cuz Zeus is wrong today
		 <3>	 5/29/94	DJ		sync writing instead of async
		 <2>	 5/29/94	DJ		added dbase lookup of koolstuff msg
		 <2>	 5/25/94	DJ		bug
		 <1>	 5/25/94	DJ		first checked in
	To Do:
*/

#include "ServerCore.h"
#include "Server.h"
#include "Messages.h"
#include "ServerDataBase.h"
#include "Server_Comm.h"

#include "DBTypes.h"
#include "DBConstants.h"

#include <stdio.h>


/*
	This treat can use the info in ServerState to do lookups to send targetted information
	such as:
		ranking specicific mail
		game specific info (eg. realtime NBA scores and stats)
		ads
	
*/
int Server_DownloadKoolStuff(ServerState *state)
{
PreformedMessage	*preformed;
ServerNewsPage		*page;
long				numPages, a, b;
OSErr				err;
DBType				pagetype;
DBID				id;
long				countdown;
Boolean				sentNews = false;

	Logmsg("Server_DownloadKoolStuff\n");
	if(Server_DebugService(state) != kServerFuncOK)
		return(kServerFuncAbort);


//
/////////////////////////////////////////
//
// BRAIN DAMAGE.
// Set some DBConstants for ted.  These should be done by Topping's tool.
//
	id = kListenCallWaitingTimeConst;
	countdown = 5;
	if(Server_SendDBConstants(state, 1, &id, &countdown) != kServerFuncOK)
		return(kServerFuncAbort);

	id = kDBTransportTickleConst;
	countdown = 5;
	if(Server_SendDBConstants(state, 1, &id, &countdown) != kServerFuncOK)
		return(kServerFuncAbort);
//
/////////////////////////////////////////
//





	Server_SetTransportHold(true);

	id = kNewsCountdownConst;
	countdown = 60L << 16;	// ticks per count
	countdown |= 30;		// the count value (30 sec)
	if(Server_SendDBConstants(state, 1, &id, &countdown) != kServerFuncOK)
		return(kServerFuncAbort);

	//
	// Send the general daily news (everyone gets this)
	// Send the targetted Other news (game specific, etc)
	//
//abort();	// makes TransportLayer blow up cuz TUUnthread never called if error is found by TIndication

	for(b = 0; b < 2; b++)
	{
		if(b == 1)
			pagetype = kOtherNews;
		else
			pagetype = kDailyNews;

		numPages = DataBase_GetNumNewsPages(pagetype);
		if (numPages < 0 || numPages > 64) {
			Logmsg("ERROR: got bogus numPages %ld in Server_DownloadKoolStuff\n",
				numPages);
			return (kServerFuncAbort);
		}

		for(a = 0; a < numPages; a++)
		{
			sentNews = true;

			page = DataBase_GetNewsPage(a, pagetype);
	
			ASSERT(page);
			if(!a)
				err = Server_SendFirstNewsPage( state, page);
			else
				err = Server_SendNewsPage(state, page);
		
			DataBase_ReleaseServerNewsPage( page);
			
			if(err != kServerFuncOK)
				return(kServerFuncAbort);
		}
	}

	if(!sentNews)
		Server_SendNoNewsPage(state, kDailyNews);	// Box doesn't care about the pagetype, so anything will do.


	//
	// If you want to speed up the countdown, you can change the ticks per count.
	// The count value cannot be changed after the timer starts.
	//
	countdown = 60L << 16;	// ticks per count (you would change this to change tick speed)
	countdown |= 120;		// the count value (ignored after timer is started)
	if(Server_SendDBConstants(state, 1, &id, &countdown) != kServerFuncOK)
		return(kServerFuncAbort);


	Server_SetTransportHold(false);

/*
	preformed = DataBase_GetKoolStuff();
	if(preformed)
	{
// BROKEN NOGGIN.... 
//		Server_SendPreformedMessage(state, preformed);
		DataBase_ReleasePreformedMessage(preformed);
	}
*/

	return(kServerFuncOK);
}

