/*
	File:		CTestWindow.h

	Contains:	xxx put contents here xxx

	Written by:	Brian Topping

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <4>	 6/30/94	BET		Add Timmer test framework.
		 <3>	 6/14/94	BET		Add Poo
		 <1>	  6/3/94	BET		"Play with windows and stuff"

	To Do:
*/

// ===========================================================================
//	CTestWindow.h					©1994 Brian Topping All rights reserved.
// ===========================================================================

 
#pragma once

#include "LWindow.h"
#include "LPeriodical.h"
#include "LListener.h"
#include "LCaption.h"
#include "CTextEdit.h"
#include "LEditField.h"
#include "TransportLayer.h"

#define RidL_TestControlButtons	300

#define Wind_TestView		300
#define kScrollerWidth		313
#define kScrollerHeight		148
#define kUpdateInterval		10

enum {
	kTestSendTextButtonPaneID	= 350,
	kTestLogViewScrlID			= 370,
	kTestLogViewPaneID,
	kTestEditTextFieldPaneID	= 380
	};

extern class CTestWindow *gTopTestWindow;

class CTestWindow : public LWindow, public LListener, public LPeriodical {
public:
					CTestWindow();
					CTestWindow(LStream *inStream);
/*					CTestWindow(SWindowInfo &inWindowInfo);
					CTestWindow(ResIDT inWINDid, Uint32 inAttributes,
								LCommander *inSuperCommander);
					*/
	virtual			~CTestWindow();
	void			resetStats(void);
	void			AddText(Ptr inTextP, Int32 inTextLen);
	static CTestWindow *GetTopWindow(void) { return gTopTestWindow; }
	CTextEdit		*GetWindowText(void) { return fLogView; }
	
	
							/* pp-ism's */
	virtual void	FinishCreateSelf();
	static CTestWindow*	CreateWindowStream(LStream	*inStream);
	void			SpendTime(const EventRecord &inMacEvent);
	void			ListenToMessage(MessageT inMessage, void *ioParam);
	void			setLoggingState(Boolean doLog);
	Boolean			ObeyCommand(CommandT inCommand, void *ioParam = nil);
	void			FindCommandStatus(CommandT inCommand,
							Boolean &outEnabled, Boolean &outUsesMark,
							Char16 &outMark, Str255 outName);

protected:
	void			doLinkTest(Boolean isServer);
	void			doGTLinkTest(Boolean isServer);
	void			doSTLinkTest(Boolean isServer);
	void			doTransportDownload(void);
	void			doTimmerScript(void);
	void			waitForConnection(Boolean useDefaultConn);
	Boolean			establishConnection(Boolean useDefaultConn);
	Boolean			killConnection(void);

	SessionRec		fTSocket;
	CTextEdit		*fLogView;
	LEditField		*fSendTextEdit;
	Boolean			fConnected;
	Handle			fTextH;
	CTestWindow		*fNextWindow;
	Boolean			fUseGameTalk;
};
