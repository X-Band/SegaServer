/*
	File:		Server_Log.c

	Contains:	Logging and some debugging stuff

	Written by:	Andy McFadden

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<20>	 9/18/94	ATM		THIS FILE IS NOW OBSOLETE.  See Common_Log.c.
		<19>	 9/16/94	ATM		Don't show pid for rpclog stuff, either.
		<18>	 9/16/94	ATM		Don't print pid on statuslog.
		<17>	 9/16/94	ATM		Added LOG_NETERR.
		<16>	 9/11/94	ATM		Moved gProduction in here too.
		<15>	 9/11/94	ATM		Shuffled things around slightly to make it easier to use this
									file everywhere.
		<14>	  9/7/94	ATM		Added LOG_RPC.
		<13>	  9/6/94	ATM		Added LOG_MATCHING.
		<12>	  9/3/94	ATM		Changed LOG_DEBUG to LOG_DBUG.
		<11>	  9/1/94	ATM		Initialize charbuf in FLoghexdump.
		<10>	  9/1/94	ATM		Rearranged all the logging stuff (again).
		 <9>	 8/20/94	BET		Oops, ascii comparison needed to be >= & <=
		 <8>	 8/20/94	BET		Add ascii dump to Loghexdump.
		 <7>	 8/17/94	ATM		Tweak.
		 <6>	 8/17/94	ATM		Added Crasmsg stuff, per request.
		 <5>	 8/17/94	ATM		Made gProduction a bit more blatant.
		 <4>	 8/17/94	ATM		Added Loghexdump.
		 <3>	 8/15/94	ATM		Now handle lines with no terminating '\n' in a clean way.
		 <2>	 8/13/94	ATM		Fixed gLogFile stuff for Mac server.
		 <1>	 8/13/94	ATM		first checked in

	To Do:
*/

// all gone!

