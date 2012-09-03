/*
	File:		DataBase_NGP.c

	Contains:	xxx put contents here xxx

	Written by:	David Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <1>	 5/31/94	DJ		first checked in

	To Do:
*/



#include <stdlib.h>

#include "ServerDataBase_priv.h"
#include "Errors.h"

SDBNGP *SDBNGP_New(void)
{
SDBNGP *ngp;
	
	ngp = (SDBNGP *)malloc(sizeof(SDBNGP));
	ASSERT(ngp);
	if(!ngp)
	{
		ERROR_MESG("out of mems");
		return(NULL);
	}

	ngp->ngp = NULL;

	return(ngp);
}
