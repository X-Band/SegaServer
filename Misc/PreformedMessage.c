/*
	File:		PreformedMessage.c

	Contains:	xxx put contents here xxx

	Written by:	Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <5>	 8/30/94	DJ		not printing warnings out
		 <4>	  8/4/94	DJ		make PackStrings a generally available util (used by news and
									rankings)
		 <3>	 7/15/94	dwh		unix-ise
		 <2>	 6/12/94	DJ		changed an error to a warning
		 <1>	 6/11/94	DJ		first checked in

	To Do:
*/


#include <stdio.h>
#include <stdlib.h>

#ifdef	unix
#include "NetTypes.h"
#endif
#include "Errors.h"
#include "PreformedMessage.h"
#include "Utils.h"

#define MESG_CHUNKSIZE 1000

//
// A handy string packer used for news and rankings.
//
void PackStrings(char *packed, long numStrings, char **strings)
{
char *s, *t;
long a;

	ASSERT(packed);
	ASSERT(strings);
	
	s = packed;
	for(a = 0; a < numStrings; a++)
	{
		for(t = strings[a]; *t; )
			*s++ = *t++;
		*s++ = *t;		// copy the NULL.
	}
}

//
// Preformed message utils
//
PreformedMessage *PreformedMessage_Duplicate(PreformedMessage *message)
{
PreformedMessage	*dup;

	ASSERT(message);
	if(!message)
		return(NULL);
	
	if(message->length < 1)
		return(NULL);
	
	ASSERT(message->message);
	if(!message->message)
		return(NULL);
	
	dup = (PreformedMessage *)malloc(sizeof(PreformedMessage));
	ASSERT_MESG(dup, "out of mems");
	
	if(!dup)
		return(NULL);
	
	dup->length = message->length;
	
	dup->message = (Ptr)malloc(dup->length);
	ASSERT_MESG(dup->message, "out of mems");
	if(!dup->message)
	{
		free(dup);
		return(NULL);
	}
	
	ByteCopy(dup->message, message->message, dup->length);
	
	return(dup);
}

void PreformedMessage_Dispose(PreformedMessage *message)
{
	ASSERT(message);
	
	if(!message)
		return;
	
	ASSERT(message->message);
	
	if(message->message)
		free(message->message);
	
	free(message);
}



PreformedMessage *PreformedMessage_ReadFromFile(char *filename)
{
PreformedMessage *message;
FILE *fp;
long allocSize, messageChunkSize;
long numbytes;


	ASSERT(filename);

	fp = fopen(filename, "rb");
	if(!fp)
	{
//		WARNING_MESG("can't read the message file");
		return(NULL);
	}
	
	message = (PreformedMessage *)malloc(sizeof(PreformedMessage));
	if(!message){
		ERROR_MESG("out of mems");
		fclose(fp);
		return(NULL);
	}
	
	messageChunkSize = MESG_CHUNKSIZE;
	allocSize = messageChunkSize;
	message->message = (Ptr)malloc((size_t)allocSize);
	ASSERT(message->message);

	message->length = 0;
	numbytes = fread(&message->message[message->length], 1, messageChunkSize, fp);
	
	for(; numbytes > 0;)
	{
		message->length += numbytes;
		if(numbytes < messageChunkSize)
			break;
		
		allocSize += messageChunkSize;
		message->message = (Ptr)realloc(message->message, (size_t)allocSize);
		ASSERT(message->message);
		
		// here you could adjust messageChunkSize for big messages.
		// double it until it hits 10K or something.
		// start at 100 bytes.
		
		numbytes = fread(&message->message[message->length],1,  messageChunkSize, fp);
	}
	
	
	fclose(fp);
	return(message);
}
