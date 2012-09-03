/*
	File:		PreformedMessage.h

	Contains:	xxx put contents here xxx

	Written by:	Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <2>	  8/4/94	DJ		make PackStrings a generally avail util
		 <1>	 6/11/94	DJ		first checked in

	To Do:
*/



#ifndef __PreformedMessage__
#define __PreformedMessage__

// message is a data chunk that contains opCode and the data.
typedef struct PreformedMessage {
	long 	length;
	Ptr		message;
} PreformedMessage;


PreformedMessage *PreformedMessage_Duplicate(PreformedMessage *message);
void PreformedMessage_Dispose(PreformedMessage *message);

PreformedMessage *PreformedMessage_ReadFromFile(char *filename);

// String packing, used by news and rankings.
//
void PackStrings(char *packed, long numStrings, char **strings);

#endif __PreformedMessage__
