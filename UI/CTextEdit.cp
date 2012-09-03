/*
	File:		CTextEdit.cp

	Contains:	xxx put contents here xxx

	Written by:	Brian Topping

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <2>	  6/1/94	BET		Make it scroll
		 <1>	 5/31/94	BET		first checked in
	To Do:
*/


#include "CTextEdit.h"

CTextEdit::CTextEdit()
{
}

CTextEdit::CTextEdit(LStream *inStream)
	:LTextEdit(inStream)
{
}

CTextEdit*
CTextEdit::CreateTextEditStream(
	LStream	*inStream)
{
	return (new CTextEdit(inStream));
}

CTextEdit::~CTextEdit()
{
}

void
CTextEdit::AddText(Ptr inTextP, Int32 inTextLen)
{
RgnHandle	rgn = NewRgn(), rgn2 = NewRgn();
Rect		theRect;

	GetClip(rgn);
	GetRevealedRect(theRect);
	SetRectRgn(rgn2, theRect.left, theRect.top, theRect.right, theRect.bottom);
	SetClip(rgn2);
	TESetSelect(32767,32767,mTextEditH);
	TEInsert(inTextP,inTextLen,mTextEditH);
	TECalText(mTextEditH);
	AdjustImageToText();
	ScrollImageBy(0,mImageSize.height - mFrameSize.height -
							(mFrameLocation.v - mImageLocation.v), false);
	SetClip(rgn);
	DisposeRgn(rgn);
	DisposeRgn(rgn2);
	Refresh();
}

