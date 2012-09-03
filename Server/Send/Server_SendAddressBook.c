
/*
	File:		Server_SendAddressBook.c

	Contains:	Server send address book function

	Written by:	Dave Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <4>	 8/12/94	ATM		Converted to Logmsg.
		 <3>	 7/17/94	BET		Make printf's go to gLogFile instead of stdout, which is the
									modem.
		 <2>	  6/5/94	DJ		nothing
		 <1>	 5/25/94	DJ		first checked in
	To Do:
*/

#include "ServerCore.h"
#include "Server.h"
#include "Messages.h"
#include <stdio.h>


int Server_SendAddressBook()
{
	Logmsg("Server_SendAddressBook\n");

	return(kServerFuncOK);
}

