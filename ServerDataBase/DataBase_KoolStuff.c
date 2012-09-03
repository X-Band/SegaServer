/*
	File:		DataBase_KoolStuff.c

	Contains:	xxx put contents here xxx

	Written by:	David Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <8>	 7/20/94	DJ		no ServerState passed to dbase routines
		 <7>	 6/11/94	DJ		using PreformedMessage_ReadFromFile
		 <6>	  6/4/94	DJ		test.smsg
		 <5>	  6/3/94	DJ		news2
		 <4>	  6/1/94	DJ		News.smsg
		 <3>	 5/31/94	DJ		added nasty comments about mems leaks
		 <2>	 5/29/94	DJ		tweaks
		 <1>	 5/29/94	DJ		first checked in

	What:
		DataBase_GetKoolStuff can use the info in ServerState to do lookups to send targetted
		information such as:
			ranking specicific mail
			game specific info (eg. realtime NBA scores and stats)
			ads

	To Do:
*/


#include "Server.h"
#include "ServerDataBase.h"
#include "Errors.h"

#include <stdio.h>
#include <stdlib.h>


//
// == BRAIN DAMAGE ==
// this fucker will leak mems cuz ReleasePreformedMessage doesn't call free anymore.
// 5/30/94 dj
//
// It should be looking this up in the DB instead of allocating here anyway.
//
PreformedMessage *DataBase_GetKoolStuff(void)
{
PreformedMessage *message;

return(NULL);

	message = PreformedMessage_ReadFromFile("test.smsg");
	
	return(message);
}
