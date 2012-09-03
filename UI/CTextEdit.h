/*
	File:		CTextEdit.h

	Contains:	xxx put contents here xxx

	Written by:	Brian Topping

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <2>	  6/3/94	BET		"Play with windows and stuff"
		 <1>	 5/31/94	BET		first checked in
	To Do:
*/

#pragma once

class CTextEdit : public LTextEdit {
public:
	static CTextEdit*	CreateTextEditStream(LStream *inStream);

					CTextEdit();
					CTextEdit(const CTextEdit &inOriginal);
					CTextEdit(const SPaneInfo &inPaneInfo,
							const SViewInfo &inViewInfo,
							Uint16 inTextAttributes,
							ResIDT inTextTraitsID);
					CTextEdit(LStream *inStream);
	virtual			~CTextEdit();
	TEHandle		GetTEHandle(void) { return mTextEditH; }
	void			AddText(Ptr inTextP, Int32 inTextLen);
};
