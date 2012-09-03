/*
	File:		Server_Comm.h

	Contains:	xxx put contents here xxx

	Written by:	Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <3>	  8/2/94	DJ		Server_SetTransportHold
		 <2>	 7/25/94	DJ		added async listen
		 <1>	 7/20/94	DJ		first checked in

	To Do:
*/


#ifndef __ServerComm__
#define __ServerComm__

#include "TransportLayer.h"

//
// These are wrappers for TransportLayer TReadDataSync and TWriteDataSync
//

int		Server_OpenDataTee(char *writeoutname, char *readoutname, char *inputfilename);
int		Server_CloseDataTee(void);

short 	Server_TNetIdle(NetParamBlock *pBlock);
OSErr	Server_TListen(SessionRec *s, PortT localPort, PortT remPort, unsigned long timeout);
OSErr	Server_TClose(SessionRec *s);
void	Server_TInit(void);
OSErr	Server_TReadDataSync(SessionRec *sess, unsigned long length, Ptr address);
OSErr	Server_TWriteDataSync(SessionRec *sess, unsigned long length, Ptr address);
OSErr	Server_TCheckError(void);

OSErr	Server_PListen(char *config, long flags);
OSErr	Server_PListenAsync(char *config, long flags);
OSErr	Server_PClose(void);
OSErr	Server_PCheckError(void);

void 	Server_SetTransportHold(Boolean hold);


#endif
