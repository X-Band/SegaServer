/*
	File:		ServerDataBase_FILE.h

	Contains:	xxx put contents here xxx

	Written by:	David Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <6>	 8/16/94	ATM		Added kSDB_GamePatchFile.
		 <5>	  8/8/94	ATM		Added kSDB_NewsFile.
		 <4>	  8/5/94	DJ		make work on mac
		 <3>	  8/3/94	ATM		Added kSDB_BMail.
		 <2>	 6/30/94	DJ		system patch file prefix
		 <1>	 5/31/94	DJ		save and restore database to file

	To Do:
*/


#ifndef __ServerDataBase_FILE__
#define __ServerDataBase_FILE__

#define kSDBFILE	"ServerDB.save"

typedef long	SDBCode;

#define kSDBCode_Server		'svdb'
#define kSDBCode_System		1


#define kSDB_BMail		"BroadcastMail"
#define kSDB_NewsFile	"NewsPages"
#define kSDB_GamePatchFile	"Patches.smsgs"


#define kSDB_SystemPatchFilenameLength	1000
#define kSDB_SystemPatchFilePrefix		"SystemPatch"



#endif __ServerDataBase_FILE__
