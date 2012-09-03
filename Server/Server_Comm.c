/*
	File:		Server_Comm.c

	Contains:	xxx put contents here xxx

	Written by:	Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<11>	 9/16/94	ATM		Add a prototype.
		<10>	 9/16/94	ATM		Whoops.
		 <9>	 9/16/94	ATM		Add SERVER_SIMULATE_DELAY stuff.
		 <8>	 8/12/94	ATM		Converted to Logmsg.
		 <7>	  8/2/94	DJ		fixed setransphold bug when in readfromfile
		 <6>	  8/2/94	DJ		added Server_SetTransportHold
		 <5>	 7/27/94	DJ		fixing NULL pBlock
		 <4>	 7/27/94	DJ		made Server_TNetIdle set pBlock to kConnOpen if doing File input
									instead of com layer.
		 <3>	 7/26/94	BET		Add #ifndef unix to Server_PListen.  Our priceless unix
									compilers are not smart enough to dead code strip on link.
		 <2>	 7/25/94	DJ		using async write now
		 <1>	 7/20/94	DJ		first checked in

	To Do:
*/


// define this to read/write from a file instead of from the modem
//#define SERVER_READFROMINPUTFILE	1

// define this to write data to a tee file
#define SERVER_WRITE_TEE	1
#define SERVER_READ_TEE		1

// If we're reading from an input file, we can only write to a file (modem can't run)
#ifdef SERVER_READFROMINPUTFILE
#define SERVER_WRITE_TEE	1
#endif

#ifdef SERVER_READFROMINPUTFILE
// Add delays to simulate a 200 cps modem when reading from a file.  Makes
// the simulated runs more representative of the real thing, by spacing
// the database calls out to appropriate intervals, and keeping the
// server front-end alive for a longer time.
//
// Makes little sense to do this if we're not reading from a file.
//
#define SERVER_SIMULATE_DELAY	1
#ifdef SERVER_SIMULATE_DELAY
# define ONE_MILLION	1000000
# define SERVER_CHAR_DELAY	(ONE_MILLION/200)		// 5000 usec per character
static int writeDelay = 0;	// fake async writes by doing them all at TClose
void Server_SimulateDelay(int nchars);
#endif
#endif /*SERVER_READFROMINPUTFILE*/

#include "Server.h"
#include "Server_Comm.h"
#include "PhysicalLayer.h"

#include <stdio.h>


static FILE *writeTee, *readTee, *inTee;


int Server_OpenDataTee(char *writefilename, char *readfilename, char *infilename)
{
#ifdef SERVER_WRITE_TEE
	writeTee = fopen(writefilename, "wb");
	if(!writeTee)
		return(-1);
#endif

#ifdef SERVER_READ_TEE
	readTee = fopen(readfilename, "wb");
	if(!readTee)
		return(-1);
#endif

#ifdef SERVER_READFROMINPUTFILE
	inTee = fopen(infilename, "rb");
	if(!inTee)
		return(-1);
#endif

	return(0);
}

int Server_CloseDataTee()
{
#ifdef SERVER_WRITE_TEE
	if(writeTee)
		fclose(writeTee);
#endif

#ifdef SERVER_READ_TEE
	if(readTee)
		fclose(readTee);
#endif

#ifdef SERVER_READFROMINPUTFILE
	if(inTee)
		fclose(inTee);
#endif

	return(0);
}


void Server_SetTransportHold(Boolean hold)
{
#ifndef SERVER_READFROMINPUTFILE
	TSetTransportHold(hold);
#endif

}

//
// These are wrappers for TransportLayer TReadDataSync and TWriteDataSync
//

OSErr	Server_TReadDataSync(SessionRec *sess, unsigned long length, Ptr address)
{
OSErr err;

	err = 0;
#ifdef SERVER_READFROMINPUTFILE
	if(fread(address, length, 1, inTee) != 1)
		err = -1;
#else
	err = TReadDataSync(sess, length, address);
#endif

#ifdef SERVER_SIMULATE_DELAY
	Server_SimulateDelay(length);
#endif

#ifdef SERVER_READ_TEE
	if(!err && readTee)
		fwrite(address, length, 1, readTee);
#endif

	return(err);
}

OSErr	Server_TWriteDataSync(SessionRec *sess, unsigned long length, Ptr address)
{
OSErr err;

	err = 0;
#ifndef SERVER_READFROMINPUTFILE
//	err = TWriteDataSync(sess, length, address);
	err = TWriteDataASync(sess, length, address);
#endif

#ifdef SERVER_SIMULATE_DELAY
	writeDelay += length;
#endif

#ifdef SERVER_WRITE_TEE
	if(!err && writeTee)
		fwrite(address, length, 1, writeTee);
#endif
	return(err);
}

OSErr Server_TCheckError(void)
{
OSErr	err;

	err = 0;
#ifndef SERVER_READFROMINPUTFILE
	err = TCheckError();
	if(err != noErr)
		Logmsg("*** Server_TCheckError: err = %ld\n", (long)err);
#endif
	return(err);
}


OSErr	Server_TListen(SessionRec *s, PortT localPort, PortT remPort, unsigned long timeout)
{
OSErr	err;

	err = 0;
#ifndef SERVER_READFROMINPUTFILE
	err = TListen(s, localPort, remPort, timeout);
#endif

	return(err);
}

OSErr	Server_TClose(SessionRec *s)
{
OSErr	err;

	err = 0;
#ifndef SERVER_READFROMINPUTFILE
	err = TClose(s);
#endif

#ifdef SERVER_SIMULATE_DELAY
	Server_SimulateDelay(writeDelay);
	writeDelay = 0;
#endif

	return(err);
}

short 	Server_TNetIdle(NetParamBlock *pBlock)
{
#ifndef SERVER_READFROMINPUTFILE
	return(TNetIdle(pBlock));
#else
	if(pBlock)
		pBlock->ioPhysNetState = kConnOpen;
	return(0);
#endif
}


void	Server_TInit(void)
{
#ifndef SERVER_READFROMINPUTFILE
	TInit();
#endif
}

//
// Physical layer stuff
//

OSErr	Server_PListen(char *config, long flags)
{
OSErr	err;

	err = 0;
#ifndef SERVER_READFROMINPUTFILE
	err = PListen(config, flags);
#endif
	return(err);
}

#ifndef unix
OSErr	Server_PListenAsync(char *config, long flags)
{
OSErr	err;

	err = 0;
#ifndef SERVER_READFROMINPUTFILE
	err = PListenAsync(config, flags);
#endif
	return(err);
}
#endif

OSErr	Server_PClose(void)
{
OSErr	err;

	err = 0;
#ifndef SERVER_READFROMINPUTFILE
	err = PClose();
#endif
	return(err);
}

OSErr	Server_PCheckError(void)
{
OSErr	err;

	err = 0;
#ifndef SERVER_READFROMINPUTFILE
	err = PCheckError();
#endif
	return(err);
}


#ifdef SERVER_SIMULATE_DELAY
void Server_SimulateDelay(int nchars)
{
	unsigned long usec;

	//Logmsg("SLEEP %d chars", nchars);

	usec = nchars * SERVER_CHAR_DELAY;		// overflow at 4294 seconds
	if (usec > ONE_MILLION) {
		//Logmsg(" (%ld sec)", usec / ONE_MILLION);
		sleep(usec / ONE_MILLION);
		usec = usec % ONE_MILLION;
	}
	//Logmsg(" (%ld usec)", usec);
	if (usec)
		usleep(usec);
	//Logmsg("\n");
}
#endif

