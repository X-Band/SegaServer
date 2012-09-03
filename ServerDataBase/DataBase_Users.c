/*
	File:		DataBase_Users.c

	Contains:	xxx put contents here xxx

	Written by:	David Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <4>	 8/17/94	ATM		Changed the "1" to be HAPPY_REGION (now in Server.h).
		 <3>	 8/15/94	DJ		made all new unique boxregions = 1 in order to invalidate all
									existing users (8/15/94)
		 <2>	 7/12/94	DJ		updated Database_GenerateUniqueBoxSerialNumber to new boxsernum
									format
		 <1>	 5/31/94	DJ		Handle user accounts, box serial numbers, etc

	To Do:
*/



#include <stdlib.h>

#include "Server.h"
#include "ServerDataBase_priv.h"
#include "Errors.h"

SDBUsers *SDBUsers_New(void)
{
SDBUsers *users;
	
	users = (SDBUsers *)malloc(sizeof(SDBUsers));
	ASSERT(users);
	if(!users)
	{
		ERROR_MESG("out of mems");
		return(NULL);
	}

	users->numUsers = 0;
	users->uniqueBoxSerialNumber = 1;
	users->timeStamp = 1;
	users->list = NewSortedList();

	return(users);
}

void Database_GenerateUniqueBoxSerialNumber(BoxSerialNumber *boxSerialNumber)
{
SDBUsers *users;

	users = SDB_GetUsers(gSDB);
	ASSERT(users);
	
	boxSerialNumber->box = users->uniqueBoxSerialNumber++;
	boxSerialNumber->region = HAPPY_REGION;
}

long Database_GetTimeStamp(void)
{
SDBUsers *users;

	users = SDB_GetUsers(gSDB);
	ASSERT(users);
	
	return(users->timeStamp);
}

long Database_IncrementTimeStamp(void)
{
SDBUsers *users;

	users = SDB_GetUsers(gSDB);
	ASSERT(users);
	
	return(++users->timeStamp);
}
