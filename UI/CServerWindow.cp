/*
	File:		CServerWindow.cp

	Contains:	xxx put contents here xxx

	Written by:	Brian Topping

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<13>	 6/14/94	BET		Add Poo
		<12>	 6/10/94	DJ		made config strings of [kConfigStrLength]
		<11>	  6/9/94	BET		Change  TOpen TListen interfaces
		<10>	  6/5/94	BET		Make it multisession
		 <8>	  6/3/94	BET		"Play with windows and stuff"
		 <7>	  6/1/94	BET		Add stuff for proto basher
		 <6>	  6/1/94	BET		More poo
		 <5>	 5/31/94	BET		Add server poo
		 <4>	 5/29/94	BET		I thought this got checked in yesterday.  The fix for the
									"Expected 1, got 10" bug was the window code stealing bytes.
		 <3>	 5/27/94	DJ		added #ifdef JEVANS for using the real server
		 <2>	 5/26/94	BET		Remove refs to obsoleted GameTalk.h
		 <2>	 5/23/94	BET		first checked in

	To Do:
*/

// ===========================================================================
//	CServerWindow.cp					©1994 Brian Topping All rights reserved.
// ===========================================================================


#include "CServerWindow.h"
#include "CSegaServerApp.h"
#include "LControl.h"
#include "LButton.h"
#include "UDesktop.h"
#include "UReanimator.h"
#include "TransportLayer.h"
#include "NetMisc.h"
#include "LStream.h"
#include <Timer.h>
#include <printf.h>
#include <string.h>
#include <Connections.h>

// Globals
CServerWindow	*gTopServerWindow;

CServerWindow::CServerWindow()
{
}

CServerWindow::CServerWindow(LStream *inStream)
	:LWindow(inStream)
{
	fNextWindow = gTopServerWindow;
	gTopServerWindow = this;
	fConnected = false;
	fTextH = NewHandle(0);
	fBoxState = nil;
}

CServerWindow::~CServerWindow()
{
	gTopServerWindow = fNextWindow;
	killConnection();
}

CServerWindow*
CServerWindow::CreateWindowStream(
	LStream	*inStream)
{
	return (new CServerWindow(inStream));
}


void
CServerWindow::SpendTime(const EventRecord &inMacEvent)
{
//	Don't do this here, because there may be no connection yet.
//	NetIdle(nil);
	DoCommand(fBoxState);
}

void
CServerWindow::ListenToMessage(MessageT aMessage, void *aParam)
{
	switch (aMessage) {
		default:
			break;
		}
}

void
CServerWindow::FinishCreateSelf()
{
	fLogView = (CTextEdit *)FindPaneByID(kLogViewPaneID);
	fLogView->SetTextPtr(" ",1);
}

void
CServerWindow::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	Char16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand) {
		case kServerDefWait:
		case kServerDefOpen:
		case kServerWaitForConnectCmd:
		case kServerOpenConnectCmd:
			outEnabled = (fConnected ? false : true);
			break;
		case kServerCloseConnectCmd:
			outEnabled = (fConnected ? true : false);
			break;
		default:
			LWindow::FindCommandStatus(inCommand, outEnabled,
													outUsesMark, outMark, outName);
			break;
	}
}

Boolean
CServerWindow::ObeyCommand(
	CommandT	inCommand,
	void		*ioParam)
{
	Boolean			cmdHandled = true;

	switch (inCommand) {
		case kServerDefWait:
			waitForConnection(true);
			break;
		case kServerDefOpen:
			establishConnection(true);
			break;
		case kServerWaitForConnectCmd:
			waitForConnection(false);
			break;
		case kServerOpenConnectCmd:
			establishConnection(false);
			break;
		case kServerCloseConnectCmd:
			killConnection();
			break;
		default:
			cmdHandled = LWindow::ObeyCommand(inCommand, ioParam);
			break;
	}

	return cmdHandled;
}

void
CServerWindow::waitForConnection(Boolean useDefaultConn)
{
char		configStr[kConfigStrLength];

	if (useDefaultConn){
		CServApp::GetConnInfo(false, configStr);
		fBoxState = InitServer((char *)configStr);
		}
	else
		fBoxState = InitServer(nil);

	if (fBoxState) {
		fConnected = true;
		StartRepeating();
		}
}

void
CServerWindow::establishConnection(Boolean useDefaultConn)
{
char configStr[kConfigStrLength];

	if (useDefaultConn) {
		CServApp::GetConnInfo(false, configStr);
		fBoxState = InitServer((char *)configStr);
		}
	else
		fBoxState = InitServer(nil);

	if (fBoxState) {
		fConnected = true;
		StartRepeating();
		}
}


Boolean
CServerWindow::killConnection(void)
{
Boolean retVal = false;

	if (fConnected) {
		retVal = KillServer(fBoxState);
		StopRepeating();
		fConnected = false;
		}
	return retVal;
}


void CServerWindow::AddText(Ptr inTextP, Int32 inTextLen)
{
	fLogView->AddText(inTextP, inTextLen);
}



