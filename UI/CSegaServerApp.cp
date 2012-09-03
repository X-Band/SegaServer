/*
	File:		CSegaServerApp.cp

	Contains:	xxx put contents here xxx

	Written by:	Brian Topping

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <7>	 6/30/94	BET		Add Timmer test framework.
		 <6>	 6/10/94	DJ		changed GetConnInfo to use configStr[1000]
		 <4>	  6/3/94	BET		"Play with windows and stuff"
		 <3>	 5/31/94	BET		Add CTextEdit Stuff
		 <2>	 5/26/94	BET		Remove refs to obsoleted GameTalk.h
		 <2>	 5/23/94	BET		Update CWindow references to CServerWindow
		 <1>	 5/22/94	BET		first checked in

	To Do:
*/

// ===========================================================================
//	CServApp.cp					©1994 Brian Topping All rights reserved.
// ===========================================================================

#include "CSegaServerApp.h"
#include "CTestWindow.h"
#include "CServerWindow.h"
#include "CTestWindow.h"
#include "CView.h"
#include "TransportLayer.h"

#include "Errors.h"

#include "LFile.h"
#include "LTextEdit.h"
#include "LPrintout.h"
#include "LPlaceHolder.h"
#include "LGrowZone.h"
#include "UPowerTools.h"
#include "UMemoryMgr.h"

#include "UScreenPort.h"
#include "URegistrar.h"
#include "UReanimator.h"
#include "UPrintingMgr.h"
#include "UDesktop.h"
#include "UQuickTime.h"
#include "PP_Messages.h"
#include "PPobClasses.h"

#include <CommResources.h>
#include <CTBUtilities.h>
#include <string.h>

#ifndef __STANDARDFILE__
#include <StandardFile.h>
#endif


// ===========================================================================
//		• Main Program
// ===========================================================================

void main(void)
{
	InitializeHeap(3);
	InitializeToolbox();
	new LGrowZone(20000);
	
	CServApp	theApp;
	theApp.Run();
}


// ---------------------------------------------------------------------------
//		• CServApp
// ---------------------------------------------------------------------------
//	Default constructor

CServApp::CServApp(void)
{
	TInit();
	
	UScreenPort::Initialize();
	mDragAndDropPresent = LDragAndDrop::DragAndDropIsPresent();
	fOpenDocuments = 0;
	
	RegisterAllPPClasses();
	URegistrar::RegisterClass('cWnd', (ClassCreatorFunc) CServerWindow::CreateWindowStream);
	URegistrar::RegisterClass('cTWn', (ClassCreatorFunc) CTestWindow::CreateWindowStream);
	URegistrar::RegisterClass('cVew', (ClassCreatorFunc) CView::CreateViewStream);
	URegistrar::RegisterClass('cTxt', (ClassCreatorFunc) CTextEdit::CreateTextEditStream);
	
}



// ---------------------------------------------------------------------------
//		• ~CServApp
// ---------------------------------------------------------------------------
//	Destructor
//

CServApp::~CServApp()
{
}

LModelObject*
CServApp::MakeNewServerDocument()
{
CServerWindow *aWindow;

	aWindow = (CServerWindow *)CServerWindow::CreateWindow(Wind_DocView, this);
	if (aWindow)
		fOpenDocuments += 1;
	return (LModelObject*)aWindow;
}


LModelObject*
CServApp::MakeNewTestDocument()
{
CTestWindow *aWindow;

	aWindow = (CTestWindow *)CTestWindow::CreateWindow(Wind_TestView, this);
	if (aWindow)
		fOpenDocuments += 1;
	return (LModelObject*)aWindow;
}


void
CServApp::ChooseDocument()
{
	LFile				*theFile = nil;
	StandardFileReply	macFileReply;
	SFTypeList			typeList;
	
	UDesktop::Deactivate();
	typeList[0] = kDocCreator;
	::StandardGetFile(nil, 1, typeList, &macFileReply);
	UDesktop::Activate();
	if (macFileReply.sfGood) {
		OpenDocument(&macFileReply.sfFile);
	}
}

void
CServApp::OpenDocument(
	FSSpec	*inMacFSSpec)
{
	LFile	*theFile = new LFile(*inMacFSSpec);
	BuildDocument(theFile);
}


