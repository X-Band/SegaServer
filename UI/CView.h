/*
	File:		CView.h

	Contains:	xxx put contents here xxx

	Written by:	Brian Topping

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <2>	 5/26/94	BET		Remove refs to obsoleted GameTalk.h
		 <1>	 5/22/94	BET		first checked in

	To Do:
*/

// ===========================================================================
//	CView.h					©1994 Brian Topping All rights reserved.
// ===========================================================================

#include "LView.h"
#include "TransportLayer.h"

class CSerialLink;

enum {
	kRowHeight = 12,
	kModemColumn = 90,
	kPrinterColumn = 150
	};


class CView : public LView {
public:
					CView();
					CView(LStream *inStream);
	virtual			~CView();

	void			DrawSelf();
	static CView*	CreateViewStream(LStream	*inStream);
	
	void			setBuffer(RDS *theBuf) { fTheBuffer = theBuf; }
protected:
	RDS				*fTheBuffer;
};
