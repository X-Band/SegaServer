/*
	File:		CServerWindow.h

	Contains:	xxx put contents here xxx

	Written by:	Brian Topping

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <8>	 6/14/94	BET		Add Poo
		 <7>	  6/5/94	BET		Make it multisession
		 <5>	  6/3/94	BET		"Play with windows and stuff"
		 <4>	  6/1/94	BET		Add stuff for proto basher
		 <3>	 5/31/94	BET		Add FUD
		 <2>	 5/26/94	BET		Remove refs to obsoleted GameTalk.h
		 <1>	 5/23/94	BET		first checked in
	To Do:
*/

// ===========================================================================
//	CServerWindow.h					©1994 Brian Topping All rights reserved.
// ===========================================================================

 
#pragma once

#include "LWindow.h"
#include "LPeriodical.h"
#include "LListener.h"
#include "LCaption.h"
#include "CTextEdit.h"
#include "LEditField.h"
#include "TransportLayer.h"
#include "ServerCore.h"

#define RidL_ControlButtons	200

#define Wind_DocView		200
#define kScrollerWidth		313
#define kScrollerHeight		148
#define kUpdateInterval		10

enum {
	kLogViewScrlID			= 270,
	kLogViewPaneID
	};

extern class CServerWindow *gTopServerWindow;

class CServerWindow : public LWindow, public LListener, public LPeriodical {
public:
					CServerWindow();
					CServerWindow(LStream *inStream);
/*					CServerWindow(SWindowInfo &inWindowInfo);
					CServerWindow(ResIDT inWINDid, Uint32 inAttributes,
								LCommander *inSuperCommander);
					*/
	virtual			~CServerWindow();
	void			resetStats(void);
	void			AddText(Ptr inTextP, Int32 inTextLen);
	static CServerWindow *GetTopWindow(void) { return gTopServerWindow; }
	CTextEdit		*GetWindowText(void) { return fLogView; }
	
	
							/* pp-ism's */
	virtual void	FinishCreateSelf();
	static CServerWindow*	CreateWindowStream(LStream	*inStream);
	void			SpendTime(const EventRecord &inMacEvent);
	void			ListenToMessage(MessageT inMessage, void *ioParam);
	void			setLoggingState(Boolean doLog);
	Boolean			ObeyCommand(CommandT inCommand, void *ioParam = nil);
	void			FindCommandStatus(CommandT inCommand,
							Boolean &outEnabled, Boolean &outUsesMark,
							Char16 &outMark, Str255 outName);

protected:
	void			waitForConnection(Boolean useDefaultConn);
	void			establishConnection(Boolean useDefaultConn);
	Boolean			killConnection(void);
	
	SessionRec		fTSocket;
	CTextEdit		*fLogView;
	Boolean			fConnected;
	Handle			fTextH;
	CServerWindow	*fNextWindow;
	ServerState		*fBoxState;
};