void
CServApp::BuildDocument(
	LFile	*theFile)
{
	CServerWindow	*theWindow = (CServerWindow*) CServerWindow::CreateWindow(Wind_DocView, this);
	
	FSSpec	theFileSpec;
	theFile->GetSpecifier(theFileSpec);
	theWindow->SetDescriptor(theFileSpec.name);
	
	theWindow->Show();
}

Boolean
CServApp::ObeyCommand(
	CommandT	inCommand,
	void		*ioParam)
{
	Boolean		cmdHandled = true;

	switch (inCommand) {
		case kNewServerCmd:
			MakeNewServerDocument();
			break;
		case kNewTestingCmd:
			MakeNewTestDocument();
			break;
		case kAppConfigPorts:
			GetConnInfo(true, nil);
			break;
		default:
			cmdHandled = LDocApplication::ObeyCommand(inCommand, ioParam);
			break;
	}

	return cmdHandled;
}


void
CServApp::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	Char16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand) {
		case kNewServerCmd:
		case kNewTestingCmd:
		case kNewScriptCmd:
		case kAppConfigPorts:
			outEnabled = true;
			break;
		default:
			LDocApplication::FindCommandStatus(inCommand, outEnabled,
													outUsesMark, outMark, outName);
			break;
	}
}

short
CServApp::GetToolProcID(void)
{
Str255			toolName;
OSErr			theErr;
short			procID;
	
	procID = CMGetProcID("\pSerial Tool");
	if (procID == -1) {
		if (!(theErr = CRMGetIndToolName(classCM,1,toolName)))
			procID = CMGetProcID(toolName);
		}
	return procID;
}

void
CServApp::GetConnInfo(Boolean doChoose, char *configStr)
{
ConnHandle		connH;
Point			where = {100,100};
char			*fname = (char *)"\pSegaServer Prefs";
OSErr			theErr;
char			*config;
Str255			localConfigStr;
Handle			configHnd;
FSSpec			fileSpec;
short			fRefNum;
CMBufferSizes	sizes;
short			s1len;
long			hSize;

	sizes[cmDataIn] = 0;
	sizes[cmDataOut] = 0;
	sizes[cmCntlIn] = 0;
	sizes[cmCntlOut] = 0;
	sizes[cmAttnIn] = 0;
	sizes[cmAttnOut] = 0;

	connH = CMNew(GetToolProcID(), 0, sizes, 0, 0);
	if (!connH)
		return;
	
	FindFolder(kOnSystemDisk, kPreferencesFolderType, true, &fileSpec.vRefNum, &fileSpec.parID);
	BlockMove(fname, fileSpec.name, fname[0]+1);

	// load the config string from the prefs file.
	//
	if(!doChoose) {
		// just load it from a file
		fRefNum = FSpOpenResFile(&fileSpec, fsRdWrPerm);
		if(fRefNum > -1){	// if no such file, fall through to choose via dialog
			configHnd = GetResource('PREF', 128);
			if (configStr) {
				hSize = GetHandleSize(configHnd);
				ASSERT_MESG(hSize <= kConfigStrLength, "kConfigStrLength is not long enough");
				
				BlockMoveData(*configHnd, configStr, hSize);
				}
			ReleaseResource(configHnd);
			return;
			}
		}
	
	// no prefs file, so pop up the Comm Toolbox dialog to select a service.
	//
	theErr = CMChoose(&connH, where, nil);
	if ((theErr == chooseOKMinor) || (theErr == chooseOKMajor)) {
		configHnd = NewHandle(0);

		CMGetToolName((**connH).procID, localConfigStr);
		p2cstr(localConfigStr);
		s1len = Munger(configHnd, 0, nil, 0, localConfigStr, strlen((char *)localConfigStr)+1);

		config = CMGetConfig(connH);
		strncpy((char *)localConfigStr, config, 255);
		Munger(configHnd, s1len, nil, 0, config, strlen(config)+1);

		FSpDelete(&fileSpec);
		FSpCreateResFile(&fileSpec, 'BET1', 'PREF', 0);
		fRefNum = FSpOpenResFile(&fileSpec, fsRdWrPerm);
		AddResource(configHnd, 'PREF', 128 ,"\p");
		WriteResource(configHnd);

		if (configStr) {
			BlockMoveData(*configHnd, configStr, GetHandleSize(configHnd));
			}

		CloseResFile(fRefNum);

		CMDispose(connH);
		}

	// there should be an error return here.
	// also, what do we do if configStr is NULL?
	//
}



