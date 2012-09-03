
/*
	File:		Server_SendLoopback.c

	Contains:	Drive a loopback to the box for network testing.

	Written by:	Dave Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<19>	  9/2/94	DJ		read from loop.data if such file exists
		<18>	  9/2/94	DJ		send control-p and control-x to see if that kills it
		<17>	  9/2/94	DJ		send a test pattern (0 - FF) instead of zeros
		<16>	 8/22/94	DJ		groomed dialog text
		<15>	 8/12/94	ATM		Converted to Logmsg.
		<14>	  8/8/94	DJ		receive timing results from sega, bounce them back in a dialog
		<13>	  8/8/94	DJ		make box send timing numbers
		<12>	  8/4/94	DJ		uncommented transportholds
		<11>	  8/3/94	DJ		made it a throughput tester instead of cheesy loopback
		<10>	 7/20/94	DJ		added Server_Comm stuff
		 <9>	 7/17/94	BET		Make printf's go to gLogFile instead of stdout, which is the
									modem.
		 <8>	 7/12/94	DJ		using Server_TCheckError instead of TCheckError
		 <7>	  7/1/94	DJ		making server handle errors from the comm layer
		 <6>	  6/9/94	BET		Fix hairy compiles based on TransportSlayer.h
		 <5>	  6/5/94	DJ		making everything take a ServerState instead of SessionRec
		 <4>	 5/29/94	DJ		sync writing instead of async
		 <3>	 5/26/94	DJ		forgot to send numLoops
		 <2>	 5/26/94	DJ		bugs
		 <2>	 5/26/94	DJ		bugs
		 <1>	 5/26/94	DJ		first checked in
	To Do:
*/

#include "ServerCore.h"
#include "Server.h"
#include "Messages.h"
#include "UsrConfg.h"
#include "Server_Comm.h"
#include <stdio.h>

#include "PreformedMessage.h"


int Server_SendLoopback(ServerState *state, long dataSize, Boolean sticky)
{
long i, dummy, j;
messOut	opCode;
unsigned char	dataChunk[1000];
long	looptime1, looptime2;
char	blob[1000];
unsigned char 	d;
PreformedMessage	*m;

	Logmsg("Server_SendLoopback\n");
	// don't call Server_DebugService() or you will go recursive!

	opCode = msLoopBack;
	Server_TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);
	Server_TWriteDataSync(state->session, sizeof(long), (Ptr)&dataSize);

	// wait for box to get into msg handler and be ready to receive our treats.

	Server_TReadDataSync( state->session, sizeof(long), (Ptr)&dummy );

	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	// this kills it.
	
	for(i = 0, d = 0; i < 1000; i++, d++)
		dataChunk[i] = d;

/* control-p and control-x.... doesn't seem to kill it.

	for(i = 0; i < 500; i++)
		dataChunk[i] = 0x10;
	for(; i < 1000; i++)
		dataChunk[i] = 0x18;
*/

	m = PreformedMessage_ReadFromFile("loop.data");
	if(m)
	{
		for(i = 0, d = 0; i < 1000; i++)
		{
			dataChunk[i] = (unsigned char)m->message[d++];
			if(d == m->length)
				d = 0;
		}
		PreformedMessage_Dispose(m);
	}
	
	Server_SetTransportHold(true);

	for(i = 0, j = 0; i < dataSize; i+=100)
	{
		if(j+100 > 1000)
			j = 0;
		Server_TWriteDataSync(state->session, 100, (Ptr)&dataChunk[j]);
		j+= 100;
	}

	for(i = 0; i < dataSize; i+=1000)
		Server_TWriteDataSync(state->session, 1000, (Ptr)dataChunk);

 Server_SetTransportHold(false);


	if(Server_TCheckError() != noErr)
		return(kServerFuncAbort);

	if(Server_TReadDataSync( state->session, sizeof(long), (Ptr)&looptime1 ) != noErr)
		return(kServerFuncAbort);
	if(Server_TReadDataSync( state->session, sizeof(long), (Ptr)&looptime2 ) != noErr)
		return(kServerFuncAbort);

	looptime1 /= 60;	// get seconds
	looptime2 /= 60;	
	sprintf(blob, "(DEBUG) Server_SendLoopback: 100b packets %ld cps, 1Kb packets %ld cps\n",
						10000/looptime1, 10000/looptime2);

	Logmsg(blob);
	Server_SendDialog(state, blob, sticky);

	return(kServerFuncOK);
}
