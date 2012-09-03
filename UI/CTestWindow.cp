/*
	File:		CTestWindow.cp

	Contains:	xxx put contents here xxx

	Written by:	Brian Topping

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<13>	 7/25/94	DJ		update to PListen change
		<12>	  7/3/94	DJ		update to PListen change
		<11>	  7/2/94	BET		Add POpen interface changes
		<10>	 6/30/94	BET		Add Timmer test framework.
		 <9>	 6/28/94	BET		Update for latest transport changes.
		 <8>	 6/18/94	BET		Revert Ted's ioPhysNetState.  Add stuff to make it easier to
									dial and support PModem.  Comment out GameTalk stuff.
		 <7>	 6/16/94	BET		Revert last change.
		 <6>	 6/15/94	BET		Change ioPhysNetUp to ioPhysNetState
		 <5>	 6/14/94	BET		Add Poo
		 <4>	 6/10/94	DJ		made config strings of [kConfigStrLength]
		 <3>	  6/9/94	BET		Change TOpen/TListen interfaces
		 <1>	  6/3/94	BET		"Play with windows and stuff"

	To Do:
*/

// ===========================================================================
//	CTestWindow.cp					©1994 Brian Topping All rights reserved.
// ===========================================================================


#include "CTestWindow.h"
#include "CSegaServerApp.h"
#include "ServerCore.h"
#include "LControl.h"
#include "LButton.h"
#include "UDesktop.h"
#include "UReanimator.h"
#include "TransportLayer.h"
#include "PhysicalLayer.h"
#include "NetMisc.h"
#include "LStream.h"
#include "TimmerScript.h"
#include <Timer.h>
#include <stdio.h>

// Globals
CTestWindow	*gTopTestWindow;

CTestWindow::CTestWindow()
{
}

CTestWindow::CTestWindow(LStream *inStream)
	:LWindow(inStream)
{
	fNextWindow = gTopTestWindow;
	gTopTestWindow = this;
	fUseGameTalk = fConnected = false;
	fTextH = NewHandle(0);
}

CTestWindow::~CTestWindow()
{
	gTopTestWindow = fNextWindow;
	killConnection();
}

CTestWindow*
CTestWindow::CreateWindowStream(
	LStream	*inStream)
{
	return (new CTestWindow(inStream));
}


void
CTestWindow::SpendTime(const EventRecord &inMacEvent)
{
#ifdef HASVIEW
unsigned long	dataReady;
Ptr				hold;
OSErr			theErr;

	NetIdle(nil);
	if (fUseGameTalk == false) {
		dataReady = TDataReady();
		if (fConnected && dataReady) {
			hold = NewPtr(dataReady);
			theErr = TReadData(dataReady, (Ptr)hold, false);
			Munger(fTextH,GetHandleSize(fTextH),nil,0,hold, dataReady);
			fLogView->SetTextHandle(fTextH);
			DisposePtr(hold);
			}
		}
#else
	NetIdle(nil);
#endif
}

void
CTestWindow::ListenToMessage(MessageT aMessage, void *aParam)
{
Str255	theStr;

	switch (aMessage) {
		case kStartBashCmd:
			doLinkTest(true);
			break;
		case kRecieveBashCmd:
			doLinkTest(false);
			break;
		case kTestSendText:
			fSendTextEdit->GetDescriptor(theStr);
			TWriteDataSync(&fTSocket,theStr[0], (Ptr)&theStr[1]);
			break;
		default:
			break;
		}
}

void
CTestWindow::FinishCreateSelf()
{
	fSendTextEdit = (LEditField *)FindPaneByID(kTestEditTextFieldPaneID);
	fLogView = (CTextEdit *)FindPaneByID(kTestLogViewPaneID);
	fLogView->SetTextPtr(" ",1);
	UReanimator::LinkListenerToControls(this, this, RidL_TestControlButtons);
}

void
CTestWindow::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	Char16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand) {
		case kTestDefWait:
		case kTestDefOpen:
		case kTestWait:
		case kTestOpen:
		case kExecTimmerScript:
			outEnabled = (fConnected ? false : true);
			break;
		case kSendFileMessageCmd:
		case kTestCloseConnectCmd:
			outEnabled = (fConnected ? true : false);
			break;
		case kUseGameTalkCmd:
			outEnabled = (fConnected ? false : true);
			outUsesMark = true;
			outMark = (fUseGameTalk ? checkMark : noMark);
			break;
		default:
			LWindow::FindCommandStatus(inCommand, outEnabled,
													outUsesMark, outMark, outName);
			break;
	}
}

Boolean
CTestWindow::ObeyCommand(
	CommandT	inCommand,
	void		*ioParam)
{
	Boolean			cmdHandled = true;

	switch (inCommand) {
		case kTestDefWait:
			waitForConnection(true);
			break;
		case kTestDefOpen:
			establishConnection(true);
			break;
		case kTestWait:
			waitForConnection(false);
			break;
		case kTestOpen:
			establishConnection(false);
			break;
		case kTestCloseConnectCmd:
			killConnection();
			break;
		case kSendFileMessageCmd:
			doTransportDownload();
			break;
		case kUseGameTalkCmd:
			fUseGameTalk = (fUseGameTalk ? false : true);
			break;
		case kExecTimmerScript:
			doTimmerScript();
			break;
		default:
			cmdHandled = LWindow::ObeyCommand(inCommand, ioParam);
			break;
	}

	return cmdHandled;
}

