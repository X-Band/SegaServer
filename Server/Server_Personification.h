/*
	File:		Server_Personification.h

	Contains:	xxx put contents here xxx

	Written by:	Brian Topping

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <4>	 8/15/94	DJ		sendpersonification
		 <3>	 8/11/94	DJ		new personifuckation that takes/sends a flag of what changed
		 <2>	 8/10/94	DJ		deletepersonification
		 <1>	  8/4/94	BET		first checked in

	To Do:
*/

#ifndef __Server_Personification__
#define __Server_Personification__

#include "ServerState.h"

int Server_GetPersonificationFromWire( ServerState *state, PersonificationSetupRec *OPers, unsigned char flags );
void Server_PutPersonificationOnWire( ServerState *state, PersonificationSetupRec *PSR );
void Server_DeletePersonification( PersonificationSetupRec *OPers );
void Server_SendPersonification(ServerState *state, unsigned char flags);

#endif
