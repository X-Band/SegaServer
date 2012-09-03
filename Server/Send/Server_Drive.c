#ifdef OBSOLETE

/*
	File:		Server_Drive.c

	Contains:	Drive the box.

	Written by:	Dave Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<15>	  9/4/94	ATM		THIS FILE IS OBSOLETE and should go away.
		<14>	  8/4/94	ATM		Commented out basically everything.
		<13>	 6/11/94	DJ		commented out Server_SetupAccounts
		<12>	  6/9/94	BET		Fix hairy compiles based on TransportSlayer.h
		<11>	  6/5/94	DJ		making everything take a ServerState instead of SessionRec
		<10>	  6/3/94	DJ		mail from root
		 <9>	  6/1/94	DJ		SendMail
		 <8>	 5/31/94	DJ		mail handling and account verification
		 <7>	 5/29/94	DJ		wiggles
		 <6>	 5/29/94	DJ		made it actually drive the database etc
		 <5>	 5/27/94	DJ		made it actually drive the database etc
		 <4>	 5/26/94	DJ		loopback
		 <3>	 5/26/94	DJ		sends some text
		 <2>	 5/25/94	DJ		sends some text
	To Do:
*/

#include "Server.h"
#include "ServerState.h"
#include "Messages.h"
#include "UsrConfg.h"
#include "ServerDataBase.h"

#include "Dates.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static void Server_SetupAccounts(void)
{
userIdentification 	user1 = {13, 0, {"Root"}},
					user2 = {0, 0, {"Krazy Kon"}};

Mail	*from1;

//	user1.boxSerialNumber = Database_GenerateUniqueBoxSerialNumber();
	Database_CheckAccount(&user1);
	Database_CheckAccount(&user2);

	from1 = (Mail *)malloc(sizeof(Mail) + 200);
	from1->to = user2;
	from1->from = user1;
	from1->serialNumber = 0;
	from1->date = Date(1994, 5, 3);
	from1->iconRef = 0;
	strcpy(from1->title, "Welcome");
	strcpy(from1->message, "Catapult Entertainment welcomes you to The Zone!  Have fun playing our many entertaining games and give us your money, you slacker.");
	DataBase_AddMailToIncoming(from1);
	free(from1);

/*
Mail	*from2;

	from2 = (Mail *)malloc(sizeof(Mail) + 200);
	from2->to = user1;
	from2->from = user2;
	from2->serialNumber = 0;
	from2->date = 0;
	from2->iconRef = 0;
	strcpy(from2->title, "re: special message");
	strcpy(from2->message, "Jevans, you're right!");
	DataBase_AddMailToIncoming(from2);
	free(from2);
*/

}


/**********

We read everything the box sends and store it in a ServerState.
(could add a wait for validation at the box... leave for now).
Then figure out what to send the box (drive it).


Box sends:

Login
GameIDAndPatchVersion
SystemVersion
Challenge Request or CompetitiveGameRequest
NGP Version
SendQ (mail, addr books, game results, any personification shit)

We gather this all into state (or load into DB directly?)
We could send KoolStuff right after Login

Then we figure out what to send him....
new system version number and system+message patches
newest NGP
New game patch
incoming mail, addr book validation, ranking info, any personification shit
Game Play info (either wait for connection or dail this number)
End


*/

int Server_Drive(ServerState *state)
{

	ServerDataBase_Initialize();

	Server_SetupAccounts();


	Server_DownloadKoolStuff(state);

//	Server_SendLoopback(session, 1000);

/*


	if(Server_ValidateLogin(state) != kDataBase_AccountValid)
		return(kServerFuncAbort);

	Server_ValidateSystem(state);

	Server_UpdateNGPVersion(state);

	Server_UpdateGamePatch(state);
*/

	Server_ProcessSendQ(state);

	Server_SendMail(state);

/*
	Server_StartGamePlay(state);
*/

	ServerDataBase_Shutdown();

printf("Server_Drive done\n");
	return(kServerFuncOK);
}

#endif /*OBSOLETE*/

