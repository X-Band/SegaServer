/*
	File:		DataBase_System.c

	Contains:	xxx put contents here xxx

	Written by:	David Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <6>	 8/12/94	ATM		Converted to Logmsg.
		 <5>	 7/17/94	BET		Make printf's go to gLogFile instead of stdout, which is the
									modem.
		 <4>	 6/30/94	DJ		patchlist is now *pointers* to PreformedMessages
		 <3>	  6/1/94	BET		re-add printf
		 <2>	  6/1/94	BET		printf
		 <1>	 5/31/94	DJ		Handles system versions + patches

	To Do:
*/



#include <stdlib.h>

#include "ServerDataBase_priv.h"
#include "ServerCore.h"
#include "Errors.h"


SDBSystem *SDBSystem_New(void)
{
SDBSystem *system;
	
	system = (SDBSystem *)malloc(sizeof(SDBSystem));
	ASSERT(system);
	if(!system)
	{
		ERROR_MESG("out of mems");
		return(NULL);
	}

	system->list = NewSortedList();

	return(system);
}

Err SDBSystem_Save(FILE *fp)
{
SDBSystem		*sys;
SDBCode			code;
long			len;
long			count, i;
SDBSystemNode	*sn;
ListNode		*node;

	sys = SDB_GetSystem(gSDB);
	
	// write a header.
	code = kSDBCode_System;
	len = fwrite(&code, 1, sizeof(SDBCode), fp);
	ASSERT(len == sizeof(SDBCode));

	// write the system patches.
	count = NumListNodesInList(sys->list);
	
	len = fwrite(&count, 1, sizeof(long), fp);
	ASSERT(len == sizeof(long));

printf("SDBSystem_Save: writing %ld system patches\n", count);

	for(node = GetFirstListNode(sys->list); node ; node = GetNextListNode(node), count--)
	{
		sn = (SDBSystemNode *)GetListNodeData(node);
		ASSERT(sn);

		len = fwrite(&sn->version, 1, sizeof(long), fp);
		ASSERT(len == sizeof(long));
		len = fwrite(&sn->keyframe, 1, sizeof(Boolean), fp);
		ASSERT(len == sizeof(Boolean));
		len = fwrite(&sn->numPatches, 1, sizeof(long), fp);
		ASSERT(len == sizeof(long));
		
		for(i = 0; i < sn->numPatches; i++)
		{
			len = fwrite(&sn->patches[i]->length, 1, sizeof(long), fp);
			ASSERT(len == sizeof(long));
			len = fwrite(sn->patches[i]->message, 1, sn->patches[i]->length, fp);
			ASSERT(len == sn->patches[i]->length);
		}
	}
	
	ASSERT_MESG((node == NULL) && (count == 0), "Impl Error");

	return(kNoError);
}

Err SDBSystem_Restore(FILE *fp)
{
SDBSystem 		*sys;
SDBCode			code;
long			len;
long			count, i;
SDBSystemNode	*sn;
ListNode		*ln;

	sys = SDB_GetSystem(gSDB);

	// read the header.
	//
	len = fread(&code, 1, sizeof(SDBCode), fp);
	ASSERT(len == sizeof(SDBCode));
	if(len != sizeof(SDBCode))
		return(kUnexpectedEOF);
		
	ASSERT(code == kSDBCode_System);
	if(code != kSDBCode_System)
		return(kUnexpectedCode);

	// read the system patches.
	//
	
	len = fread(&count, 1, sizeof(long), fp);
	ASSERT(len == sizeof(long));
	if(len != sizeof(long))
		return(kUnexpectedEOF);


printf("SDBSystem_Restore: reading %ld system patches\n", count);

	for(; count--;){
		sn = (SDBSystemNode *)malloc(sizeof(SDBSystemNode));
		ASSERT_MESG(sn, "Out of mems");
		
		len = fread(&sn->version, 1, sizeof(long), fp);
		ASSERT(len == sizeof(long));
		len = fread(&sn->keyframe, 1, sizeof(Boolean), fp);
		ASSERT(len == sizeof(Boolean));
		len = fread(&sn->numPatches, 1, sizeof(long), fp);
		ASSERT(len == sizeof(long));
		
		sn->patches = (PreformedMessage **)malloc(sn->numPatches * sizeof(PreformedMessage *));
		ASSERT_MESG(sn->patches, "Out of mems");
		
		for(i = 0; i < sn->numPatches; i++)
		{
			sn->patches[i] = (PreformedMessage *)malloc(sizeof(PreformedMessage));
			ASSERT(sn->patches[i]);
	
			len = fread(&sn->patches[i]->length, 1, sizeof(long), fp);
			ASSERT(len == sizeof(long));
			
			sn->patches[i]->message = (Ptr)malloc(sn->patches[i]->length);
			ASSERT_MESG(sn->patches[i]->message, "Out of mems");
			
			len = fread(sn->patches[i]->message, 1, sn->patches[i]->length, fp);
			ASSERT(len == sn->patches[i]->length);
		}

		//
		// checks if version is already loaded cuz you can't have two of the same version.
		//
		if(SearchList(sys->list, sn->version))
		{
			ERROR_MESG("Tried to add an existing system version");

			for(i = 0; i < sn->numPatches; i++)
				free(sn->patches[i]->message);
			free(sn);
		} else 
		{
			ln = NewSortedListNode((Ptr)sn, sn->version);
			AddListNodeToSortedList(sys->list, ln);
		}
	}

	return(kNoError);
}

Err SDBSystem_Print(void)
{
SDBSystem		*sys;
long			count, i, a;
SDBSystemNode	*sn;
ListNode		*node;

	sys = SDB_GetSystem(gSDB);
	
	count = NumListNodesInList(sys->list);
printf("SDBSystem_Print: there are %ld system patches\n", count);

	for(a = 0, node = GetFirstListNode(sys->list); node; node = GetNextListNode(node), a++, count--)
	{
		sn = (SDBSystemNode *)GetListNodeData(node);
		ASSERT(sn);

		Logmsg("Patch #%ld:\n", a);
		
		Logmsg("	version = %ld\n", sn->version);
		Logmsg("	keyframe = %s\n", sn->keyframe ? "true" : "false");
		Logmsg("	num patches = %ld\n", sn->numPatches);
		
		for(i = 0; i < sn->numPatches; i++)
		{
			Logmsg("	patch #%ld: length = %ld\n", i, sn->patches[i]->length);
		}
	}

	ASSERT_MESG((node == NULL) && (count == 0), "Impl Error");

	return(kNoError);
}