void
CTestWindow::waitForConnection(Boolean useDefaultConn)
{
char configStr[kConfigStrLength], *cp;
NetParamBlock	npb;

	if (useDefaultConn) {
		cp = configStr;
		CServApp::GetConnInfo(false, configStr);
		}
	else
		cp = nil;

	if (PListen(cp, kUseServerProtocol) != noErr)
		return;
		
	while ((npb.ioPhysNetState != kConnOpen) && (!Button()))
		PNetIdle(&npb);
	
	if (TListen(&fTSocket, 0, 0, 0) == noErr) {
		fConnected = true;
		StartRepeating();
		}
	else
		fConnected = false;
}

Boolean
CTestWindow::establishConnection(Boolean useDefaultConn)
{
char configStr[kConfigStrLength], *cp;
NetParamBlock	npb;

	if (useDefaultConn) {
		cp = configStr;
		CServApp::GetConnInfo(false, configStr);
		}
	else
//		cp = "7771434";
		cp = nil;

	if (POpen(cp, kUseServerProtocol | kDisableCallWaiting) != noErr) {
		printf("false return from POpen");
		return false;
		}
		
	while ((npb.ioPhysNetState != kConnOpen) && (!Button()))
		PNetIdle(&npb);
	
	if (TOpen(&fTSocket, 0, 0) == noErr) {
		fConnected = true;
		StartRepeating();
		}
	else
		fConnected = false;

	return fConnected;
}

Boolean
CTestWindow::killConnection(void)
{
Boolean retVal = false;

	if (fConnected) {
		retVal = TClose(&fTSocket);
		StopRepeating();
		fConnected = false;
		}
	PClose();
	return retVal;
}

void
CTestWindow::doTransportDownload(void)
{
StandardFileReply	macFileReply;
SFTypeList			typeList;
	
	UDesktop::Deactivate();
	typeList[0] = 'TEXT';
	typeList[1] = 'SMsg';
	::StandardGetFile(nil, 2, typeList, &macFileReply);
	UDesktop::Activate();
	if (macFileReply.sfGood) {
		short fRefNum;
		unsigned long size;
	
		if (FSpOpenDF(&macFileReply.sfFile, fsRdPerm, &fRefNum)) {
			FSClose(fRefNum);
			SysBeep(5);
			return;
			}
		GetEOF(fRefNum,(long *)&size);
		Ptr buffer = NewPtr(size);
		if (!buffer) {
			FSClose(fRefNum);
			SysBeep(5);
			return;
			}
		if (FSRead(fRefNum, (long *)&size, buffer)) {
			DisposePtr(buffer);
			FSClose(fRefNum);
			SysBeep(5);
			return;
			}
		if (TWriteData(size, buffer, false))
			SysBeep(5);
		DisposePtr(buffer);
		FSClose(fRefNum);
		}
}

void
CTestWindow::doTimmerScript(void)
{
StandardFileReply	macFileReply;
SFTypeList			typeList;
	
	UDesktop::Deactivate();
	typeList[0] = 'TEXT';
	::StandardGetFile(nil, 1, typeList, &macFileReply);
	UDesktop::Activate();
	if (macFileReply.sfGood) {
		DoScript(&macFileReply);
		}
}

void CTestWindow::AddText(Ptr inTextP, Int32 inTextLen)
{
	fLogView->AddText(inTextP, inTextLen);
}

void
CTestWindow::doLinkTest(Boolean isServer)
{
	doSTLinkTest(isServer);
}

void
CTestWindow::doSTLinkTest(Boolean isServer)
{
#define	kTestSize 4096
#define kNumLoops 64
#define DATATYPE unsigned short
#define DATASIZE sizeof(DATATYPE)
DATATYPE		count, count2, c2, size;
UnsignedWide	microTickCount;
DATATYPE		*crap;
long			startTime, seconds, bytes;

	crap = (DATATYPE *)NewPtr(kTestSize*DATASIZE);
	if (!crap) {
		SysBeep(5);
		return;
		}
		
	if (isServer) {
		for (count = 0; count < kTestSize; count++) 
			crap[count] = count;
		}
	
	bytes = 0;
	startTime = LMGetTicks() - 60;
	for (count = 0; count <kNumLoops; count++) {
		if (isServer) {
			Microseconds(&microTickCount);
			size = ((microTickCount.lo & 0xffff) * kTestSize) / 0xffff;
			c2 = (kTestSize-1) - size;
			crap[c2] = size;
			TWriteData((crap[c2]*DATASIZE) + DATASIZE, (Ptr)&crap[c2], false);
			bytes += (crap[c2]+1)*DATASIZE;
			crap[c2] = c2;
			}
		else {
			TReadData(DATASIZE, (Ptr)&crap[0], false);
			TReadData(crap[0]*DATASIZE, (Ptr)&crap[1], false);
			
			c2 = (kTestSize-1);
			for (count2=crap[0]; count2>0; count2--) {
				if (crap[count2] != c2--)
					DebugStr("\pBad Byte Detected!!");
				}
			bytes += (crap[0]+1)*DATASIZE;
			}
			
		seconds = (LM