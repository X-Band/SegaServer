/*
	File:		CProtoTestApp.h

	Contains:	xxx put contents here xxx

	Written by:	Brian Topping

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <4>	 6/10/94	DJ		GetConnInfo takes a char* instead of Char255
		 <2>	  6/3/94	BET		"Play with windows and stuff"
		 <1>	 5/22/94	BET		first checked in

	To Do:
*/

// ===========================================================================
//	CServApp.h					©1994 Brian Topping, All rights reserved.
// ===========================================================================

#pragma once

#include "LApplication.h"
#include "LListener.h"
#include "ResIDs.h"
#include <MixedMode.h>

#define kDocCreator	'BET1'
#define kDocType	'ssia'

// =================
// Error ID's
// =================

typedef OSErr BrianAppErr;
enum {
	kDocumentOpenErr = 1000
	};

#ifndef DEBUG_Level
#define DEBUG_Level DEBUG_On
#endif

class	LFile;

class	CServApp : public LDocApplication {

public:
						CServApp(void);
	virtual 			~CServApp(void);
	
						/* pp-isms */
	LModelObject*		MakeNewServerDocument(void);
	LModelObject*		MakeNewTestDocument(void);
	virtual void		ChooseDocument();
	void				OpenDocument(FSSpec *inFileSpec);
	void				BuildDocument(LFile *inFile);							
	Boolean				ObeyCommand(CommandT inCommand, void *ioParam = nil);
	void				FindCommandStatus(CommandT inCommand,
							Boolean &outEnabled, Boolean &outUsesMark,
							Char16 &outMark, Str255 outName);

						/* app-isms */
	static void			GetConnInfo(Boolean doChoose, char *configStr);
	static short		GetToolProcID(void);

protected:
	Boolean	mDragAndDropPresent;
	short	fOpenDocuments;
};

