/*
	File:		DataBase_SystemPatches.c

	Contains:	xxx put contents here xxx

	Written by:	David Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <7>	 8/30/94	ATM		Got sick of WARNING_MESGs, changed them to Logmsgs.
		 <6>	 8/11/94	ATM		Converted to Logmsg().
		 <5>	 7/17/94	BET		Make printf's go to gLogFile instead of stdout, which is the
									modem.
		 <4>	 6/30/94	DJ		DataBase_AddSystemPatch and LoadSystemPatches
		 <3>	 5/31/94	DJ		added all the real version handling etc
		 <2>	 5/29/94	DJ		added dispose method
		 <1>	 5/27/94	DJ		first checked in

	To Do:
*/

#include "Server.h"	// this is only for PreformedMessage type
#include "ServerDataBase.h"
#include "ServerDataBase_priv.h"

#include <stdio.h>
#include <stdlib.h>


//
// Yikes.  must be careful who calls this.. because the DB for now
// is not allocating the messages... it is returning ptrs to the actual
// message in the DB.	5/30/94
//
void DataBase_ReleasePreformedMessage(PreformedMessage *msg)
{
// So we won't free anything.
//
/*
	if(msg->message){
		free(msg->message);
		msg->message = NULL;
	}
	free(msg);
*/
}


long DataBase_GetLatestSystemVersion(void)
{
SDBSystem		*sys;
ListNode		*node;
SDBSystemNode	*sn;

	sys = SDB_GetSystem(gSDB);

	node = GetLastListNode(sys->list);
	if(node)
	{
		sn = (SDBSystemNode *)GetListNodeData(node);
		return(sn->version);
	}

	//WARNING_MESG("No loaded system version?");
	Logmsg("No loaded system version?\n");
	return(1);
}

long DataBase_GetLatestSystemKeyframeVersion(void)
{
SDBSystem		*sys;
ListNode		*node;
SDBSystemNode	*sn;

	sys = SDB_GetSystem(gSDB);

	// find the latest system version.
	// if it's not a keyframe, work backwards until we find one.
	//
	node = GetLastListNode(sys->list);
	while(node)
	{
		sn = (SDBSystemNode *)GetListNodeData(node);
		if(sn->keyframe == true)
			return(sn->version);
		node = GetPrevListNode(node);
	}

	//WARNING_MESG("No keyframe system version?");
	Logmsg("No keyframe system version?\n");
	return(1);
}

long DataBase_GetSystemNumPatches(long version)
{
SDBSystem		*sys;
ListNode		*node;
SDBSystemNode	*sn;

	sys = SDB_GetSystem(gSDB);

	node = SearchList(sys->list, version);
	if(node)
	{
		sn = (SDBSystemNode *)GetListNodeData(node);
		return(sn->numPatches);
	}

	//WARNING_MESG("No such system version?!?");
	Logmsg("No such system version?!?\n");

	return(0);
}

PreformedMessage *DataBase_GetSystemPatch(long version, long patchNum)
{
SDBSystem		*sys;
ListNode		*node;
SDBSystemNode	*sn;

	sys = SDB_GetSystem(gSDB);
	ASSERT(sys);

	ASSERT_MESG(patchNum >= 0, "Negative patch numbers are illegal");

	node = SearchList(sys->list, version);
	if(node)
	{
		sn = (SDBSystemNode *)GetListNodeData(node);
		ASSERT_MESG(patchNum < sn->numPatches, "Requested system patch number is out of range");
		if(patchNum >= sn->numPatches)
			return(NULL);	// big error
		
		return(sn->patches[patchNum]);
	}

	//WARNING_MESG("No such system version?!?");
	Logmsg("No such system version?!?\n");

	return(NULL);
}


//
// Appropriates the patch... does not copy it when it adds to the DB.
//
Err DataBase_AddSystemPatch(long version, long patchNum, PreformedMessage *patch)
{
SDBSystem		*sys;
ListNode		*node;
SDBSystemNode	*sn;

	sys = SDB_GetSystem(gSDB);
	ASSERT(sys);

	ASSERT_MESG(patchNum >= 0, "Negative patch numbers are illegal");

	node = SearchList(sys->list, version);
	if(node)
	{
		sn = (SDBSystemNode *)GetListNodeData(node);
		ASSERT(sn);
		ASSERT_MESG(patchNum == sn->numPatches, "Patches must be added in linear order... what's up!?");
		if(patchNum != sn->numPatches)
			return(kAddingSystemPatchIsFucked);	// big error
		
		sn->numPatches++;
		sn->patches = (PreformedMessage **)realloc(sn->patches, sn->numPatches * sizeof(PreformedMessage *));
		ASSERT(sn->patches);

		sn->patches[patchNum] = patch;
		return(kNoError);
	}

	ASSERT(patchNum == 0);

	// malloc a node
	// set its numpatches to 1
	// copy in this patch.

	sn = (SDBSystemNode *)malloc(sizeof(SDBSystemNode));
	ASSERT(sn);
	
	sn->numPatches = 1;
	sn->version = version;
	sn->keyframe = 1;				// as a hack for now, every system release is a keyframe. 6/29/94  dj

	sn->patches = (PreformedMessage **)malloc(sizeof(PreformedMessage *));
	ASSERT(sn->patches);

	sn->patches[0] = patch;

	node = NewSortedListNode((Ptr)sn, sn->version);
	AddListNodeToSortedList(sys->list, node);

	return(kNoError);

}

Err SDBSystem_LoadSystemPatches(void)
{
long		count, version, patchnum;
char		str[kSDB_SystemPatchFilenameLength];
PreformedMessage	*msg;
char		noversion;

	version = 1;
	patchnum = 0;

	noversion = 1;
	count = 0;
	for(;;)
	{
		sprintf(str, "%s.%ld.%ld", kSDB_SystemPatchFilePrefix, version, patchnum);
		msg = PreformedMessage_ReadFromFile(str);
		if(!msg)
		{
			if(noversion)
				break;
			version++;
			patchnum = 0;
			noversion = 1;
			continue;
		}

		DataBase_AddSystemPatch(version, patchnum, msg);	// appropriates the patch... so we don't delete it here.

		patchnum++;
		count++;
		noversion = 0;
	}
	
	Logmsg("Read %ld patches, latest system version is %ld\n", count, version-1);

	return(kNoError);
}
