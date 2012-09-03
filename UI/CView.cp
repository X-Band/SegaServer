/*
	File:		CView.cp

	Contains:	xxx put contents here xxx

	Written by:	Brian Topping

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <1>	 5/22/94	BET		first checked in

	To Do:
*/

// ===========================================================================
//	CView.cp					©1994 Brian Topping All rights reserved.
// ===========================================================================

#include "CView.h"
#include <stdio.h>


CView::CView()
{
}

CView::CView(LStream *inStream)
	:LView(inStream)
{
	fTheBuffer = nil;
}

CView::~CView()
{
}

CView*
CView::CreateViewStream(
	LStream	*inStream)
{
	return (new CView(inStream));
}

void
CView::DrawSelf()
{
	Rect	frame;
	CalcLocalFrameRect(frame);
	
	SDimension32	imageSize;
	GetImageSize(imageSize);
	
	SPoint32	topPos;
	LocalToImagePoint(topLeft(frame), topPos);
	SPoint32	botPos;
	LocalToImagePoint(botRight(frame), botPos);
	if (botPos.v > imageSize.height) {
		botPos.v = imageSize.height;
	}
	SPoint32	imagePos;
	imagePos.h = 4;
	imagePos.v = 0;
	
	EraseRect(&frame);
	
	Point	localPos;
	ImageToLocalPoint(imagePos, localPos);
	MoveTo(localPos.h, localPos.v + kRowHeight);
	if (fTheBuffer)
		DrawString((StringPtr)fTheBuffer->buf1);
}